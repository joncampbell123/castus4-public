
#include <castus4-public/schedule_helpers.h>
#include <castus4-public/chomp.h>
#include <string.h>


namespace Castus4publicScheduleHelpers {

    bool load(class Castus4publicSchedule &schedule, std::string file) {
	    FILE *fp;
    	char line[1024];

	    fp = fopen(file.c_str(),"r");
        if (fp==NULL)
            return false;

	    schedule.begin_load();
	    memset(line,0,sizeof(line));
	    while (!feof(fp) && !ferror(fp)) {
		    if (fgets(line,sizeof(line)-1,fp) == NULL) break;
		    castus4public_chomp(line);
		    schedule.load_take_line(line);
	    }
        // TODO return false if !ferror(fp)

	    schedule.end_load();
	    fclose(fp);
        return true;
    }
}
