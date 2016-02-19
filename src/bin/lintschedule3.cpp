
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <castus4-public/schedule.h>
#include <castus4-public/parsetime.h>
#include <castus4-public/gentime.h>
#include <castus4-public/chomp.h>
#include <castus4-public/schedule_object.h>

#include <iostream>

using namespace std;

int main(int argc,char **argv) {
	Castus4publicSchedule schedule;
	char line[1024];
	FILE *fp;

	if (argc < 2) {
		fprintf(stderr,"loadschedule <schedule>\n");
		return 1;
	}

	fp = fopen(argv[1],"r");
	if (!fp) {
		fprintf(stderr,"Cannot open schedule file %s\n",argv[1]);
		return 1;
	}

	schedule.begin_load();
	memset(line,0,sizeof(line));
	while (!feof(fp) && !ferror(fp)) {
		if (fgets(line,sizeof(line)-1,fp) == NULL) break;
		castus4public_chomp(line);
		schedule.load_take_line(line);
	}
	schedule.end_load();
	fclose(fp);

	for (std::list<Castus4publicSchedule::ScheduleItem>::iterator i=schedule.schedule_items.begin();i!=schedule.schedule_items.end();i++) {
		Castus4publicSchedule::ideal_time_t ideal;
		unsigned long tm_usec;
		const char *item;
		struct tm tm;

		item = i->getItem();
		fprintf(stderr,"Entry: %s\n",item != NULL ? item : "(null)");

		ideal = i->getStartTime();
		i->getStartTimeTm(tm,tm_usec);
		fprintf(stderr,"   Starts at: (%lld) %04u-%02u-%02u (wday=%u) %02u:%02u:%02u.%06lu\n",
			(signed long long)ideal,
			tm.tm_year+1900,
			tm.tm_mon+1,
			tm.tm_mday,
			tm.tm_wday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec,
			(unsigned long)tm_usec);

		i->setStartTime(ideal);
		if (i->getStartTime() != ideal) fprintf(stderr,"!! start time set fail\n");

		ideal = i->getEndTime();
		i->getEndTimeTm(tm,tm_usec);
		fprintf(stderr,"     Ends at: (%lld) %04u-%02u-%02u (wday=%u) %02u:%02u:%02u.%06lu\n",
			(signed long long)ideal,
			tm.tm_year+1900,
			tm.tm_mon+1,
			tm.tm_mday,
			tm.tm_wday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec,
			(unsigned long)tm_usec);

		i->setEndTime(ideal);
		if (i->getEndTime() != ideal) fprintf(stderr,"!! start time set fail\n");
	}

	for (std::list<Castus4publicSchedule::ScheduleBlock>::iterator i=schedule.schedule_blocks.begin();i!=schedule.schedule_blocks.end();i++) {
		Castus4publicSchedule::ideal_time_t ideal;
		unsigned long tm_usec;
		const char *item;
		struct tm tm;

		item = i->getBlockName();
		fprintf(stderr,"Block: %s\n",item != NULL ? item : "(null)");

		ideal = i->getStartTime();
		i->getStartTimeTm(tm,tm_usec);
		fprintf(stderr,"   Starts at: (%lld) %04u-%02u-%02u (wday=%u) %02u:%02u:%02u.%06lu\n",
			(signed long long)ideal,
			tm.tm_year+1900,
			tm.tm_mon+1,
			tm.tm_mday,
			tm.tm_wday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec,
			(unsigned long)tm_usec);

		i->setStartTime(ideal);
		if (i->getStartTime() != ideal) fprintf(stderr,"!! start time set fail\n");

		ideal = i->getEndTime();
		i->getEndTimeTm(tm,tm_usec);
		fprintf(stderr,"     Ends at: (%lld) %04u-%02u-%02u (wday=%u) %02u:%02u:%02u.%06lu\n",
			(signed long long)ideal,
			tm.tm_year+1900,
			tm.tm_mon+1,
			tm.tm_mday,
			tm.tm_wday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec,
			(unsigned long)tm_usec);

		i->setEndTime(ideal);
		if (i->getEndTime() != ideal) fprintf(stderr,"!! start time set fail\n");
	}

	schedule.sort_schedule_items();
	schedule.sort_schedule_blocks();
	if (!schedule.write_out(cout))
		fprintf(stderr,"Error while writing schedule\n");

	return 0;
}

