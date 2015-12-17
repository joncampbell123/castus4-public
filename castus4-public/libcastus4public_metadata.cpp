
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h> /* flock */
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <castus4-public/libcastus4public_metadata.h>
#include <castus4-public/libcastus4public_chomp.h>

const char *castus4public_metadir_prefix = ".castusmeta.";

castus4public_metadata_list::castus4public_metadata_list() {
}

castus4public_metadata_list::~castus4public_metadata_list() {
}

bool castus4public_metadata_list::read_metadata(const char *path) {
	char line[4096];
	char *equ;
	FILE *fp;

	list.clear();
	if (path == NULL) return false;

	fp = fopen(path,"r");
	if (!fp) return false;

	/* Linux/POSIX offers file locking. Castus uses file locking for the metadata file. We do too.
	 * You can skip this step but then you risk reading an incomplete metadata file if the user is updating
	 * metadata from the web UI at exactly the same time */
	if (::flock(fileno(fp),LOCK_EX)) {
		fclose(fp);
		return false;
	}

	while (!feof(fp) && !ferror(fp)) {
		if (fgets(line,sizeof(line)-1,fp) == NULL) break;
		castus4public_chomp(line);

		equ = strchr(line,'=');
		if (equ == NULL) continue;
		*equ++ = 0; /* ASCIIz snip */

		/* line = name
		 * equ = value
		 *
		 * If multiple entries occur, they are assumed to be multi-line tags. This is a recent addition to the Castus 4 standard.
		 * Prior versions assume name=value pair per line and that repeats overwrite earlier values. Note that some parts of
		 * Castus 4 still assume single line pairs and some fields including duration and file type should not be multi-line.
		 * If you do use the multi-line mode, make sure the multiple lines are stored next to each other (no other tags inbetween). */
		if (list.find(line) == list.end())
			list[line] = equ;
		else
			list[line] += std::string("\n") + equ;
	}

	::flock(fileno(fp),LOCK_UN);
	fclose(fp);
	return true;
}

void castus4public_metadata_list::clear() {
	list.clear();
}

const char *castus4public_metadata_list::getValue(const char *name) {
	std::map<std::string,std::string>::iterator i = list.find(name);
	if (i == list.end()) return NULL;
	return i->second.c_str();
}

void castus4public_metadata_list::setValue(const char *name,const char *value) {
	list[name] = value;
}

void castus4public_metadata_list::setValue(const char *name,std::string &value) {
	list[name] = value;
}

bool castus4public_is_metadata_dir(const char *path) {
	const char *s = strrchr(path,'/');

	if (s != NULL) s++;
	else s = path;

	/* if it starts with .castusmeta. then it's a metadata directory path */
	return (strncmp(s,castus4public_metadir_prefix,strlen(castus4public_metadir_prefix)) == 0);
}

bool castus4public_is_metadata_dir(const std::string &path) {
	return castus4public_is_metadata_dir(path.c_str());
}

std::string castus4public_file_to_metadata_dir(const char *path) {
	std::string res;
	const char *s;

	if (castus4public_is_metadata_dir(path)) return "";

	s = strrchr(path,'/');

	if (s != NULL) s++;
	else s = path;

	if (s != path) res = std::string(path,(size_t)(s-path));
	res += castus4public_metadir_prefix;
	res += s;
	return res;
}

std::string castus4public_file_to_metadata_dir(const std::string &path) {
	return castus4public_file_to_metadata_dir(path.c_str());
}

