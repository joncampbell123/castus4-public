
#include <castus4-public/schedule_object.h>
#include <iostream>

#include "utils_schedule.h"

using namespace std;

/**
 * Loads a schedule in from stdin
 */
void load(Castus4publicSchedule &schedule) {
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
}

/**
 * Verifies the start and end times are valid
 *
 * \param item The schedule item
 * \return true if valid, otherwise false
 */
bool is_valid(const Castus4publicSchedule::ScheduleItem &item) {
    return item.getStartTime() != Castus4publicSchedule::ideal_time_t_invalid &&
           item.getEndTime() != Castus4publicSchedule::ideal_time_t_invalid &&
           item.getStartTime() < item.getEndTime();
}

/**
 * Writes a schedule to stdout
 *
 * \param schedule The Castus schedule
 * \return false if there was an error, otherwise true
 */
bool write(Castus4publicSchedule &schedule) {
	schedule.sort_schedule_items();
	schedule.sort_schedule_blocks();
	if (!schedule.write_out(cout)) {
		fprintf(stderr,"Error while writing schedule\n");
        return false;
    }
    return true;
}

/**
 * Checks if an item is inside a block
 *
 * \param item The schedule item
 * \param block The block to check if `item` is within
 * \return true if the item is inside the block, false otherwise
 */
bool in_block(const Castus4publicSchedule::ScheduleItem& item,
        const Castus4publicSchedule::ScheduleBlock& block) {
    return(block.getStartTime() <= item.getStartTime() && item.getStartTime() < block.getEndTime());
}

/**
 * Loops over all the items in a schedule, two at a time, if they are both valid itpasses them
 * to a callback.
 *
 * \param schedule The castus schedule
 * \param logic A lamda that takes two schedule items.
 */
void loop(Castus4publicSchedule &schedule,
        std::function<void (Castus4publicSchedule::ScheduleItem &current_item,
                    Castus4publicSchedule::ScheduleItem &next_item)> logic) {
    // Skip an item at the start (it will be manually visited)
    for (auto schedule_item = ++schedule.schedule_items.begin();
              schedule_item !=  schedule.schedule_items.end();
              ++schedule_item) {
        // Stash the current iterator
        auto tmp = schedule_item;
        // Bind the "next item", walking the stashed iterator back a step
        auto& next = *(tmp--);
        // Bind the current item
        auto& current = *tmp;

        if (is_valid(current) && is_valid(next)) {
            logic(current, next);
        }
    }
}

/**
 * Loops over all the items in a schedule.  If it is valid it calls the lamda on it.
 *
 * \param schedule The castus schedule
 * \param logic A lamda that takes a schedule items.
 */
void loop(Castus4publicSchedule &schedule,
        std::function<void (Castus4publicSchedule::ScheduleItem &current_item)> logic) {
    for (auto schedule_item=schedule.schedule_items.begin();schedule_item!=schedule.schedule_items.end();schedule_item++)
        if (is_valid(*schedule_item)) logic(*schedule_item);
}
