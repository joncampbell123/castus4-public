
#define ENABLE_RIPPLE


#include <castus4-public/schedule.h>
#include <castus4-public/schedule_object.h>

#include "utils.h"
#include "utils_schedule.h"

using namespace std;

int main() {
	Castus4publicSchedule schedule;
    load(schedule);

    // step 1: add a tag for each item that is touching
    // TODO(Alex): The data this populates is not used here; remove?
    tag_touching_item(schedule);

    // step 2: check each item, read the duration from metadata, and update the duration from meta
    update_duration(schedule);

    // step 4: ripple down overlapping items
    ripple_down_overlapping(schedule);

    write(schedule);

	return 0;
}

