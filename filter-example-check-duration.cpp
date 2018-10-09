
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
#include <castus4-public/metadata.h>
#include <castus4-public/schedule_object.h>

#include <iostream>

using namespace std;

int main() {
    const Castus4publicSchedule::ideal_time_t min_blank_interval = 1000000; /* 1000000us = 1000ms = 1 sec */
	Castus4publicSchedule schedule;
	char line[1024];

	schedule.begin_load();
	memset(line,0,sizeof(line));
	while (!feof(stdin) && !ferror(stdin)) {
		if (fgets(line,sizeof(line)-1,stdin) == NULL) break;
		castus4public_chomp(line);
		schedule.load_take_line(line);
	}
	schedule.end_load();

    schedule.sort_schedule_items();
    schedule.sort_schedule_blocks();

#ifdef ENABLE_RIPPLE
    // step 1: add a tag for each item that is touching
    for (auto sci=schedule.schedule_items.begin();sci!=schedule.schedule_items.end();) {
        Castus4publicSchedule::ideal_time_t c_start,c_end;
        Castus4publicSchedule::ideal_time_t n_start,n_end;

        auto c_item = sci;

        c_item->deleteValue("x-next-joined");

        sci++;
        if (sci == schedule.schedule_items.end()) break;

        auto n_item = sci;

        c_start = c_item->getStartTime();
        c_end = c_item->getEndTime();

        n_start = n_item->getStartTime();
        n_end = n_item->getEndTime();

        n_item->deleteValue("x-next-joined");

        if (c_start != Castus4publicSchedule::ideal_time_t_invalid &&
            c_end !=   Castus4publicSchedule::ideal_time_t_invalid &&
            n_start != Castus4publicSchedule::ideal_time_t_invalid &&
            n_end !=   Castus4publicSchedule::ideal_time_t_invalid &&
            c_start < c_end && n_start < n_end) {
            if ((c_end + min_blank_interval) >= n_start) {
                char tmp[64];

                if (c_end < n_start) {
                    sprintf(tmp,"%llu",n_start - c_end);
                    c_item->setValue("x-next-joined-gap-us",tmp);
                }

                c_item->setValue("x-next-joined","1");
            }
        }
    }
#endif

    // step 2: check each item, read the duration from metadata, and update the duration from meta
    for (auto sci=schedule.schedule_items.begin();sci!=schedule.schedule_items.end();sci++) {
        Castus4publicSchedule::ideal_time_t c_start,c_end;

        c_start = sci->getStartTime();
        c_end = sci->getEndTime();

        if (c_start == Castus4publicSchedule::ideal_time_t_invalid ||
            c_end ==   Castus4publicSchedule::ideal_time_t_invalid)
            continue;

        const char *item = sci->getValue("item");

        if (item == NULL) continue;

        std::string metadir = castus4public_file_to_metadata_dir(item);
        if (metadir.empty()) continue;

        castus4public_metadata_list meta;
        std::string metapath = metadir + "/metadata";
        if (!meta.read_metadata(metapath.c_str())) continue;

        const char *duration_str = meta.getValue("duration");
        if (duration_str == NULL) continue;

        double duration = strtod(duration_str,NULL);
        if (duration < 0.1) continue;

	{
		const char *m = meta.getValue("out");
		if (m != NULL && *m != 0) {
			double v = strtod(m,NULL);
			if (v >= 0 && duration > v) duration = v;
			sci->setValue("out",m);
		}
		else {
			sci->deleteValue("out");
		}
	}

	{
		const char *m = meta.getValue("in");
		if (m != NULL && *m != 0) {
			double v = strtod(m,NULL);
			if (v >= 0) duration -= v;
			if (duration < 0.01) duration = 0.01;
			sci->setValue("in",m);
		}
		else {
			sci->deleteValue("in");
		}
	}

        {
            char tmp[128];
            sprintf(tmp,"%.3f",duration);
            sci->setValue("item duration",tmp);
        }

        c_end = c_start + (Castus4publicSchedule::ideal_time_t)(duration * 1000000);
        sci->setEndTime(c_end);
    }

#ifdef ENABLE_RIPPLE
    // step 3: ripple up or down connected items
    for (auto sci=schedule.schedule_items.begin();sci!=schedule.schedule_items.end();) {
        Castus4publicSchedule::ideal_time_t c_start,c_end;
        Castus4publicSchedule::ideal_time_t n_start,n_end;

        auto c_item = sci;

        sci++;
        if (sci == schedule.schedule_items.end()) break;

        auto n_item = sci;

        c_start = c_item->getStartTime();
        c_end = c_item->getEndTime();

        n_start = n_item->getStartTime();
        n_end = n_item->getEndTime();

        if (c_start != Castus4publicSchedule::ideal_time_t_invalid &&
            c_end !=   Castus4publicSchedule::ideal_time_t_invalid &&
            n_start != Castus4publicSchedule::ideal_time_t_invalid &&
            n_end !=   Castus4publicSchedule::ideal_time_t_invalid &&
            c_start < c_end && n_start < n_end) {
            const char *joined = c_item->getValue("x-next-joined");
            if (joined != NULL && atoi(joined) > 0) {
                /* no matter whether the item duration grew or shrank, the items remain joined together */
                unsigned long long gap = 0;
                unsigned long long old_duration = n_end - n_start;
                const char *gap_str = c_item->getValue("x-next-joined-gap-us");

                if (gap_str != NULL)
                    gap = strtoull(gap_str,NULL,0);

                // move the next item
                n_start = c_end + gap;
                n_end = n_start + old_duration;

                n_item->setStartTime(n_start);
                n_item->setEndTime(n_end);
            }
        }

        c_item->deleteValue("x-next-joined-gap-us");
        c_item->deleteValue("x-next-joined");
    }

    // step 4: ripple down overlapping items
    for (auto sci=schedule.schedule_items.begin();sci!=schedule.schedule_items.end();) {
        Castus4publicSchedule::ideal_time_t c_start,c_end;
        Castus4publicSchedule::ideal_time_t n_start,n_end;

        auto c_item = sci;

        sci++;
        if (sci == schedule.schedule_items.end()) break;

        auto n_item = sci;

        c_start = c_item->getStartTime();
        c_end = c_item->getEndTime();

        n_start = n_item->getStartTime();
        n_end = n_item->getEndTime();

        if (c_start != Castus4publicSchedule::ideal_time_t_invalid &&
            c_end !=   Castus4publicSchedule::ideal_time_t_invalid &&
            n_start != Castus4publicSchedule::ideal_time_t_invalid &&
            n_end !=   Castus4publicSchedule::ideal_time_t_invalid &&
            c_start < c_end && n_start < n_end) {
            if (c_end > n_start) {
                unsigned long long old_duration = n_end - n_start;

                // move the next item down
                n_start = c_end;
                n_end = n_start + old_duration;

                n_item->setStartTime(n_start);
                n_item->setEndTime(n_end);
            }
        }
    }
#else
    /* if the items overlap only SLIGHTLY, then go ahead and trim back a bit */
    for (auto sci=schedule.schedule_items.begin();sci!=schedule.schedule_items.end();) {
        Castus4publicSchedule::ideal_time_t c_start,c_end;
        Castus4publicSchedule::ideal_time_t n_start,n_end;

        auto c_item = sci;

        sci++;
        if (sci == schedule.schedule_items.end()) break;

        auto n_item = sci;

        c_start = c_item->getStartTime();
        c_end = c_item->getEndTime();

        n_start = n_item->getStartTime();
        n_end = n_item->getEndTime();

        if (c_start != Castus4publicSchedule::ideal_time_t_invalid &&
            c_end !=   Castus4publicSchedule::ideal_time_t_invalid &&
            n_start != Castus4publicSchedule::ideal_time_t_invalid &&
            n_end !=   Castus4publicSchedule::ideal_time_t_invalid &&
            c_start < c_end && n_start < n_end) {
            if (c_end > n_start && c_end < (n_start + 500000ull))
                c_item->setEndTime(n_start);
        }
    }
#endif

	schedule.sort_schedule_items();
	schedule.sort_schedule_blocks();
	if (!schedule.write_out(cout))
		fprintf(stderr,"Error while writing schedule\n");

	return 0;
}

