
#include <castus4-public/schedule_object.h>
#include <castus4-public/schedule.h>
#include <castus4-public/parsetime.h>
#include <castus4-public/gentime.h>
#include <castus4-public/chomp.h>
#include <castus4-public/schedule_helpers.h>

/**
* This file contains c wrappers for schedules.   They
* are used for porting the library to other languages.
* Ruby has been done
**/

/**
* Converts the schedule time to a string
*
* \param time The schedule time
* \return A C string to be printed
*
* Schedule time needs to be divided by 1000000 to convert to seconds
* Then they are printed with %li
**/
char *time_to_string( signed long long time);

/**
* Allocates memory for a schedule
* \return The allocated space
**/
Castus4publicSchedule* schedule_alloc();

/**
* Loads a schedule from a string
*
* \param self Pointer to the schedule 
* \param data The string
* \return true if successful
**/
bool schedule_load_from_string(Castus4publicSchedule* self, const char* data);

/**
* Frees the schedules memory
*
* \param sched The schedule
**/
void schedule_free(Castus4publicSchedule* sched);

/**
* Loads a schedule from a file
*
* \param self Pointer to the schedule
* \param path The full path of the file to load
**/
bool schedule_load(Castus4publicSchedule* self, const char* path);

/**
* Returns the string representation of the schedule type
*
* \param self Pointer to the schedule
* \return The schedule string
*
* There are 5 types of schedules.   They are Yearly, Monthly, Weekly, Daily and Interval
**/
const char *schedule_type(Castus4publicSchedule* self);

/**
* \param The pointer to the schedule
* \return The length in days of the schedule
**/
int schedule_interval_in_days(Castus4publicSchedule* self);

/**
* \param self The pointer to the schedule
* \return Number of global items.
*
* This function is used with schedule_global_item_key() and schedule_global_item_value()
* to determine the maximium value of pos in each function.
**/
int schedule_globals_count(Castus4publicSchedule* self);

/**
* \param self Pointer to the schedule
* \param pos The position of the key
* \return The key at pos
*
* \sa schedule_globals_count
**/
const char *schedule_global_item_key(Castus4publicSchedule* self, unsigned int pos);

/**
* \param self Pointer to the schedule
* \param pos The position of the keys value
* \return The keys value at pos
*
* \sa schedule_globals_count
**/
const char *schedule_global_item_value(Castus4publicSchedule* self, unsigned int pos);

/**
* \param self The pointer to the schedule
* \return Number of schedule items.
*
* This function is used with schedule_item_key().
* to determine the maximium value of pos.
**/
int schedule_item_count(Castus4publicSchedule* self);

/**
* \param self Pointer to the schedule
* \param pos The position of the schedule item to return
* \return The schedule item at the position
**/
Castus4publicSchedule::ScheduleItem *schedule_item(Castus4publicSchedule* self, int pos);

/**
* \param self The pointer to the schedule
* \return Number of blocks.
*
* This function is used with schedule_block() 
* to determine the maximium value of pos.
**/
int schedule_block_count(Castus4publicSchedule* self);

/**
* \param self Pointer to the schedule
* \param pos The position of the block
*
* \return The block at pos
* \sa schedule_block_count()
**/
Castus4publicSchedule::ScheduleBlock *schedule_block(Castus4publicSchedule* self, int pos);

/**
* \param self Pointer to the block
* \return Name of the block
**/
const char *block_name( Castus4publicSchedule::ScheduleBlock *self );

/**
* \param self Pointer to the block
* \return The relative time the block starts at
**/
long long block_start_time( Castus4publicSchedule::ScheduleBlock *self );

/**
* \param self Pointer to the block
* \return The relative time the block stops at
**/
long long block_stop_time( Castus4publicSchedule::ScheduleBlock *self );

/**
* \param self Pointer to the block
* \return The number of entries the block has
* \sa block_entry_key
**/
int block_entry_count( Castus4publicSchedule::ScheduleBlock *self );

/**
* \param self Pointer to the block
* \param pos The position for the block entry
* \return The block entries key
**/
const char *block_entry_key(Castus4publicSchedule::ScheduleBlock* self, int pos);

/**
* \param self Pointer to the schedule item
* \return The relative time the schedule item starts at
**/
long long item_start_time( Castus4publicSchedule::ScheduleItem *self );

/**
* \param self Pointer to the schedule item
* \return The relative stop time of the schedule
**/
long long item_stop_time( Castus4publicSchedule::ScheduleItem *self );

/**
* \param self Pointer to the schedule item
* \return The number of entries the item has
*
* \sa item_entry_key, item_entry_value
**/
int item_entry_count( Castus4publicSchedule::ScheduleItem *self );

/**
* \param self Pointer to the schedule item
* \param pos The position of the entry
* \return The entry key at pos
**/
const char *item_entry_key(Castus4publicSchedule::ScheduleItem* self, int pos);

/**
* \param self Pointer to the schedule item
* \param pos The position of the entry
* \return The entry key's value at pos
**/
const char *item_entry_value(Castus4publicSchedule::ScheduleItem* self, int pos);




