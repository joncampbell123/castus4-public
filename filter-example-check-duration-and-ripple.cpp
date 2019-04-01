
#define ENABLE_RIPPLE


#include <castus4-public/schedule.h>
#include <castus4-public/schedule_object.h>

#include "utils.h"
#include "utils_schedule.h"

using namespace std;

int main() {
	Castus4publicSchedule schedule;
    load(schedule);

    // step 1: check each item, read the duration from metadata, and update the duration from meta
    update_duration(schedule);

    // step 2: ripple down overlapping items
    ripple_down_overlapping(schedule);

    write(schedule);

	return 0;
}

