
#include <time.h>	// for struct tm

struct tm castus4_schedule_parse_time(const char *v,unsigned long *sub_us,int *type);

enum {
	C4_SCHED_TYPE_NONE=-1,
	C4_SCHED_TYPE_DAILY=0,
	C4_SCHED_TYPE_WEEKLY=1,
	C4_SCHED_TYPE_MONTHLY=2,
	C4_SCHED_TYPE_YEARLY=3
};

