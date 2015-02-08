// #include "LPC214x.h"		/* LPC21XX Peripheral Registers	*/
// #include "types.h"
#include <stdint.h>
// #include "irq.h"
// #include "print_dbg.h"
// #include "timer.h"
#include "delay_timer.h"
// #include "Cstartup.h"


uint32_t delay_value[MAX_NUM_DELAY];
uint32_t delay_const[MAX_NUM_DELAY];
static char delay_enable[MAX_NUM_DELAY];

uint32_t accum_update_a;
uint32_t accum_update_b;

uint32_t delay_timer_32;
//-----------PWM DESCRIPTION-----------------------------
//PWMPR:prescalar  
//default timer working at 15Mhz
//after prescaling: 15Mhz/1000 =15 khz.
//		Timeperiod: 66.66usec
//		 max count: (2^32) * 66.66usec
//--------------------------------------------------------

void dt_init(void){	
	delay_timer_32 = 0;
}

/*
sec mul is the number of divisions of a second allowed...
i.e., for secmul = 15000 and VPB_DIV = 4 and CCLK = 60MHz, PWMPR = 999 


*/
void dt_init(uint32_t sec_mul){
	delay_timer_32 = 0;
	for (int i=0;i<MAX_NUM_DELAY;i++){
		delay_value[i] = 0;
		delay_enable[i] = 0;
	}
}

void dt_update(void){
	delay_timer_32 ++;
}

static uint32_t value_check(){
	do{
		accum_update_a = delay_timer_32;
		accum_update_b = delay_timer_32;
	}while(accum_update_a != accum_update_b);
	return accum_update_a;
}

static int is_timeup(uint32_t start_time, uint32_t time_delay){
	
	uint32_t l_delay_timer_32 = value_check();	
	int32_t time_diff = l_delay_timer_32 - start_time;

	if (time_diff < 0){
	// add 0xffffffff for -ve values for the 32 bit timer...
		time_diff += 0xFFFFFFFF;
	}
	
	if (time_diff > (int32_t)time_delay)
		return 1;
	else
		return 0;
}

static uint32_t time_left(uint32_t start_time, uint32_t time_delay){
	
	uint32_t l_delay_timer_32 = value_check();
	int32_t time_diff = l_delay_timer_32 - start_time;
	
	if (time_diff < 0){
		// add 0xffffffff for -ve values for the 32 bit timer...
		time_diff += 0xFFFFFFFF;
	}	
	if (time_diff > (int32_t)time_delay){
		return 0;
	} else{
		return (time_delay - time_diff);
	}
}

void my_delay(long d)
{
	long t_val;
	uint32_t l_delay_timer_32 = value_check();
	
	t_val = l_delay_timer_32;
		while (1){
			if (!is_timeup(t_val, d));
			//	update_32bit_counter_value();
			//	delay_timer_32 = PWMTC;
			else
				break;
	}
}

int dt_is_timeup(int i)
{
	return is_timeup(delay_value[i], delay_const[i]);

}

// this function is not working... not sure why...
int dt_timeup_reset(int i)
{
	int retVal;
	retVal=is_timeup(delay_value[i], delay_const[i]);
	dt_reset(i);
	return retVal;
}

int dt_reset(int i)
{
	uint32_t l_delay_timer_32 = value_check();
	
	delay_value[i] = l_delay_timer_32;
	return 1;
}

uint32_t dt_time_left(int i)
{
	return time_left(delay_value[i],delay_const[i]);
}

void dt_set_delay(int i, uint32_t d)
{
	delay_const[i] = d;
}

uint32_t dt_get_delay(int i)
{
	return delay_const[i];
}

DT_HMS dt_get_hms(uint32_t time_left, uint32_t sec_mul)
{
	DT_HMS l_hms;
	uint32_t total_secs_left = time_left/ sec_mul;
	l_hms.hrs =  total_secs_left/3600;
	
	total_secs_left = (total_secs_left % 3600);
	l_hms.min = total_secs_left / 60;
	
	l_hms.sec = total_secs_left%60;
	
	return l_hms;
	
}

uint32_t dt_convert_hms(DT_HMS hms, uint32_t sec_mul)
{
	uint32_t cnt_time;
	
	cnt_time = (hms.hrs *3600 + hms.min*60 + hms.sec) * sec_mul;
	
	return cnt_time;
}

int dt_start(int i){
	
	uint32_t l_delay_timer_32 = value_check();
	delay_value[i] = l_delay_timer_32;
	delay_enable[i] = 1;
	
	return 1;
}

int dt_stop(int i){
	delay_enable[i] = 0;
	
	return 1;
}	
	
int dt_is_timeup_with_enable(int i){
	if (delay_enable[i] == 1){
		return is_timeup(delay_value[i], delay_const[i]);
	}
	else{
		return 0;
	}
}
