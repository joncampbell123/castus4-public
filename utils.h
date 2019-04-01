
#ifndef UTILS
#define UTILS
#include <castus4-public/schedule.h>

void tag_touching_item(Castus4publicSchedule &schedule,
    Castus4publicSchedule::ideal_time_t min_blank_interval);
void update_duration(Castus4publicSchedule &schedule);
void ripple_connected_item(Castus4publicSchedule &schedule);
void trim_overlapping(Castus4publicSchedule &schedule);
void ripple_down_overlapping(Castus4publicSchedule &schedule);
void update_timing(Castus4publicSchedule::ScheduleItem& schedule_item);
void ripple_one(const Castus4publicSchedule::ScheduleItem &current_item,
                Castus4publicSchedule::ScheduleItem &next_item);
void repair_gap(Castus4publicSchedule::ScheduleItem &current_item,
                Castus4publicSchedule::ScheduleItem &next_item);

#endif
