#ifndef DELAY_TIMER_h
#define DELAY_TIMER_h
#include <stdint.h>

#include "config.h"

#define MAX_NUM_DELAY DELAY_TOTAL_NUM


void dt_init(void);
void dt_init(uint32_t sec_mul);
void dt_update();
void my_delay(long d);
int dt_is_timeup(int i);
int dt_timeup_reset(int i);
int dt_reset(int i);
uint32_t dt_time_left(int i);
void dt_set_delay(int i, uint32_t d);
uint32_t dt_get_delay(int i);

int dt_is_timeup_with_enable(int i);
int dt_start(int i);
int dt_stop(int i);

extern uint32_t delay_timer_32;
// #define dt_start(i)	dt_reset(i)


typedef struct { 
	int hrs; 
	int min; 
	int sec;
} DT_HMS;

/* sec_mul is the seconds multiplier..*/
DT_HMS dt_get_hms(uint32_t time_left, uint32_t sec_mul);
uint32_t dt_convert_hms(DT_HMS hms, uint32_t sec_mul);

#endif
