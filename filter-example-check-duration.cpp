
#include <castus4-public/schedule.h>
#include <castus4-public/schedule_object.h>

#include "utils.h"

using namespace std;

int main() {
	Castus4publicSchedule schedule;
    load(schedule);


    // step 2: check each item, read the duration from metadata, and update the duration from meta
    update_duration(schedule);

    /* if the items overlap only SLIGHTLY, then go ahead and trim back a bit */
    trim_overlapping(schedule);

    write(schedule);

	return 0;
}
