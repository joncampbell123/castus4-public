
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
#include <castus4-public/libcastus4public_schedule_helpers.h>

using namespace std;
using namespace Castus4publicScheduleHelpers;

int main(int argc,char **argv) {
	Castus4publicSchedule schedule;

	if (argc < 2) {
		fprintf(stderr,"loadschedule <schedule>\n");
		return 1;
	}

	if ( !load(schedule, argv[1]) ) {
		cerr << "Problem loading file " << argv[1] << endl;
		return 1;
	} 

	cout << "Schedule type: " << schedule.type() << endl;

	printf("Interval length: %u days\n",schedule.interval_length);

	printf("Global-level entries:\n");
	for (std::map<std::string,std::string>::iterator i=schedule.global_values.begin();
		i!=schedule.global_values.end();i++)
		printf("  %s = %s\n",i->first.c_str(),i->second.c_str());

	printf("Default item entries (type '%s'):\n",schedule.defaults_type.c_str());
	for (std::map<std::string,std::string>::iterator i=schedule.defaults_values.begin();
		i!=schedule.defaults_values.end();i++)
		printf("  %s = %s\n",i->first.c_str(),i->second.c_str());

	printf("Schedule blocks:\n");
	for (std::list<Castus4publicSchedule::ScheduleBlock>::iterator i=schedule.schedule_blocks.begin();
		i!=schedule.schedule_blocks.end();i++) {
		printf("  {\n");

		std::map<std::string,std::string> &block = (*i).entry;
		for (std::map<std::string,std::string>::iterator j=block.begin();j!=block.end();j++)
			printf("    %s = %s\n",j->first.c_str(),j->second.c_str());

		printf("  }\n");
	}

	printf("Schedule items:\n");
	for (std::list<Castus4publicSchedule::ScheduleItem>::iterator i=schedule.schedule_items.begin();
		i!=schedule.schedule_items.end();i++) {
		printf("  {\n");

		std::map<std::string,std::string> &block = (*i).entry;
		for (std::map<std::string,std::string>::iterator j=block.begin();j!=block.end();j++)
			printf("    %s = %s\n",j->first.c_str(),j->second.c_str());

		printf("  }\n");
	}

	return 0;
}

