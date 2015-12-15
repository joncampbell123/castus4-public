
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

int main(int argc,char **argv) {
	unsigned long rtm_us = 0; // microsecond component
	int rtm_type = -1; // schedule type, from parsing string
	struct tm rtm; // time, in seconds
	char *a;
	int i;

	if (argc < 2) {
		fprintf(stderr,"gentime [options]\n");
		fprintf(stderr,"\n");
		fprintf(stderr," -t <n>                    Schedule type 0=daily 1=weekly 2=monthly 3=yearly\n");
		fprintf(stderr," -year <year>              Year (1900...)\n");
		fprintf(stderr," -month <month>            Month (1..12)\n");
		fprintf(stderr," -mday <day of month>      Day of month (1..31)\n");
		fprintf(stderr," -wday <day of week>       Day of week (0..6 for sun..sat)\n");
		fprintf(stderr," -hour <hour>              Hour (0..23)\n");
		fprintf(stderr," -minute <minute>          Minute (0..59)\n");
		fprintf(stderr," -second <second>          Second (0..59)\n");
		return 1;
	}

	memset(&rtm,0,sizeof(rtm));
	for (i=1;i < argc;) {
		a = argv[i++];

		if (*a == '-') {
			do { a++; } while (*a == '-');

			if (!strcmp(a,"t")) {
				rtm_type = atoi(argv[i++]);
			}
			else if (!strcmp(a,"year")) {
				rtm.tm_year = atoi(argv[i++]) - 1900;
				if (rtm_type < 0) rtm_type = C4_SCHED_TYPE_YEARLY;
			}
			else if (!strcmp(a,"month")) {
				rtm.tm_mon = atoi(argv[i++]) - 1;
				if (rtm_type < 0) rtm_type = C4_SCHED_TYPE_YEARLY;
			}
			else if (!strcmp(a,"mday")) {
				rtm.tm_mday = atoi(argv[i++]);
				if (rtm_type < 0) rtm_type = C4_SCHED_TYPE_MONTHLY;
			}
			else if (!strcmp(a,"wday")) {
				rtm.tm_wday = atoi(argv[i++]);
				if (rtm_type < 0) rtm_type = C4_SCHED_TYPE_WEEKLY;
			}
			else if (!strcmp(a,"hour")) {
				rtm.tm_hour = atoi(argv[i++]);
			}
			else if (!strcmp(a,"minute")) {
				rtm.tm_min = atoi(argv[i++]);
			}
			else if (!strcmp(a,"second")) {
				double f = atof(argv[i++]);
				rtm.tm_sec = (int)floor(f);
				rtm_us = (unsigned long)((f - rtm.tm_sec) * 1000000);
			}
			else {
				fprintf(stderr,"Unknown switch '%s'\n",a);
			}
		}
		else {
			fprintf(stderr,"Unknown arg '%s'\n",a);
			return 1;
		}
	}

	if (rtm_type < 0)
		rtm_type = C4_SCHED_TYPE_DAILY;

	printf("Input:\n");
	printf("        Type: %d\n",rtm_type);			/* C4_SCHED_TYPE_... */
	printf("        Year: %u\n",rtm.tm_year + 1900);	/* 1900... */
	printf("       Month: %u\n",rtm.tm_mon + 1);		/* 0..11 -> 1..12 */
	printf("Day of month: %u\n",rtm.tm_mday);		/* 1..31 */
	printf("     Weekday: %u\n",rtm.tm_wday);		/* 0..6 sun..sat if specifier is for weekly schedules */
	printf("        Hour: %u\n",rtm.tm_hour);		/* 0..23 */
	printf("      Minute: %u\n",rtm.tm_min);		/* 0..59 */
	printf("      Second: %u\n",rtm.tm_sec);		/* 0..59 */
	printf(" Microsecond: %lu\n",rtm_us);			/* 0..999999 */

	string result = castus4_schedule_print_time(rtm_type,&rtm,rtm_us);

	printf("Output:\n");
	printf("'%s'\n",result.c_str());

	return 0;
}

