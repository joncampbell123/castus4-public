
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

using namespace std;

#include <string>
#include <vector>
#include <list>
#include <map>

/* C/C++ equivalent of Perl's chomp function TODO move out */
void castus4public_chomp(char *s) {
	char *orig_s = s;
	size_t l = strlen(s);
	if (l == 0) return;
	s += l - 1;
	while (s > orig_s && (*s == '\n' || *s == '\r')) *s-- = 0;
}

class Castus4publicSchedule {
public:
	enum entry_parse_mode {
		Global=0,
		Item,
		ScheduleBlock,
		Defaults,
		Unknown
	};
public:
	Castus4publicSchedule() {
		reset();
	}
	virtual ~Castus4publicSchedule() {
	}
	void reset() {
		schedule_type = C4_SCHED_TYPE_NONE;
		schedule_blocks.clear();
		defaults_values.clear();
		schedule_items.clear();
		defaults_type.clear();
		interval_length = 0;
		entry.clear();
		head = false;
		in_entry = false;
	}
	void begin_load() {
		reset();
		head = true;
		in_entry = false;
		entry_mode = Global;
	}
	void end_load() {
		if (schedule_type == C4_SCHED_TYPE_NONE)
			schedule_type = C4_SCHED_TYPE_WEEKLY;

		if (schedule_type == C4_SCHED_TYPE_DAILY)
			interval_length = 1;
		else if (schedule_type == C4_SCHED_TYPE_WEEKLY)
			interval_length = 7;
		else if (schedule_type == C4_SCHED_TYPE_MONTHLY)
			interval_length = 31;
		else if (schedule_type == C4_SCHED_TYPE_YEARLY)
			interval_length = 12*31;
	}
	void load_take_line(const char *line) {
		if (*line == '*') {
			if (head && schedule_type == C4_SCHED_TYPE_NONE) {
				/* Castus originally started with weekly schedules. Then v3.0 added monthly, yearly, daily, etc. and v4.0 added interval schedules */
				if (!strcasecmp(line,"*monthly"))
					schedule_type = C4_SCHED_TYPE_MONTHLY;
				else if (!strcasecmp(line,"*yearly"))
					schedule_type = C4_SCHED_TYPE_YEARLY;
				else if (!strcasecmp(line,"*daily"))
					schedule_type = C4_SCHED_TYPE_DAILY;
				else if (!strcasecmp(line,"*interval"))
					schedule_type = C4_SCHED_TYPE_INTERVAL;
				else if (!strcasecmp(line,"*weekly"))
					schedule_type = C4_SCHED_TYPE_WEEKLY;
			}
		}
		else {
			while (*line == ' ' || *line == '\t') line++;

			/* ignore comments */
			if (*line == '#') return;

			/* entry/exit blocks */
			{
				const char *curly = in_entry ? NULL : strrchr(line,'{');
				const char *equ = strchr(line,'=');

				if (curly != NULL && !strcmp(curly,"{")) { /* must end in { */
					/* eat whitespace at the end */
					while (curly > line && curly[-1] == ' ') curly--;
					entry = std::string(line,(size_t)(curly-line));
					in_entry = true;

					if (entry == "") {
						entry_mode = Item;
						schedule_items.push_back(std::map<std::string,std::string>());
					}
					else if (!strncasecmp(entry.c_str(),"defaults,",9)) {
						const char *s = entry.c_str()+9;
						while (*s == ' ') s++;
						entry_mode = Defaults;
						defaults_type = s;

						if (schedule_type == C4_SCHED_TYPE_NONE) {
							if (defaults_type == "of the day")
								schedule_type = C4_SCHED_TYPE_DAILY;
							else if (defaults_type == "day of the week")
								schedule_type = C4_SCHED_TYPE_WEEKLY;
							else if (defaults_type == "day of the month")
								schedule_type = C4_SCHED_TYPE_MONTHLY;
							else if (defaults_type == "day of the year")
								schedule_type = C4_SCHED_TYPE_YEARLY;
							else if (defaults_type == "day of the interval")
								schedule_type = C4_SCHED_TYPE_INTERVAL;
						}
					}
					else if (entry == "schedule block") {
						entry_mode = ScheduleBlock;
						schedule_blocks.push_back(std::map<std::string,std::string>());
					}
					else {
						entry_mode = Unknown;
					}
				}
				else if (*line == '}') {
					entry_mode = Global;
					in_entry = false;
					entry.clear();
				}
				else if (equ != NULL) {
					/* global level name = value pair */
					const char *ns = equ - 1;
					const char *vs = equ + 1;

					while (*vs == ' ') vs++;
					while (ns > line && *ns == ' ') ns--;
					ns++;

					std::string name = std::string(line,(size_t)(ns-line));
					std::string value = vs;

					/* NTS: if there are multiple lines with the same name, we combine them
					 *      in the same way the Server Sent Events do, concat together with
					 *      newlines. */
					switch (entry_mode) {
						case Global: {
							std::map<std::string,std::string>::iterator global_i = global_values.find(name);
							if (global_i == global_values.end())
								global_values[name] = value;
							else {
								global_i->second += "\n";
								global_i->second += value;
							}
							} break;
						case Defaults: {
							std::map<std::string,std::string>::iterator defaults_i = defaults_values.find(name);
							if (defaults_i == defaults_values.end())
								defaults_values[name] = value;
							else {
								defaults_i->second += "\n";
								defaults_i->second += value;
							}
							} break;
						case ScheduleBlock: {
							assert(!schedule_blocks.empty());
							std::map<std::string,std::string> &entry = schedule_blocks.back();
							std::map<std::string,std::string>::iterator entry_i = entry.find(name);
							if (entry_i == entry.end())
								entry[name] = value;
							else {
								entry_i->second += "\n";
								entry_i->second += value;
							}
							} break;
						case Item: {
							assert(!schedule_items.empty());
							std::map<std::string,std::string> &entry = schedule_items.back();
							std::map<std::string,std::string>::iterator entry_i = entry.find(name);
							if (entry_i == entry.end())
								entry[name] = value;
							else {
								entry_i->second += "\n";
								entry_i->second += value;
							}
							} break;
					}
				}
			}
		}
	}
public:
	bool						head;
	std::string					entry;			// if within { ... } block
	bool						in_entry;
	enum entry_parse_mode				entry_mode;
// parsed output
	std::list< std::map<std::string,std::string> >	schedule_items;
	std::list< std::map<std::string,std::string> >	schedule_blocks;
	std::map<std::string,std::string>		defaults_values;
	std::string					defaults_type;
	std::map<std::string,std::string>		global_values;
	int						schedule_type;
	int						interval_length;
};

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

	printf("Schedule type: ");
	switch (schedule.schedule_type) {
		case C4_SCHED_TYPE_NONE:	printf("None"); break;
		case C4_SCHED_TYPE_DAILY:	printf("Daily"); break;
		case C4_SCHED_TYPE_WEEKLY:	printf("Weekly"); break;
		case C4_SCHED_TYPE_MONTHLY:	printf("Monthly"); break;
		case C4_SCHED_TYPE_YEARLY:	printf("Yearly"); break;
		case C4_SCHED_TYPE_INTERVAL:	printf("Interval"); break;
	};
	printf("\n");
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
	for (std::list< std::map<std::string,std::string> >::iterator i=schedule.schedule_blocks.begin();
		i!=schedule.schedule_blocks.end();i++) {
		printf("  {\n");

		std::map<std::string,std::string> &block = *i;
		for (std::map<std::string,std::string>::iterator j=block.begin();j!=block.end();j++)
			printf("    %s = %s\n",j->first.c_str(),j->second.c_str());

		printf("  }\n");
	}

	printf("Schedule items:\n");
	for (std::list< std::map<std::string,std::string> >::iterator i=schedule.schedule_items.begin();
		i!=schedule.schedule_items.end();i++) {
		printf("  {\n");

		std::map<std::string,std::string> &block = *i;
		for (std::map<std::string,std::string>::iterator j=block.begin();j!=block.end();j++)
			printf("    %s = %s\n",j->first.c_str(),j->second.c_str());

		printf("  }\n");
	}

	return 0;
}

