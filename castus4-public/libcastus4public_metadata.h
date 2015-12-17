
#include <string>
#include <list>
#include <map>

class castus4public_metadata_list {
public:
	castus4public_metadata_list();
	~castus4public_metadata_list();
public:
	bool read_metadata(const char *path);
	void clear();
	const char *getValue(const char *name);
	void setValue(const char *name,const char *value);
	void setValue(const char *name,std::string &value);
public:
	std::map<std::string,std::string>	list;
};

bool castus4public_is_metadata_dir(const char *path);
bool castus4public_is_metadata_dir(const std::string &path);
std::string castus4public_file_to_metadata_dir(const char *path);
std::string castus4public_file_to_metadata_dir(const std::string &path);

