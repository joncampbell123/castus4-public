
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <castus4-public/parsetime.h>

using namespace std;

int main(int argc,char **argv) {
	unsigned long rtm_us; // microsecond component
	int rtm_type = -1; // schedule type, from parsing string
	struct tm rtm; // time, in seconds

	if (argc < 2) {
		fprintf(stderr,"parsetime <timespec>\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"Where <timespec> is a date/time specifier in Castus format\n");
		return 1;
	}

	rtm = castus4_schedule_parse_time(argv[1],&rtm_us,&rtm_type);
	printf("Result:\n");
	printf("        Type: %d\n",rtm_type);			/* C4_SCHED_TYPE_... */
	printf("        Year: %u\n",rtm.tm_year + 1900);	/* 1900... */
	printf("       Month: %u\n",rtm.tm_mon + 1);		/* 0..11 -> 1..12 */
	printf("Day of month: %u\n",rtm.tm_mday);		/* 1..31 */
	printf("     Weekday: %u\n",rtm.tm_wday);		/* 0..6 sun..sat if specifier is for weekly schedules */
	printf("        Hour: %u\n",rtm.tm_hour);		/* 0..23 */
	printf("      Minute: %u\n",rtm.tm_min);		/* 0..59 */
	printf("      Second: %u\n",rtm.tm_sec);		/* 0..59 */
	printf(" Microsecond: %lu\n",rtm_us);			/* 0..999999 */
	return 0;
}

