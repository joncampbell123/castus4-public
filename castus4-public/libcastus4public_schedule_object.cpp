
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
#include <castus4-public/libcastus4public_schedule_object.h>

#include <string>
#include <vector>
#include <list>
#include <map>

void Castus4publicSchedule::common_std_map_name_value_pair_entry(std::map<std::string,std::string> &entry,std::string &name,std::string &value) {
	std::map<std::string,std::string>::iterator entry_i = entry.find(name);
	if (entry_i == entry.end())
		entry[name] = value;
	else {
		entry_i->second += "\n";
		entry_i->second += value;
	}
}

Castus4publicSchedule::ScheduleItem::ScheduleItem() {
}

Castus4publicSchedule::ScheduleItem::~ScheduleItem() {
}

void Castus4publicSchedule::ScheduleItem::takeNameValuePair(std::string &name,std::string &value) {
	common_std_map_name_value_pair_entry(/*&*/entry,name,value);
}

Castus4publicSchedule::ScheduleBlock::ScheduleBlock() {
}

Castus4publicSchedule::ScheduleBlock::~ScheduleBlock() {
}

void Castus4publicSchedule::ScheduleBlock::takeNameValuePair(std::string &name,std::string &value) {
	common_std_map_name_value_pair_entry(/*&*/entry,name,value);
}

Castus4publicSchedule::Castus4publicSchedule() {
	reset();
}

Castus4publicSchedule::~Castus4publicSchedule() {
}

void Castus4publicSchedule::reset() {
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

void Castus4publicSchedule::begin_load() {
	reset();
	head = true;
	in_entry = false;
	entry_mode = Global;
}

void Castus4publicSchedule::end_load() {
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

void Castus4publicSchedule::load_take_line(const char *line) {
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
					schedule_items.push_back(ScheduleItem());
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
					entry_mode = ScheduleBlockItem;
					schedule_blocks.push_back(ScheduleBlock());
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
					case Global:
						common_std_map_name_value_pair_entry(/*&*/global_values,name,value);
						break;
					case Defaults:
						common_std_map_name_value_pair_entry(/*&*/defaults_values,name,value);
						break;
					case ScheduleBlockItem:
						assert(!schedule_blocks.empty());
						schedule_blocks.back().takeNameValuePair(name,value);
						break;
					case Item:
						assert(!schedule_items.empty());
						schedule_items.back().takeNameValuePair(name,value);
						break;
				}
			}
		}
	}
}

