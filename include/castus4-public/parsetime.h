
#ifndef Castus4publicParcetime_h
#define Castus4publicParcetime_h

#include <time.h>	// for struct tm

struct tm castus4_schedule_parse_time(const char *v,unsigned long *sub_us,int *type);

#endif