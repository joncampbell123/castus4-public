
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <castus4-public/libcastus4public_schedule.h>
#include <castus4-public/libcastus4public_parsetime.h>
#include <castus4-public/libcastus4public_gentime.h>
#include <castus4-public/libcastus4public_chomp.h>
#include <castus4-public/libcastus4public_schedule_object.h>

namespace Castus4publicScheduleHelpers {
    bool load(Castus4publicSchedule &schedule, std::string file);
}
