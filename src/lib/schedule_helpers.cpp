
#include <castus4-public/schedule_helpers.h>
#include <castus4-public/chomp.h>
#include <string.h>
#include <boost/algorithm/string.hpp>

namespace Castus4publicScheduleHelpers {

    /**
    * \param schedule The schedule data structure to be filled
    * \param data The string containing the schedule
    * \return true
    *
    * Loads a schedule from a string
    **/
    bool load_from_string(class  Castus4publicSchedule &schedule, std::string data) {
        schedule.begin_load();

        // Split the string into an array of lines
        std::vector<std::string> lines;
        boost::split(lines, data, boost::is_any_of("\n\r"));

        for (std::vector<std::string>::iterator i = lines.begin(); i!=lines.end(); i++) {
            std::string line = *i;
            // Remove trailing return if it exists
            boost::algorithm::trim_right(line);
            // Only accept lines with text in case the line split
            // gives us blank lines
            if (line.size() != 0)
                schedule.load_take_line( line.c_str() );
        }
        schedule.end_load();
        return true;
    }

    /**
    * \param schedule The schedule object to load
    * \param file The full path of the file to load
    *
    * Loads a schedule file
    **/
    bool load(class Castus4publicSchedule &schedule, std::string file) {
        FILE *fp;
        char line[1024];

        fp = fopen(file.c_str(),"r");
        if (fp==NULL) {
            return false;
        }

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
