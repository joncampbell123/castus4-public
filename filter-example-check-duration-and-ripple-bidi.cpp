
#include <castus4-public/schedule.h>
#include <castus4-public/schedule_object.h>

#include "utils.h"

using namespace std;

int main() {
    const Castus4publicSchedule::ideal_time_t min_blank_interval = 1000000; /* 1000000us = 1000ms = 1 sec */
	Castus4publicSchedule schedule;
    load(schedule);


    // step 1: add a tag for each item that is touching
    tag_touching_item(schedule);

    // step 2: check each item, read the duration from metadata, and update the duration from meta
    update_duration(schedule);

    // step 3: ripple up or down connected items
    ripple_connected_item(schedule);
    // step 4: ripple down overlapping items
    ripple_down_overlapping(schedule);

    write(schedule);

	return 0;
}

