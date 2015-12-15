
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <string>

std::string castus4_schedule_print_time(int stm_type,struct tm *stm,unsigned long stm_us=0);

