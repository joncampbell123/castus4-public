
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <castus4-public/schedule.h>
#include <castus4-public/parsetime.h>
#include <castus4-public/gentime.h>
#include <castus4-public/schedule_object.h>

#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <map>

const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_microsecond = 1;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_second      = 1000000 * ideal_microsecond;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_minute      = 60      * ideal_second;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_hour        = 60      * ideal_minute;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_day         = 24      * ideal_hour;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_week        = 7       * ideal_day;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_month       = 31      * ideal_day;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_year        = 12      * ideal_month;

const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_time_t_invalid   = (ideal_time_t)(-1LL);
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_microsec_per_sec = 1000000;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_sec_per_min      = 60;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_min_per_hour     = 60;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_hour_per_day     = 24;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_day_per_week     = 7;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_day_per_month    = 31;
const Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ideal_month_per_year   = 12;

void Castus4publicSchedule::common_std_map_name_value_pair_entry(std::map<std::string,std::string> &entry,const std::string &name,const std::string &value) {
	std::map<std::string,std::string>::iterator entry_i = entry.find(name);
	if (entry_i == entry.end())
		entry[name] = value;
	else {
		entry_i->second += "\n";
		entry_i->second += value;
	}
}

Castus4publicSchedule::ScheduleItem::ScheduleItem(const int schedule_type) : schedule_type(schedule_type) {
}

Castus4publicSchedule::ScheduleItem::~ScheduleItem() {
}

void Castus4publicSchedule::ScheduleItem::takeNameValuePair(const std::string &name,const std::string &value) {
	common_std_map_name_value_pair_entry(/*&*/entry,name,value);
}

Castus4publicSchedule::ScheduleBlock::ScheduleBlock(const int schedule_type) : schedule_type(schedule_type) {
}

Castus4publicSchedule::ScheduleBlock::~ScheduleBlock() {
}

void Castus4publicSchedule::ScheduleBlock::takeNameValuePair(const std::string &name,const std::string &value) {
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
					schedule_items.push_back(ScheduleItem(schedule_type));
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
					schedule_blocks.push_back(ScheduleBlock(schedule_type));
				}
				else if (entry == "triggers") {
					entry_mode = Triggers;
				}
				else if (entry == "intervals") {
					entry_mode = Intervals;
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
					case Unknown:
						break;
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
					case Triggers:
					case Intervals:
						// Find where the index info of the trigger is
						const char* open_square  = strchr(line, '[');
						// const char* close_square = strchr(line, ']');
						// Parse out the trigger name
						std::string trigger(line, open_square-line);
						// Parse out the trigger index
						// TODO(Alex): Enforce that the trigger indices are sequential and start at zero
						//             for a given trigger
						//int index = stoi(std::string(open_square+1, open_square+1-close_square));
						if (entry_mode == Triggers) {
							schedule_triggers.insert(std::pair<std::string, std::string>(trigger, value));
						} else if (entry_mode == Intervals) {
							schedule_intervals.insert(std::pair<std::string, std::string>(trigger, value));
						}
						break;
				}
			}
		}
	}
}

bool Castus4publicSchedule::write_out(FILE *fp) {
	if (fp == NULL) return false;
	return write_out(&write_out_stdio_cb,(void*)fp);
}

bool Castus4publicSchedule::write_out_stdio_cb(Castus4publicSchedule *_this,const char *line,void *opaque) {
	FILE *fp = (FILE*)opaque;
	if (fp == NULL) return false;
	if (feof(fp) || ferror(fp)) return false;
	if (fputs(line,fp) == EOF) return false;
	return true;
}

bool Castus4publicSchedule::write_out(std::ostream &cout) {
	return write_out(&write_out_iostream_cb,(void*)(&cout));
}

bool Castus4publicSchedule::write_out_iostream_cb(Castus4publicSchedule *_this,const char *line,void *opaque) {
	std::ostream *hax_cout = (std::ostream*)opaque;
	// FIXME: How do we detect error on output?
	*hax_cout << line;
	return true;
}

