
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <castus4-public/schedule.h>
#include <castus4-public/parsetime.h>
#include <castus4-public/gentime.h>
#include <castus4-public/chomp.h>
#include <castus4-public/schedule_object.h>
#include <castus4-public/metadata.h>

#include <iostream>

using namespace std;

const char *ads_path = "/mnt/main/Breaks";

class AdBreak {
public:
	AdBreak() : duration_us(0) {
	}
	~AdBreak() {
	}
public:
	unsigned long long		duration_us;
	std::string			path;
};

std::list<Castus4publicSchedule::ScheduleItem>::iterator insert_ad_break(std::vector<AdBreak> &breaks,std::list<Castus4publicSchedule::ScheduleItem>::iterator in,std::list<Castus4publicSchedule::ScheduleItem> &schedule_items,unsigned long long &play_adjust,int schedule_type,unsigned long long startTime) {
	unsigned long long round = 5ULL * 1000000ULL;
	unsigned long long length;
	char tmp[64];

	if (breaks.empty()) return in;

	size_t pick = ((size_t)rand()) % breaks.size();
	AdBreak &advert = breaks[pick];

	if (advert.duration_us == 0ULL || advert.path.empty()) return in;
	length = advert.duration_us;
	length += round / 2UL;
	length -= length % round;
	play_adjust += length;

	Castus4publicSchedule::ScheduleItem entry(schedule_type);
	entry.setItem(advert.path);
	sprintf(tmp,"%.3f",(double)advert.duration_us / 1000000);
	entry.setValue("item duration",tmp);
	entry.setStartTime(startTime);
	entry.setEndTime(startTime + std::min(advert.duration_us,length));
	entry.setValue("advertisement","1");
	in = schedule_items.insert(in,entry);
	in++;
	return in;
}

void load_ad_breaks(std::vector<AdBreak> &breaks) {
	std::string file_path,meta_path,metadata_file;
	castus4public_metadata_list mlist;
	struct dirent *d;
	struct stat st;
	DIR *dir;

	breaks.clear();

	dir = opendir(ads_path);
	if (dir == NULL) return;

	while ((d=readdir(dir)) != NULL) {
		if (d->d_name[0] == '.') continue;

		file_path = std::string(ads_path) + "/" + d->d_name;
		if (stat(file_path.c_str(),&st) || !S_ISREG(st.st_mode)) continue;

		meta_path = castus4public_file_to_metadata_dir(file_path);
		if (meta_path.empty()) continue;

		metadata_file = meta_path + "/metadata";
		if (stat(metadata_file.c_str(),&st) || !S_ISREG(st.st_mode)) continue;

		if (!mlist.read_metadata(metadata_file.c_str())) continue;

		AdBreak ad;

		const char *duration = mlist.getValue("duration");
		if (duration != NULL) ad.duration_us = (unsigned long long)(atof(duration) * 1000000);
		ad.path = file_path;

		breaks.push_back(ad);
	}

	closedir(dir);
}

