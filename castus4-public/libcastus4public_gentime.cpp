
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
#include <castus4-public/libcastus4public_gentime.h>

using namespace std;

#include <string>

static const char *castus4_schedule_dayofweek[7] = {
	"sun",
	"mon",
	"tue",
	"wed",
	"thu",
	"fri",
	"sat"
};

string castus4_schedule_print_time(int stm_type,const struct tm *stm,unsigned long stm_us) {
	char tmp[256],*w=tmp,*wf=tmp+sizeof(tmp)-1;

	if (stm == NULL) return "";

	/* do we need the next specifier? */
	if (stm_type == C4_SCHED_TYPE_DAILY) {
		if (stm->tm_hour >= 24) w += snprintf(w,(size_t)(wf-w),"next ");
	}
	else if (stm_type == C4_SCHED_TYPE_WEEKLY) {
		if (stm->tm_wday < 0) return "";
		if (stm->tm_wday >= 7) w += snprintf(w,(size_t)(wf-w),"next ");
		w += snprintf(w,(size_t)(wf-w),"%s ",castus4_schedule_dayofweek[stm->tm_wday%7]);
	}
	else if (stm_type == C4_SCHED_TYPE_MONTHLY) {
		if (stm->tm_mday > 31) w += snprintf(w,(size_t)(wf-w),"next ");
		w += snprintf(w,(size_t)(wf-w),"day %u ",stm->tm_mday);
	}
	else if (stm_type == C4_SCHED_TYPE_YEARLY) {
		if (stm->tm_mon > 12) w += snprintf(w,(size_t)(wf-w),"next ");
		w += snprintf(w,(size_t)(wf-w),"month %u day %u ",stm->tm_mon+1,stm->tm_mday);
	}

	/* hours, minutes, seconds */
	w += snprintf(w,(size_t)(wf-w),"%d:%02d:%02d",
		((stm->tm_hour+11)%12)+1,		/* 0, 1, 2, 3... -> 11, 0, 1, 2... -> 12, 1, 2, 3... */
		stm->tm_min,
		stm->tm_sec);

	if (stm_us != 0) {
		if ((stm_us % 10000UL) != 0)
			w += snprintf(w,(size_t)(wf-w),".%06lu",stm_us);
		else
			w += snprintf(w,(size_t)(wf-w),".%02lu",stm_us / 10000UL);
	}

	w += snprintf(w,(size_t)(wf-w)," %s ",
		(stm->tm_hour >= 12) ? "pm" : "am");

	assert(w <= wf);
	while ((w-1) >= tmp && w[-1] == ' ') w--;
	*w = 0;

	return tmp; /* converted to string() on return */
}

