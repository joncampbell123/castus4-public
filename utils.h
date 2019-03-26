
#ifndef UTILS
#define UTILS
#include <castus4-public/schedule.h>

void load(Castus4publicSchedule &schedule);
bool write(Castus4publicSchedule &schedule);
bool is_valid(const Castus4publicSchedule::ScheduleItem &item);

void tag_touching_item(Castus4publicSchedule &schedule, const Castus4publicSchedule::ideal_time_t min_blank_interval);
void update_duration(Castus4publicSchedule &schedule);
void ripple_connected_item(Castus4publicSchedule &schedule);
void trim_overlapping(Castus4publicSchedule &schedule);
void ripple_down_overlapping(Castus4publicSchedule &schedule);

#endif