bool Castus4publicSchedule::write_out_name_value_pair(const std::string &name,const std::string &value,writeout_cb_t f,void *opaque,bool tab,bool spcequ) {
	std::string line;
	const char *s,*n;

	s = value.c_str();
	while (s != NULL) {
		n = strchr(s,'\n');

		line.clear();
		if (tab) line += '\t';

		if (n != NULL) {
			line += name + (spcequ ? " = " : "=") + std::string(s,(size_t)(n-s)) + "\n";
			n++;
		}
		else {
			line += name + (spcequ ? " = " : "=") + s + "\n";
		}

		if (!f(this,line.c_str(),opaque)) return false;
		s = n;
	}

	return true;
}

bool Castus4publicSchedule::write_out(writeout_cb_t f,void *opaque) {
	std::string line;

	if (schedule_type == C4_SCHED_TYPE_NONE) return false;

	switch (schedule_type) {
		case C4_SCHED_TYPE_DAILY:
			if (!f(this,"*daily\n",opaque)) return false;
			if (!f(this,"defaults, of the day{\n",opaque)) return false;
			break;
		case C4_SCHED_TYPE_WEEKLY:
			if (!f(this,"*weekly\n",opaque)) return false;
			if (!f(this,"defaults, day of the week{\n",opaque)) return false;
			break;
		case C4_SCHED_TYPE_MONTHLY:
			if (!f(this,"*monthly\n",opaque)) return false;
			if (!f(this,"defaults, day of the month{\n",opaque)) return false;
			break;
		case C4_SCHED_TYPE_YEARLY:
			if (!f(this,"*yearly\n",opaque)) return false;
			if (!f(this,"defaults, day of the year{\n",opaque)) return false;
			break;
		case C4_SCHED_TYPE_INTERVAL:
			if (!f(this,"*interval\n",opaque)) return false;
			if (!f(this,"defaults, day of the interval{\n",opaque)) return false;
			break;
	}

	for (std::map<std::string,std::string>::iterator i=defaults_values.begin();i!=defaults_values.end();i++) {
		if (!write_out_name_value_pair(i->first,i->second,f,opaque,/*tab=*/true,/*spcequ*/false)) return false;
	}

	if (!f(this,"}\n",opaque)) return false;

	for (std::map<std::string,std::string>::iterator i=global_values.begin();i!=global_values.end();i++) {
		if (!write_out_name_value_pair(i->first,i->second,f,opaque,/*tab=*/false,/*spcequ*/true)) return false;
	}

	for (std::list<ScheduleBlock>::iterator i=schedule_blocks.begin();i!=schedule_blocks.end();i++) {
		if (!f(this,"schedule block {\n",opaque)) return false;

		std::map<std::string,std::string> &block = (*i).entry;
		for (std::map<std::string,std::string>::iterator j=block.begin();j!=block.end();j++) {
			if (!write_out_name_value_pair(j->first,j->second,f,opaque,/*tab=*/true,/*spcequ*/false)) return false;
		}

		if (!f(this,"}\n",opaque)) return false;
	}

	for (std::list<ScheduleItem>::iterator i=schedule_items.begin();i!=schedule_items.end();i++) {
		if (!f(this,"{\n",opaque)) return false;

		std::map<std::string,std::string> &block = (*i).entry;
		for (std::map<std::string,std::string>::iterator j=block.begin();j!=block.end();j++) {
			if (!write_out_name_value_pair(j->first,j->second,f,opaque,/*tab=*/true,/*spcequ*/false)) return false;
		}

		if (!f(this,"}\n",opaque)) return false;
	}

	if (!schedule_triggers.empty()) {
		if (!f(this, "triggers {\n", opaque)) return false;
		std::multimap<std::string, std::string>::iterator prev = schedule_triggers.end();
		int counter = 0;
		for (auto trigger = schedule_triggers.begin(); trigger != schedule_triggers.end(); ++trigger) {
			if (prev != schedule_triggers.end() && prev->first != trigger->first) {
				counter = 0;
			}
			auto written = write_out_name_value_pair(
				trigger->first + "[" + std::to_string(counter) + "]",
				trigger->second,
				f,
				opaque,
				/*tab=*/true,
				/*spcequ*/false
			);
			if (!written) return false;
			prev = trigger;
		}
		if (!f(this,"}\n",opaque)) return false;
	}

	if (!schedule_intervals.empty()) {
		if (!f(this, "intervals {\n", opaque)) return false;
		std::multimap<std::string, std::string>::iterator prev = schedule_triggers.end();
		int counter = 0;
		for (auto interval = schedule_intervals.begin(); interval != schedule_intervals.end(); ++interval) {
			if (prev != schedule_intervals.end() && prev->first != interval->first) {
				counter = 0;
			}
			auto written = write_out_name_value_pair(
				interval->first + "[" + std::to_string(counter) + "]",
				interval->second,
				f,
				opaque,
				/*tab=*/true,
				/*spcequ*/false
			);
			if (!written) return false;
			prev = interval;
		}
		if (!f(this,"}\n",opaque)) return false;
	}

	return true;
}

