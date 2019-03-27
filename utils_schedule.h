#include <castus4-public/parsetime.h>
#include <castus4-public/gentime.h>
#include <castus4-public/chomp.h>
#include <castus4-public/metadata.h>
#include <castus4-public/schedule_object.h>
#include <functional>

void load(Castus4publicSchedule &schedule);
bool is_valid(const Castus4publicSchedule::ScheduleItem &item);
bool write(Castus4publicSchedule &schedule);
void loop(Castus4publicSchedule &schedule,
        std::function<void (Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item)> logic);
void loop(Castus4publicSchedule &schedule,
        std::function<void (Castus4publicSchedule::ScheduleItem &current_item)> logic);