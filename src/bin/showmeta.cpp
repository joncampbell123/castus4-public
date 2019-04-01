
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

#include <castus4-public/metadata.h>

using namespace std;

int main(int argc,char **argv) {
	struct stat st;

	if (argc < 2) {
		fprintf(stderr,"showmeta <media file>\n");
		return 1;
	}

	std::string file_path = argv[1];
	std::string metadir_path = castus4public_file_to_metadata_dir(file_path);
	if (metadir_path.empty()) {
		fprintf(stderr,"Unable to obtain metadata directory\n");
		return 1;
	}

	fprintf(stderr,"File:    '%s'\n",file_path.c_str());
	fprintf(stderr,"Metadir: '%s'\n",metadir_path.c_str());

	if (stat(file_path.c_str(),&st)) {
		fprintf(stderr,"Cannot stat file %s\n",file_path.c_str());
		return 1;
	}
	if (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode)) {
		fprintf(stderr,"%s is neither file nor directory\n",file_path.c_str());
		return 1;
	}

	if (stat(metadir_path.c_str(),&st)) {
		fprintf(stderr,"Cannot stat metadir %s\n",metadir_path.c_str());
		return 1;
	}
	if (!S_ISDIR(st.st_mode)) {
		fprintf(stderr,"%s is not directory\n",metadir_path.c_str());
		return 1;
	}

	std::string meta_path = metadir_path + "/metadata";
	if (stat(meta_path.c_str(),&st)) {
		fprintf(stderr,"Cannot stat file %s\n",meta_path.c_str());
		return 1;
	}
	if (!S_ISREG(st.st_mode)) {
		fprintf(stderr,"%s is not file\n",meta_path.c_str());
		return 1;
	}

	castus4public_metadata_list mlist;
	if (!mlist.read_metadata(meta_path.c_str())) {
		fprintf(stderr,"Unable to read metadata\n");
		return 1;
	}

	for (std::map<std::string,std::string>::iterator i = mlist.list.begin();i != mlist.list.end();i++)
		printf("'%s' => '%s'\n",
			i->first.c_str(),
			i->second.c_str());

	return 0;
}

