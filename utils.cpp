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

    auto logic = [min_blank_interval](Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item) {
        current_item.deleteValue("x-next-joined");
        // auto c_start = current_item->getStartTime();
        auto c_end = current_item.getEndTime();

        auto n_start = next_item.getStartTime();
        auto n_end = next_item.getEndTime();

        next_item.deleteValue("x-next-joined");

        if ((c_end + min_blank_interval) >= n_start) {
            char tmp[64];

            if (c_end < n_start) {
                sprintf(tmp,"%llu",n_start - c_end);
                current_item.setValue("x-next-joined-gap-us",tmp);
            }

            current_item.setValue("x-next-joined","1");
        }

    };

    loop(schedule, logic);
}

/**
 * This function takes a schedule and reads the duration out of the metadata and
 * sets `item duration` to it.
 *
 * \param schedule The Castus schedule
 **/
void update_duration(Castus4publicSchedule &schedule) {
    auto logic = [](Castus4publicSchedule::ScheduleItem &schedule_item) {
        const char *item = schedule_item.getValue("item");

        if (item == NULL) return;

        std::string metadir = castus4public_file_to_metadata_dir(item);
        if (metadir.empty()) return;

        castus4public_metadata_list meta;
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
            sprintf(tmp,"%.3f",duration);
            schedule_item.setValue("item duration",tmp);
        }

        auto c_start = schedule_item.getStartTime();
        auto c_end = c_start + (Castus4publicSchedule::ideal_time_t)(duration * 1000000);
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
    auto logic = [](Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item) {
        auto c_start = current_item.getStartTime();
        auto c_end = current_item.getEndTime();

        auto n_start = next_item.getStartTime();
        auto n_end = next_item.getEndTime();

        const char *joined = current_item.getValue("x-next-joined");
        if (joined != NULL && atoi(joined) > 0) {
            /* no matter whether the item duration grew or shrank, the items remain joined together */
            unsigned long long gap = 0;
            unsigned long long old_duration = n_end - n_start;
            const char *gap_str = current_item.getValue("x-next-joined-gap-us");

            if (gap_str != NULL)
                gap = strtoull(gap_str,NULL,0);

            // move the next item
            n_start = c_end + gap;
            n_end = n_start + old_duration;

            next_item.setStartTime(n_start);
            next_item.setEndTime(n_end);
        }

    current_item.deleteValue("x-next-joined-gap-us");
    current_item.deleteValue("x-next-joined");
    };

    loop(schedule, logic);
}

/**
 * This function takes a schedule and if any two sequential schedule items overlap
 * it adjusts the times of the second time
 *
 * \param schedule The Castus schedule
 **/
void ripple_down_overlapping(Castus4publicSchedule &schedule) {

    auto logic = [](Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item) {

        // auto c_start = current_item->getStartTime();
        auto c_end = current_item.getEndTime();

        auto n_start = next_item.getStartTime();
        auto n_end = next_item.getEndTime();

        if (c_end > n_start) {
            unsigned long long old_duration = n_end - n_start;

            // move the next item down
            n_start = c_end;
            n_end = n_start + old_duration;

            next_item.setStartTime(n_start);
            next_item.setEndTime(n_end);
        }
    };

    loop(schedule, logic);
}

/**
 * This function takes a schedule and if any two sequential schedule items overlap
 * the seconds times are adjusted
 *
 * \param schedule The Castus schedule
 **/
void trim_overlapping(Castus4publicSchedule &schedule) {
    auto logic = [](Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item) {
        auto c_end = current_item.getEndTime();

        auto n_start = next_item.getStartTime();
        auto n_end = next_item.getEndTime();
        if (c_end > n_start && c_end < (n_start + 1000000ull)) current_item.setEndTime(n_start);
    };
    loop(schedule, logic);
}