int main(int argc,char **argv) {
	Castus4publicSchedule schedule;
	std::vector<AdBreak> breaks;
	char line[1024];

	srand(time(NULL));
	load_ad_breaks(/*&*/breaks);
#if 0//DEBUG
	for (size_t i=0;i < breaks.size();i++) {
		AdBreak &a = breaks[i];

		fprintf(stderr,"Ad break '%s' duration=%.3f\n",
			a.path.c_str(),(double)a.duration_us / 1000000);
	}
#endif

	schedule.begin_load();
	memset(line,0,sizeof(line));
	while (!feof(stdin) && !ferror(stdin)) {
		if (fgets(line,sizeof(line)-1,stdin) == NULL) break;
		castus4public_chomp(line);
		schedule.load_take_line(line);
	}
	schedule.end_load();

	{
		Castus4publicSchedule::ideal_time_t start,end,bump=0,last_end=0;
		std::list<Castus4publicSchedule::ScheduleItem>::iterator sciter,sciter_next;

		for (sciter=schedule.schedule_items.begin();sciter!=schedule.schedule_items.end();) {
			start=sciter->getStartTime();
			end=sciter->getEndTime();

			// strip out advertisements we already inserted
			{
				const char *v = sciter->getValue("advertisement");
				if (v != NULL && atoi(v) > 0) {
					Castus4publicSchedule::ideal_time_t sub = end - start;
					if (sub > bump) sub = bump;
					bump -= sub;

					sciter = schedule.schedule_items.erase(sciter);
					continue;
				}
			}

			if (start == Castus4publicSchedule::ideal_time_t_invalid ||
				end == Castus4publicSchedule::ideal_time_t_invalid) {
				sciter++;
				continue;
			}

			if (bump != 0 && start > last_end) {
				Castus4publicSchedule::ideal_time_t sub = start - last_end;
				if (sub > bump) sub = bump;
				bump -= sub;
			}

			if (bump != 0) {
				start += bump;
				end += bump;
				sciter->setStartTime(start);
				sciter->setEndTime(end);
			}

			unsigned long long chop_size = (10ULL * 60ULL * 1000000ULL); // 10 minutes   (10 min x 60 sec/min * 1000000 microsec/sec)

			unsigned long long block_start = (unsigned long long)start / chop_size; // 10 min
			unsigned long long block_end = (unsigned long long)end / chop_size; // 10 min block

			if (block_start != block_end) {
				unsigned long long play_adjust = 0;
				unsigned long long in_point = 0;
				unsigned long long seg1,seg2;
				unsigned long long block;
				char tmp[64];

				// it crosses a 10 min boundary. chop it up
				Castus4publicSchedule::ScheduleItem orig_ref = *sciter,copy(0);
				sciter = schedule.schedule_items.erase(sciter);

				// first partial
				block = block_start;
				seg1 = (block + 1ULL) * chop_size; // 10 min block
				copy = orig_ref;
				copy.setEndTime(seg1);
				sciter = schedule.schedule_items.insert(sciter,copy); sciter++;
				block++;
				in_point += seg1 - start;

				// ad break
				sciter = insert_ad_break(breaks,sciter,schedule.schedule_items,/*&*/play_adjust,schedule.schedule_type,copy.getEndTime());

				// intermediate parts
				while (block < block_end) {
					seg1 = play_adjust + ((block + 0ULL) * chop_size); // 10 min block
					seg2 = play_adjust + ((block + 1ULL) * chop_size); // 10 min block
					copy = orig_ref;
					copy.setEndTime(seg2);
					copy.setStartTime(seg1);

					sprintf(tmp,"%.3f",(double)in_point / 1000000);
					copy.setValue("in",tmp);

					sciter = schedule.schedule_items.insert(sciter,copy); sciter++;
					block++;
					in_point += chop_size;

					// ad break
					sciter = insert_ad_break(breaks,sciter,schedule.schedule_items,/*&*/play_adjust,schedule.schedule_type,copy.getEndTime());
				}

				// last partial
				assert(block == block_end);
				seg1 = play_adjust + ((block + 0ULL) * chop_size); // 10 min block
				copy = orig_ref;
				copy.setStartTime(seg1);
				copy.setEndTime(orig_ref.getEndTime() + play_adjust);

				sprintf(tmp,"%.3f",(double)in_point / 1000000);
				copy.setValue("in",tmp);

				sciter = schedule.schedule_items.insert(sciter,copy); sciter++;

				last_end = copy.getEndTime();
				bump += play_adjust;
			}
			else {
				last_end = end;
				sciter++;
			}
		}
	}

	schedule.sort_schedule_items();
	schedule.sort_schedule_blocks();
	if (!schedule.write_out(stdout))
		fprintf(stderr,"Error while writing schedule\n");

	return 0;
}

