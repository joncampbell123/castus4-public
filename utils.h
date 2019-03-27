
#ifndef UTILS
#define UTILS
#include <castus4-public/schedule.h>

void tag_touching_item(Castus4publicSchedule &schedule);
void update_duration(Castus4publicSchedule &schedule);
void ripple_connected_item(Castus4publicSchedule &schedule);
void trim_overlapping(Castus4publicSchedule &schedule);
void ripple_down_overlapping(Castus4publicSchedule &schedule);

#endif