void Castus4publicSchedule::sort_schedule_items() {
	schedule_items.sort();
}

void Castus4publicSchedule::sort_schedule_blocks() {
	schedule_blocks.sort();
}

void Castus4publicSchedule::sort_schedule_items_rev() { // TESTING only
	schedule_items.sort();
	schedule_items.reverse();
}

void Castus4publicSchedule::sort_schedule_blocks_rev() { // TESTING only
	schedule_blocks.sort();
	schedule_blocks.reverse();
}

const char *Castus4publicSchedule::ScheduleItem::getValue(const char *name) const {
	std::map<std::string,std::string>::const_iterator i = entry.find(name);
	if (i == entry.end()) return NULL;
	return i->second.c_str();
}

void Castus4publicSchedule::ScheduleItem::setValue(const char *name,const char *value) {
	entry[name] = value;
}

void Castus4publicSchedule::ScheduleItem::setValue(const char *name,const std::string &value) {
	entry[name] = value;
}

void Castus4publicSchedule::ScheduleItem::deleteValue(const char *name) {
	std::map<std::string,std::string>::iterator i = entry.find(name);
	if (i != entry.end()) entry.erase(i);
}

const char *Castus4publicSchedule::ScheduleBlock::getValue(const char *name) const {
	std::map<std::string,std::string>::const_iterator i = entry.find(name);
	if (i == entry.end()) return NULL;
	return i->second.c_str();
}

void Castus4publicSchedule::ScheduleBlock::setValue(const char *name,const char *value) {
	entry[name] = value;
}

void Castus4publicSchedule::ScheduleBlock::setValue(const char *name,const std::string &value) {
	entry[name] = value;
}

void Castus4publicSchedule::ScheduleBlock::deleteValue(const char *name) {
	std::map<std::string,std::string>::iterator i = entry.find(name);
	if (i != entry.end()) entry.erase(i);
}

Castus4publicSchedule::ideal_time_t Castus4publicSchedule::time_tm_to_ideal_time(const struct tm &t,const unsigned long usec,const int schedule_type) {
	ideal_time_t res = 0;

	if (schedule_type == C4_SCHED_TYPE_YEARLY) {
		res += (ideal_time_t)t.tm_mon;
		res *= (ideal_time_t)ideal_day_per_month;
	}

	if (schedule_type == C4_SCHED_TYPE_WEEKLY)
		res += (ideal_time_t)t.tm_wday;
	else if (schedule_type != C4_SCHED_TYPE_DAILY)
		res += (ideal_time_t)(t.tm_mday-1);
	// out: res in days

	res *= (ideal_time_t)ideal_hour_per_day;
	res += (ideal_time_t)t.tm_hour;
	// out: res in hours

	res *= (ideal_time_t)ideal_min_per_hour;
	res += (ideal_time_t)t.tm_min;
	// out: res in minutes

	res *= (ideal_time_t)ideal_sec_per_min;
	res += (ideal_time_t)t.tm_sec;
	// out: res in seconds

	res *= (ideal_time_t)ideal_microsec_per_sec;
	res += (ideal_time_t)usec;
	// out: res in microseconds

	return res;
}

