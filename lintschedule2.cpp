
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

#include <iostream>

using namespace std;

int main(int argc,char **argv) {
	Castus4publicSchedule schedule;
	char line[1024];
	FILE *fp;

	if (argc < 2) {
		fprintf(stderr,"loadschedule <schedule>\n");
		return 1;
	}

	fp = fopen(argv[1],"r");
	if (!fp) {
		fprintf(stderr,"Cannot open schedule file %s\n",argv[1]);
		return 1;
	}

	schedule.begin_load();
	memset(line,0,sizeof(line));
	while (!feof(fp) && !ferror(fp)) {
		if (fgets(line,sizeof(line)-1,fp) == NULL) break;
		castus4public_chomp(line);
		schedule.load_take_line(line);
	}
	schedule.end_load();
	fclose(fp);

	if (!schedule.write_out(cout))
		fprintf(stderr,"Error while writing schedule\n");

	return 0;
}

