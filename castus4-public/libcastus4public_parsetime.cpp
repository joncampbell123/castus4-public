
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <castus4-public/libcastus4public_parsetime.h>

struct tm castus4_schedule_parse_time(const char *v,unsigned long *sub_us,int *sched_type) {
	int pm = 0,ampm = 0,next = 0,next_type = 0/*daily schedule*/;
	struct tm t;

	memset(&t,0,sizeof(t));
	t.tm_mday = 1;
	t.tm_mon = 0;

	if (sched_type != NULL)
		*sched_type = C4_SCHED_TYPE_WEEKLY;

	if (sub_us != NULL)
		*sub_us = 0UL;

	/* v1.0/v2.0/v2.1 compat: "sun" "mon" "tue" etc. day of the week (even in foreign calendars)
	 *                        hh:mm[:ss] [am|pm]
	 * new: day of month         i.e. the 5th = 5 of month
	 *      day of year          i.e. december 25th = 12/25 */
	/* added v4.1: "next" modifier so that it's possible to encode "sun 12:00 am" vs "sun 12:00 am next"
	 * to cover a week entirely */

	while (*v) {
		while (*v == ' ') v++;
		if (*v == 0) break;

		/* next modifier */
		if (!strncasecmp(v,"next",4) && (v[4] == 0 || v[4] == ' ')) {
			next++; v += 4; while (*v == ' ') v++;
		}
		/* month [m] */
		else if (!strncasecmp(v,"month",5) && (v[5] == 0 || v[5] == ' ')) {
			v += 5; while (*v == ' ') v++;
			t.tm_mon = (int)strtol(v,(char**)(&v),0) - 1;
			if (t.tm_mon < 0) t.tm_mon = 0;
			else if (t.tm_mon > 11) t.tm_mon = 11;
			while (*v == ' ') v++;
			next_type = 3; /* yearly schedule */
		}
		/* day [d] */
		else if (!strncasecmp(v,"day",3) && (v[3] == 0 || v[3] == ' ')) {
			v += 3; while (*v == ' ') v++;
			t.tm_mday = (int)strtol(v,(char**)(&v),0);
			if (t.tm_mday < 1) t.tm_mday = 1;
			else if (t.tm_mday > 31) t.tm_mday = 31;
			while (*v == ' ') v++;
			if (next_type < 2) next_type = 2; /* monthly schedule */
		}
		else if (!strncasecmp(v,"sun",3)) {
			v += 3; while (*v && *v != ' ') v++; while (*v == ' ') v++;
			t.tm_wday = 0;
			next_type = 1; /* weekly schedule */
		}
		else if (!strncasecmp(v,"mon",3)) {
			v += 3; while (*v && *v != ' ') v++; while (*v == ' ') v++;
			t.tm_wday = 1;
			next_type = 1; /* weekly schedule */
		}
		else if (!strncasecmp(v,"tue",3)) {
			v += 3; while (*v && *v != ' ') v++; while (*v == ' ') v++;
			t.tm_wday = 2;
			next_type = 1; /* weekly schedule */
		}
		else if (!strncasecmp(v,"wed",3)) {
			v += 3; while (*v && *v != ' ') v++; while (*v == ' ') v++;
			t.tm_wday = 3;
			next_type = 1; /* weekly schedule */
		}
		else if (!strncasecmp(v,"thu",3)) {
			v += 3; while (*v && *v != ' ') v++; while (*v == ' ') v++;
			t.tm_wday = 4;
			next_type = 1; /* weekly schedule */
		}
		else if (!strncasecmp(v,"fri",3)) {
			v += 3; while (*v && *v != ' ') v++; while (*v == ' ') v++;
			t.tm_wday = 5;
			next_type = 1; /* weekly schedule */
		}
		else if (!strncasecmp(v,"sat",3)) {
			v += 3; while (*v && *v != ' ') v++; while (*v == ' ') v++;
			t.tm_wday = 6;
			next_type = 1; /* weekly schedule */
		}
		else if (!strncasecmp(v,"am",2) && (v[2] == 0 || v[2] == ' ')) {
			v += 2; while (*v == ' ') v++; pm = 0; ampm = 1;
		}
		else if (!strncasecmp(v,"pm",2) && (v[2] == 0 || v[2] == ' ')) {
			v += 2; while (*v == ' ') v++; pm = 1; ampm = 1;
		}
		else if (isdigit(*v)) {
			/* v1.0/v2.0/v2.1 compat: hh[:mm[:ss]]
			 *    or
			 * new "N of month" day of the month specifier
			 *    or
			 * month/day specifier for yearly schedule */
			const char *pdigit = v;
			while (isdigit(*pdigit)) pdigit++;
			while (*pdigit == ' ') pdigit++;
			if (!strncasecmp(pdigit,"of month",8)) {
				if (next_type < 2) next_type = 2; /* monthly schedule */
				t.tm_mday = strtol(v,NULL,10);
				v = pdigit + 8;
			}
			else if (*pdigit == '/') {
				next_type = 3; /* yearly schedule */
				t.tm_mon = strtol(v,NULL,10) - 1;
				v = pdigit + 1;
				t.tm_mday = strtol(v,(char**)(&v),10);
				while (*v && *v != ' ') v++;
				while (*v == ' ') v++;
			}
			else {
				if (sub_us != NULL) *sub_us = 0UL;
				t.tm_hour = strtol(v,(char**)(&v),10);
				if (*v == ':') v++;
				t.tm_min = strtol(v,(char**)(&v),10);
				if (*v == ':') v++;
				t.tm_sec = strtol(v,(char**)(&v),10);
				if (*v == '.' && sub_us != NULL) {
					unsigned long mult = 100000UL;

					v++;
					while (isdigit(*v)) {
						if (mult == 0UL) break;
						*sub_us += ((unsigned long)(*v - '0')) * mult;
						mult /= 10UL;
						v++;
					}
				}
				while (*v && *v != ' ') v++;
				while (*v == ' ') v++;
			}
		}
		else {
			while (*v && *v != ' ') v++;
			while (*v == ' ') v++;
		}
	}

	if (ampm) t.tm_hour %= 12;
	else t.tm_hour %= 24;
	if (pm) t.tm_hour += 12;

	if (next > 0) {
		switch (next_type) {
			case 0: t.tm_hour += 24; /* daily */ break;
			case 1: t.tm_wday += 7; /* weekly */ break;
			case 2: t.tm_mday += 31; /* monthly */ break;
			case 3: t.tm_mon += 12; /* yearly */ break;
		}
	}

	if (sched_type)
		*sched_type = next_type;

	return t;
}