void Castus4publicSchedule::ideal_time_to_time_tm(struct tm &tm,unsigned long &usec,ideal_time_t t,const int schedule_type) {
	usec = (unsigned long)(t % (ideal_time_t)ideal_microsec_per_sec);
	t /= (ideal_time_t)ideal_microsec_per_sec;

	tm.tm_isdst = -1;
	tm.tm_wday = 0;
	tm.tm_mday = 1;
	tm.tm_year = 0;
	tm.tm_mon = 0;

	tm.tm_sec = (int)(t % (ideal_time_t)ideal_sec_per_min);
	t /= (ideal_time_t)ideal_sec_per_min;

	tm.tm_min = (int)(t % (ideal_time_t)ideal_min_per_hour);
	t /= (ideal_time_t)ideal_min_per_hour;

	tm.tm_hour = (int)(t % (ideal_time_t)ideal_hour_per_day);
	t /= (ideal_time_t)ideal_hour_per_day;

	if (schedule_type == C4_SCHED_TYPE_WEEKLY)
		tm.tm_wday = (int)t;
	else if (schedule_type == C4_SCHED_TYPE_MONTHLY)
		tm.tm_mday = (int)t + 1;
	else if (schedule_type == C4_SCHED_TYPE_YEARLY) {
		tm.tm_mday = ((int)(t % (ideal_time_t)ideal_day_per_month)) + 1;
		tm.tm_mon = (int)(t / (ideal_time_t)ideal_day_per_month);
	}
}

bool Castus4publicSchedule::ScheduleItem::operator<(const ScheduleItem &a) const {
	return (getStartTime() < a.getStartTime());
}

bool Castus4publicSchedule::ScheduleItem::operator==(const ScheduleItem &a) const {
	return (getStartTime() == a.getStartTime());
}

bool Castus4publicSchedule::ScheduleBlock::operator<(const ScheduleBlock &a) const {
	return (getStartTime() < a.getStartTime());
}

bool Castus4publicSchedule::ScheduleBlock::operator==(const ScheduleBlock &a) const {
	return (getStartTime() == a.getStartTime());
}

Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ScheduleItem::getStartTime() const {
	const char *val = getValue("start");
	if (val == NULL) return Castus4publicSchedule::ideal_time_t_invalid;

	int sch_type = 0;
	unsigned long sub_us = 0;
	struct tm t = castus4_schedule_parse_time(val,&sub_us,&sch_type);

	return Castus4publicSchedule::time_tm_to_ideal_time(t,sub_us,sch_type);
}

bool Castus4publicSchedule::ScheduleItem::getStartTimeTm(struct tm &t,unsigned long &usec) const {
	const char *val = getValue("start");
	if (val == NULL) return Castus4publicSchedule::ideal_time_t_invalid;

	int sch_type = 0;
	t = castus4_schedule_parse_time(val,&usec,&sch_type);
	return true;
}

Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ScheduleItem::getEndTime() const {
	const char *val = getValue("end");
	if (val == NULL) return Castus4publicSchedule::ideal_time_t_invalid;

	int sch_type = 0;
	unsigned long sub_us = 0;
	struct tm t = castus4_schedule_parse_time(val,&sub_us,&sch_type);

	return Castus4publicSchedule::time_tm_to_ideal_time(t,sub_us,sch_type);
}

bool Castus4publicSchedule::ScheduleItem::getEndTimeTm(struct tm &t,unsigned long &usec) const {
	const char *val = getValue("end");
	if (val == NULL) return Castus4publicSchedule::ideal_time_t_invalid;

	int sch_type = 0;
	t = castus4_schedule_parse_time(val,&usec,&sch_type);
	return true;
}

Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ScheduleBlock::getStartTime() const {
	const char *val = getValue("start");
	if (val == NULL) return Castus4publicSchedule::ideal_time_t_invalid;

	int sch_type = 0;
	unsigned long sub_us = 0;
	struct tm t = castus4_schedule_parse_time(val,&sub_us,&sch_type);

	return Castus4publicSchedule::time_tm_to_ideal_time(t,sub_us,sch_type);
}

bool Castus4publicSchedule::ScheduleBlock::getStartTimeTm(struct tm &t,unsigned long &usec) const {
	const char *val = getValue("start");
	if (val == NULL) return Castus4publicSchedule::ideal_time_t_invalid;

	int sch_type = 0;
	t = castus4_schedule_parse_time(val,&usec,&sch_type);
	return true;
}

