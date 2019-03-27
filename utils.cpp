#include <functional>
#include <iostream>
#include <cmath>
#include <memory>

#include <castus4-public/schedule_object.h>

#include "utils.h"
#include "utils_schedule.h"

using namespace std;

/**
 * This function takes a schedule and if any two sequential schedule items overlap
 * it makes the item with `x-next-joined-gap-us`
 *
 * \param schedule The Castus schedule
 **/
void tag_touching_item(Castus4publicSchedule &schedule)
{
    const Castus4publicSchedule::ideal_time_t min_blank_interval = 1000000; /* 1000000us = 1000ms = 1 sec */

    auto tag_one_adjacency = [min_blank_interval](Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item) {
        // Clear previous join information
        current_item.deleteValue("x-next-joined");
        next_item.deleteValue("x-next-joined");
        // TODO(Alex): Should we also clear x-next-joined-gap-us?

        auto c_end   = current_item.getEndTime();
        auto n_start = next_item.getStartTime();
        auto n_end   = next_item.getEndTime();


        // If the current item ends within min_blank_interval of when the next starts
        if ((c_end + min_blank_interval) >= n_start) {
            char tmp[128];

            // But does not already overlap
            if (c_end < n_start) {
                snprintf(tmp, 127, "%llu",n_start - c_end);
                // Mark it with the length of the gap so it can be preserved
                current_item.setValue("x-next-joined-gap-us",tmp);
            }

            // Whether or not they overlap, mark them as joined
            current_item.setValue("x-next-joined","1");
        }

    };

    loop(schedule, tag_one_adjacency);
}

/**
 * This function takes a schedule and reads the duration out of the metadata and
 * sets `item duration` to it.
 *
 * \param schedule The Castus schedule
 **/
void update_duration(Castus4publicSchedule &schedule) {
    using SchedItem = Castus4publicSchedule::ScheduleItem;
    using Metadata = castus4public_metadata_list;

    auto logic = [](SchedItem &schedule_item) {
        const char *item = schedule_item.getValue("item");

        if (item == NULL) return;

        std::string metadir = castus4public_file_to_metadata_dir(item);
        if (metadir.empty()) return;

        Metadata meta;
        std::string metapath = metadir + "/metadata";
        if (!meta.read_metadata(metapath.c_str())) return;

        const char *duration_str = meta.getValue("duration");
        if (duration_str == NULL) return;

        double duration = strtod(duration_str,NULL);
        if (duration < 0.1) return;

        const char *m = meta.getValue("out");
        if (m != NULL && *m != 0) {
            double v = strtod(m,NULL);
            if (v >= 0 && duration > v) duration = v;
            schedule_item.setValue("out",m);
        }
        else schedule_item.deleteValue("out");

        m = meta.getValue("in");
        if (m != NULL && *m != 0) {
            double v = strtod(m,NULL);
            if (v >= 0) duration -= v;
            if (duration < 0.01) duration = 0.01;
            schedule_item.setValue("in",m);
        }
        else schedule_item.deleteValue("in");

        {
            char tmp[128];
            snprintf(tmp, 127, "%.3f",duration);
            schedule_item.setValue("item duration",tmp);
        }

        auto c_start = schedule_item.getStartTime();
        auto c_end = c_start + (Castus4publicSchedule::ideal_time_t)(duration * 1000000 /* μs */);
        schedule_item.setEndTime(c_end);
    };

    loop(schedule, logic);
}

/**
 * This function takes a schedule and if any two sequential schedule items overlap
 * it adjusts the times of the second point
 *
 * \param schedule The Castus schedule
 **/
void ripple_connected_item(Castus4publicSchedule &schedule) {
    auto repair_gap = [](Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item) {
        auto c_start = current_item.getStartTime();
        auto c_end   = current_item.getEndTime();
        auto n_start = next_item.getStartTime();
        auto n_end   = next_item.getEndTime();

        const char *joined = current_item.getValue("x-next-joined");
        // If these two items were joined together
        if (joined != NULL && atoi(joined) > 0) {
            unsigned long long gap = 0;
            unsigned long long old_duration = n_end - n_start;

            // Read back what the gap between them used to be
            const char *gap_str = current_item.getValue("x-next-joined-gap-us");
            if (gap_str != NULL) {
                gap = strtoull(gap_str,NULL,0);
            }

            // Move the next item in such a way that the old gap is restored
            n_start = c_end + gap;
            n_end = n_start + old_duration;

            next_item.setStartTime(n_start);
            next_item.setEndTime(n_end);
        }

        // Clear out the cached values
        current_item.deleteValue("x-next-joined-gap-us");
        current_item.deleteValue("x-next-joined");
    };

    loop(schedule, repair_gap);
}

/**
 * This function takes a schedule and if any two sequential schedule items overlap
 * it adjusts the times of the second time
 *
 * \param schedule The Castus schedule
 **/
void ripple_down_overlapping(Castus4publicSchedule &schedule) {

    auto ripple_one = [](Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item) {

        auto c_end   = current_item.getEndTime();
        auto n_start = next_item.getStartTime();
        auto n_end   = next_item.getEndTime();

        if (c_end > n_start) {
            unsigned long long old_duration = n_end - n_start;

            // move the next item down
            n_start = c_end;
            n_end = n_start + old_duration;

            next_item.setStartTime(n_start);
            next_item.setEndTime(n_end);
        }
    };

    loop(schedule, ripple_one);
}

/**
 * This function takes a schedule and if any two sequential schedule items overlap
 * the seconds times are adjusted
 *
 * \param schedule The Castus schedule
 **/
void trim_overlapping(Castus4publicSchedule &schedule) {
    auto trim_one = [](Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item) {
        auto c_end = current_item.getEndTime();

        auto n_start = next_item.getStartTime();
        auto n_end = next_item.getEndTime();
        // If the current item overlaps the next item by less than
        // 1,000,000 (what units? μs?), shorten it.
        if (c_end > n_start && c_end < (n_start + 1000000ull)) {
            current_item.setEndTime(n_start);
        }
    };
    loop(schedule, trim_one);
}

