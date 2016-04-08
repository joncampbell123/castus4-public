#include <castus4-public/schedule_object.h>
#include <castus4-public/schedule.h>
#include <castus4-public/parsetime.h>
#include <castus4-public/gentime.h>
#include <castus4-public/chomp.h>
#include <castus4-public/schedule_helpers.h>

const int to_second_conversion = 1000000;

extern "C" {

    char *time_to_string( signed long long time) { 
        time_t start = time/1000000;
        size_t string_length = snprintf(NULL, 0, "%li", start)+1;
        char* buf = (char*)malloc(string_length);
        if ( !buf ) {
            // Handle allocation failure
        nullptr;
        }

        snprintf(buf, string_length, "%li", start);

        return buf;
    }

    Castus4publicSchedule* schedule_alloc() {
        return new Castus4publicSchedule{};
    }

    bool schedule_load_from_string(Castus4publicSchedule* self, const char* data) {
        return Castus4publicScheduleHelpers::load_from_string(*self, data);
    }   

    void schedule_free(Castus4publicSchedule* sched) {
        delete sched;
    }

    bool schedule_load(Castus4publicSchedule* self, const char* path) {
        return Castus4publicScheduleHelpers::load(*self, path);
    }   

    const char *schedule_type(Castus4publicSchedule* self) { 
        return strdup( self->type().c_str() );
    }

    int schedule_interval_in_days(Castus4publicSchedule* self) { 
        return self->interval_length; 
    }

    int schedule_globals_count(Castus4publicSchedule* self) { 
        return self->global_values.size();
    }

    // TODO use iterator math
    const char *schedule_global_item_key(Castus4publicSchedule* self, unsigned int pos) { 
        for (auto ret = self->global_values.begin(); true; --pos, ++ret)
            if (pos<=0)
                return strdup( ret->first.c_str() );
        return 0;
    }

    // TODO use iterator math
    const char *schedule_global_item_value(Castus4publicSchedule* self, unsigned int pos) { 
        for (auto ret = self->global_values.begin(); true; --pos, ++ret)
            if (pos<=0)
                return strdup( ret->second.c_str() );
        return 0;
    }

    int schedule_item_count(Castus4publicSchedule* self) { 
        return self->schedule_items.size();
    }

    // TODO use iterator math
    Castus4publicSchedule::ScheduleItem *schedule_item(Castus4publicSchedule* self, int pos) { 
        for (auto ret = self->schedule_items.begin(); true; --pos, ++ret)
            if (pos<=0)
                return &(*ret);
           
        return nullptr;
    }

    int schedule_block_count(Castus4publicSchedule* self) { 
        return self->schedule_blocks.size();
    }


    // TODO use iterator math
    Castus4publicSchedule::ScheduleBlock *schedule_block(Castus4publicSchedule* self, int pos) { 
        for (auto ret = self->schedule_blocks.begin(); true; --pos, ++ret)
            if (pos<=0)
                return &(*ret);
           
        return nullptr;
    }

   // Block functions

    const char *block_name( Castus4publicSchedule::ScheduleBlock *self ) {
        return strdup( self->getValue("block") );
    }

    long long block_start_time( Castus4publicSchedule::ScheduleBlock *self ) {
        return self->getStartTime()/to_second_conversion;
    }


    long long block_stop_time( Castus4publicSchedule::ScheduleBlock *self ) {
        return self->getEndTime()/to_second_conversion;
    }

    int block_entry_count( Castus4publicSchedule::ScheduleBlock *self ) {
        self->entry.size();
    }

    // TODO use iterator math
    const char *block_entry_key(Castus4publicSchedule::ScheduleBlock* self, int pos) { 
        for (auto ret = self->entry.begin(); true; --pos, ++ret)
            if (pos<=0 ) 
                return strdup(ret->first.c_str());
           
        return "";
    }

    // TODO use iterator math
    const char *block_entry_value(Castus4publicSchedule::ScheduleBlock* self, int pos) { 
        for (auto ret = self->entry.begin(); true; --pos, ++ret)
            if (pos<=0 ) 
                return strdup(ret->second.c_str());
           
        return "";
    }

    // Item functions

    long long item_start_time( Castus4publicSchedule::ScheduleItem *self ) {
        return self->getStartTime()/to_second_conversion;
    }


    long long item_stop_time( Castus4publicSchedule::ScheduleItem *self ) {
        return self->getEndTime()/to_second_conversion;
    }

    int item_entry_count( Castus4publicSchedule::ScheduleItem *self ) {
        self->entry.size();
    }

    // TODO use iterator math
    const char *item_entry_key(Castus4publicSchedule::ScheduleItem* self, int pos) { 
        for (auto ret = self->entry.begin(); true; --pos, ++ret)
            if (pos<=0 ) 
                return strdup(ret->first.c_str());
           
        return "";
    }

    // TODO use iterator math
    const char *item_entry_value(Castus4publicSchedule::ScheduleItem* self, int pos) { 
        for (auto ret = self->entry.begin(); true; --pos, ++ret)
            if (pos<=0 ) 
                return strdup(ret->second.c_str());
           
        return "";
    }





}