Castus4publicSchedule::ideal_time_t Castus4publicSchedule::ScheduleBlock::getEndTime() const {
	const char *val = getValue("end");
	if (val == NULL) return Castus4publicSchedule::ideal_time_t_invalid;

	int sch_type = 0;
	unsigned long sub_us = 0;
	struct tm t = castus4_schedule_parse_time(val,&sub_us,&sch_type);

	return Castus4publicSchedule::time_tm_to_ideal_time(t,sub_us,sch_type);
}

bool Castus4publicSchedule::ScheduleBlock::getEndTimeTm(struct tm &t,unsigned long &usec) const {
	const char *val = getValue("end");
	if (val == NULL) return Castus4publicSchedule::ideal_time_t_invalid;

	int sch_type = 0;
	t = castus4_schedule_parse_time(val,&usec,&sch_type);
	return true;
}

bool Castus4publicSchedule::ScheduleItem::setStartTimeTm(const struct tm &t,unsigned long usec) {
	std::string str = castus4_schedule_print_time(schedule_type,&t,usec);
	setValue("start",str);
	return true;
}

bool Castus4publicSchedule::ScheduleBlock::setStartTimeTm(const struct tm &t,unsigned long usec) {
	std::string str = castus4_schedule_print_time(schedule_type,&t,usec);
	setValue("start",str);
	return true;
}

bool Castus4publicSchedule::ScheduleItem::setEndTimeTm(const struct tm &t,unsigned long usec) {
	std::string str = castus4_schedule_print_time(schedule_type,&t,usec);
	setValue("end",str);
	return true;
}

bool Castus4publicSchedule::ScheduleBlock::setEndTimeTm(const struct tm &t,unsigned long usec) {
	std::string str = castus4_schedule_print_time(schedule_type,&t,usec);
	setValue("end",str);
	return true;
}

bool Castus4publicSchedule::ScheduleItem::setStartTime(const ideal_time_t t) {
	unsigned long usec;
	struct tm tm;

	Castus4publicSchedule::ideal_time_to_time_tm(tm,usec,t,schedule_type);
	setStartTimeTm(tm,usec);
	return true;
}

bool Castus4publicSchedule::ScheduleBlock::setStartTime(const ideal_time_t t) {
	unsigned long usec;
	struct tm tm;

	Castus4publicSchedule::ideal_time_to_time_tm(tm,usec,t,schedule_type);
	setStartTimeTm(tm,usec);
	return true;
}

bool Castus4publicSchedule::ScheduleItem::setEndTime(const ideal_time_t t) {
	unsigned long usec;
	struct tm tm;

	Castus4publicSchedule::ideal_time_to_time_tm(tm,usec,t,schedule_type);
	setEndTimeTm(tm,usec);
	return true;
}

bool Castus4publicSchedule::ScheduleBlock::setEndTime(const ideal_time_t t) {
	unsigned long usec;
	struct tm tm;

	Castus4publicSchedule::ideal_time_to_time_tm(tm,usec,t,schedule_type);
	setEndTimeTm(tm,usec);
	return true;
}

const char* Castus4publicSchedule::ScheduleItem::getItem() const {
	return getValue("item");
}

const char* Castus4publicSchedule::ScheduleBlock::getBlockName() const {
	return getValue("block");
}

void Castus4publicSchedule::ScheduleItem::setItem(const char *str) {
	setValue("item",str);
}

void Castus4publicSchedule::ScheduleItem::setItem(const std::string &str) {
	setValue("item",str);
}

void Castus4publicSchedule::ScheduleBlock::setBlockName(const char *str) {
	setValue("block",str);
}

void Castus4publicSchedule::ScheduleBlock::setBlockName(const std::string &str) {
	setValue("block",str);
}

/**
* Returns the schedule type as a string
*
* \returns "None", "Daily", "Weekly", "Monthly", "Yearly" or "Interval" depending on schedule_type
**/
std::string Castus4publicSchedule::type() {
	switch (schedule_type) {
		case C4_SCHED_TYPE_NONE:	return "None";
		case C4_SCHED_TYPE_DAILY:	return "Daily";
		case C4_SCHED_TYPE_WEEKLY:	return "Weekly";
		case C4_SCHED_TYPE_MONTHLY:	return "Monthly";
		case C4_SCHED_TYPE_YEARLY:	return "Yearly";
		case C4_SCHED_TYPE_INTERVAL: return "Interval";
	};
    return "None";
}


