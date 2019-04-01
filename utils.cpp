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
void tag_touching_item(Castus4publicSchedule &schedule, Castus4publicSchedule::ideal_time_t min_blank_interval)
{

    auto tag_one_adjacency = [min_blank_interval](Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item) {
        // Clear previous join information
        current_item.deleteValue("x-next-joined");
        next_item.deleteValue("x-next-joined");
        // TODO(Alex): Should we also clear x-next-joined-gap-us?

        auto c_end   = current_item.getEndTime();
        auto n_start = next_item.getStartTime();


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
 * This function takes a single schedule item and updates its
 * duration based on the relevant file's on-disk metadata
 *
 * \param schedule_item The item to modify
 */
void update_timing(Castus4publicSchedule::ScheduleItem& schedule_item) {
    using SchedItem = Castus4publicSchedule::ScheduleItem;
    using Metadata = castus4public_metadata_list;

    auto load_meta = [](SchedItem& schedule_item) {
        const char *item = schedule_item.getValue("item");

        if (item == NULL) return std::unique_ptr<Metadata>{ nullptr };

        std::string metadir = castus4public_file_to_metadata_dir(item);
        if (metadir.empty()) return std::unique_ptr<Metadata>{ nullptr };

        std::unique_ptr<Metadata> meta{ new Metadata };
        std::string metapath = metadir + "/metadata";
        if (!meta->read_metadata(metapath.c_str())) return std::unique_ptr<Metadata>{ nullptr };

        return meta;
    };

    auto read_duration = [](Metadata& meta) {
        const char *duration_str = meta.getValue("duration");
        if (!duration_str) return (double)NAN;

        double duration = strtod(duration_str,NULL);
        if (duration < 0.1) return (double)NAN;

        return duration;
    };

    auto read_meta_double = [](Metadata& meta, const char* name) {
        auto entry = meta.getValue(name);
        if (entry && *entry != '\0') {
            double value = strtod(entry,NULL);
            return value;
        } else {
            return (double)NAN;
        }
    };

    auto adjust_duration = [] (double& duration, double in_point, double out_point) {
        if (!std::isnan(out_point) && out_point >= 0 && out_point < duration) {
            // Adjust the duration to remove the unplayed epilogue
            // TODO(Alex): Use `duration -= duration - out_point` because
            //             this more accurately represents the
            //             operation, and allows reordering
            //             the in/out handling.
            duration = out_point;
        }

        if (!std::isnan(in_point) && in_point >= 0) {
            // Adjust the duration to remove the skipped prologue
            duration -= in_point;
        }

        // Clamp duration to a minimum of 0.01 seconds
        if (duration < 0.01) {
            duration = 0.01;
        }
    };

    auto update_schedule = [](SchedItem &schedule_item, double duration, double in_point, double out_point) {
        char tmp[128];

        if (std::isnan(out_point)) {
            // If the item had no out point, clear the cached
            // value from the schedule
            schedule_item.deleteValue("out");
        } else {
            // Propagate the out point into the schedule
            snprintf(tmp, 127, "%.3f", out_point);
            schedule_item.setValue("out", tmp);
        }

        if (std::isnan(in_point)) {
            // If the item had no in point, clear the cached
            // value from the schedule
            schedule_item.deleteValue("in");
        } else {
            // Propagate the in point into the schedule
            snprintf(tmp, 127, "%.3f", out_point);
            schedule_item.setValue("in", tmp);
        }

        if(!std::isnan(duration)) {
            char tmp[128];
            snprintf(tmp, 127, "%.3f",duration);
            schedule_item.setValue("item duration",tmp);
        }

        auto c_start = schedule_item.getStartTime();
        auto c_end = c_start + (Castus4publicSchedule::ideal_time_t)(duration * 1000000 /* μs */);
        schedule_item.setEndTime(c_end);
    };


    auto meta = load_meta(schedule_item);
    if (!meta) {
        return;
    }

    double duration  = read_duration(*meta);
    // Return early if the duration is unset
    if (std::isnan(duration)) {
        return;
    }

    // Read the in point (where to begin playing the item)
    double in_point  = read_meta_double(*meta, "in");
    // Read the out point (the "logical" end of the item
    // even when more content remains)
    double out_point = read_meta_double(*meta, "out");
    // Adjust the duration to account for the in and out points
    adjust_duration(duration, in_point, out_point);
    // Alter the relevant fields of the schedule
    update_schedule(schedule_item, duration, in_point, out_point);
}

/**
 * This function takes a schedule and reads the duration out of the metadata and
 * sets `item duration` to it.
 *
 * \param schedule The Castus schedule
 **/
void update_duration(Castus4publicSchedule &schedule) {
    for (auto& item : schedule.schedule_items) {
        if (is_valid(item)) {
            update_timing(item);
        }
    }
}

/**
 * This function alters the timing of an item to restore the gap
 * that had existed prior to duration changes and rippling.
 *
 * \param current_item The item used for reference (only tags are modified)
 * \param next_item The item to modify
 */
// TODO(Alex): Find a better way of stashing information, so
//             that the first argument can be made `const`
void repair_gap(Castus4publicSchedule::ScheduleItem &current_item,
                Castus4publicSchedule::ScheduleItem &next_item) {
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
    // TODO(Alex): Find a better way of doing this, so the argument used
    //             as a point of reference is not mutated
    current_item.deleteValue("x-next-joined-gap-us");
    current_item.deleteValue("x-next-joined");
}

/**
 * This function takes a schedule and if any two sequential schedule items overlap
 * it adjusts the times of the second point
 *
 * \param schedule The Castus schedule
 **/
void ripple_connected_item(Castus4publicSchedule &schedule) {
    loop(schedule, repair_gap);
}

/**
 * This function moves a schedule item such that it no longer
 * overlaps with the preceding item, an action called "rippling".
 *
 * \param current_item The item to use as reference
 * \param next_item The item to modify
 */
void ripple_one(const Castus4publicSchedule::ScheduleItem &current_item,
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

/**
 * This function takes a schedule and if any two sequential schedule items overlap
 * it adjusts the times of the second time
 *
 * \param schedule The Castus schedule
 **/
void ripple_down_overlapping(Castus4publicSchedule &schedule) {
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
        // If the current item overlaps the next item by less than
        // 1,000,000 (what units? μs?), shorten it.
        if (c_end > n_start && c_end < (n_start + (Castus4publicSchedule::ideal_time_t)1000000)) {
            current_item.setEndTime(n_start);
        }
    };
    loop(schedule, trim_one);
}

