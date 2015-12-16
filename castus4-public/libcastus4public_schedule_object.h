
#include <string>
#include <vector>
#include <list>
#include <map>

class Castus4publicSchedule {
public:
	enum entry_parse_mode {
		Global=0,
		Item,
		ScheduleBlockItem,
		Defaults,
		Unknown
	};
public:
	static void common_std_map_name_value_pair_entry(std::map<std::string,std::string> &entry,std::string &name,std::string &value);
public:
	class ScheduleItem {
	public:
							ScheduleItem();
							~ScheduleItem();
		void					takeNameValuePair(std::string &name,std::string &value);
	public:
		std::map<std::string,std::string> 	entry;
	};
	class ScheduleBlock {
	public:
							ScheduleBlock();
							~ScheduleBlock();
		void					takeNameValuePair(std::string &name,std::string &value);
	public:
		std::map<std::string,std::string> 	entry;
	};
public:
							Castus4publicSchedule();
	virtual						~Castus4publicSchedule();
	void						reset();
	void						end_load();
	void						begin_load();
	void						load_take_line(const char *line);
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
	int						schedule_type;
	int						interval_length;
};

