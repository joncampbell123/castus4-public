
#ifndef Castus4publicScheduleObject_h
#define Castus4publicScheduleObject_h

#include <cinttypes>
#include <cstdio>

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>

class Castus4publicSchedule;

class Castus4publicSchedule {
public:
	// "ideal time t" is duration from start of schedule in microseconds
	typedef int64_t			ideal_time_t;

	static const ideal_time_t ideal_microsecond;
	static const ideal_time_t ideal_second;
	static const ideal_time_t ideal_minute;
	static const ideal_time_t ideal_hour;
	static const ideal_time_t ideal_day;
	static const ideal_time_t ideal_week;
	static const ideal_time_t ideal_month;
	static const ideal_time_t ideal_year;

	static const ideal_time_t ideal_time_t_invalid;
	static const ideal_time_t ideal_microsec_per_sec;
	static const ideal_time_t ideal_sec_per_min;
	static const ideal_time_t ideal_min_per_hour;
	static const ideal_time_t ideal_hour_per_day;
	static const ideal_time_t ideal_day_per_week;
	static const ideal_time_t ideal_day_per_month;
	static const ideal_time_t ideal_month_per_year;

    // The schedule type as a string
    std::string type();
public:
	enum entry_parse_mode {
		Global=0,
		Item,
		ScheduleBlockItem,
		Defaults,
		Triggers,
		Unknown
	};
	typedef bool (*writeout_cb_t)(Castus4publicSchedule *_this,const char *line,void *opaque);
public:
	static void common_std_map_name_value_pair_entry(std::map<std::string,std::string> &entry,const std::string &name,const std::string &value);
	static ideal_time_t				time_tm_to_ideal_time(const struct tm &t,const unsigned long usec,const int schedule_type);
	static void					ideal_time_to_time_tm(struct tm &tm,unsigned long &usec,ideal_time_t t,const int schedule_type);
public:
	class ScheduleItem {
	public:
							ScheduleItem(const int schedule_type);
							~ScheduleItem();
		void					takeNameValuePair(const std::string &name,const std::string &value);
		const char*				getValue(const char *name) const;
		void					setValue(const char *name,const char *value);
		void					setValue(const char *name,const std::string &value);
		void					deleteValue(const char *name);

		bool					getStartTimeTm(struct tm &t,unsigned long &usec) const;
		bool					getEndTimeTm(struct tm &t,unsigned long &usec) const;
		ideal_time_t				getStartTime() const;
		ideal_time_t				getEndTime() const;

		bool					setStartTimeTm(const struct tm &t,const unsigned long usec);
		bool					setEndTimeTm(const struct tm &t,const unsigned long usec);
		bool					setStartTime(const ideal_time_t t);
		bool					setEndTime(const ideal_time_t t);

		const char*				getItem() const;
		void					setItem(const char *str);
		void					setItem(const std::string &str);

		bool					operator<(const ScheduleItem &a) const;
		bool					operator==(const ScheduleItem &a) const;
	public:
		std::map<std::string,std::string> 	entry;
		int					schedule_type;
	};
	class ScheduleBlock {
	public:
							ScheduleBlock(const int schedule_type);
							~ScheduleBlock();
		void					takeNameValuePair(const std::string &name,const std::string &value);
		const char*				getValue(const char *name) const;
		void					setValue(const char *name,const char *value);
		void					setValue(const char *name,const std::string &value);
		void					deleteValue(const char *name);

		bool					getStartTimeTm(struct tm &t,unsigned long &usec) const;
		bool					getEndTimeTm(struct tm &t,unsigned long &usec) const;
		ideal_time_t				getStartTime() const;
		ideal_time_t				getEndTime() const;

		bool					setStartTimeTm(const struct tm &t,const unsigned long usec);
		bool					setEndTimeTm(const struct tm &t,const unsigned long usec);
		bool					setStartTime(const ideal_time_t t);
		bool					setEndTime(const ideal_time_t t);

		const char*				getBlockName() const;
		void					setBlockName(const char *str);
		void					setBlockName(const std::string &str);

		bool					operator<(const ScheduleBlock &a) const;
		bool					operator==(const ScheduleBlock &a) const;
	public:
		std::map<std::string,std::string> 	entry;
		int					schedule_type;
	};
public:
							Castus4publicSchedule();
	virtual						~Castus4publicSchedule();
	void						reset();
	void						end_load();
	void						begin_load();
	void						load_take_line(const char *line);

	void						sort_schedule_items();
	void						sort_schedule_blocks();

	void						sort_schedule_items_rev(); // only for testing
	void						sort_schedule_blocks_rev(); // only for testing

	bool						write_out(FILE *fp);
	static bool					write_out_stdio_cb(Castus4publicSchedule *_this,const char *line,void *opaque);

	bool						write_out(std::ostream &cout);
	static bool					write_out_iostream_cb(Castus4publicSchedule *_this,const char *line,void *opaque);

	bool						write_out(writeout_cb_t f,void *opaque);

	bool						write_out_name_value_pair(const std::string &name,const std::string &value,writeout_cb_t f,void *opaque,bool tab,bool spcequ);
public:
	bool						head;
	std::string					entry;			// if within { ... } block
	bool						in_entry;
	enum entry_parse_mode				entry_mode;
// parsed output
	std::list<ScheduleItem>				schedule_items;
	std::list<ScheduleBlock>			schedule_blocks;
	std::map<std::string,std::string>		defaults_values;
	std::string					defaults_type;
	std::map<std::string,std::string>		global_values;
	std::multimap<std::string, std::string>		schedule_triggers;
	int						schedule_type;
	int						interval_length;
};

#endif // Castus4publicSchedule_h

