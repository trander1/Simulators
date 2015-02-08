//!
/*!
 *Main file 
 * All the various function are called inside the while loop of the main fucntion
 * The various functions are polled using the Timer at regualr preset intervals set at the beginning of the program.
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>

#include "LPC17xx.h"
#include "ring.h"
#include "syscalls.h"
#include "global.h"
#include "config.h"
#include "delay_timer.h"

#include "Menu.h"
#include "LCDmenu.h"
#include "ShaggyLCD.h"
#include "menu_top.h"
#include "menu_float_entry.h"
#include "menu_int_entry.h"
#include "menu_multiple_float_display.h"
#include "menu_teach_program.h"
#include "menu_run_program.h"
#include "menu_inputs_display.h"
#include "RobotCtrlDisplay.h"

#include "parameters.h"
#include "timers.h"

#include "menu_combo_box.h"
#include "RobotCtrl.h"
#include "RobotOperations.h"
#include "DummyOperation.h"
#include "SingleActingOperation.h"
#include "DoubleActingOperation.h"

extern "C" {	
	#include "lpc17xx_libcfg.h"
	#include "lpc17xx_ssp.h"
	#include "lpc17xx_spi.h"
	#include "lpc17xx_clkpwr.h"
	#include "lpc17xx_i2c.h"
	#include "lpc17xx_gpio.h"
	#include "lpc17xx_uart.h"
	#include "lpc17xx_pinsel.h"
	#include "debug_frmwrk.h"
	#include "timer.h"	
	#include "input_output.h"
	#include "spi_master.h"	
	#include "ssp_master.h"	
	#include "keypad.h"
	#include "key.h"
	#include "i2c.h"
	#include "i2c_eeprom.h"
}

/*!
 *Main file!
 *
*/	

// SM_SPI_TRANSFER	sm_spi_transfer;

char str[35];
uint32_t temp_store = 0;
uint32_t delay_timer_32 = 0;
spi_data_state spi_out_data;

char myBuffer1[512] = "258 10  4  1  0  1  1  1  5600  2  0  1  1  2  0 32  5100  2  0  4  1  6  8 32  5990  2  0  1  0  3  0 32  5231  5  1  0  0  0  1  5300  3  1  2  1  5  1  5000  4  1  0  1  1  1  5600  2  8  4  0  7  0 40  5670  5  1  0  0  0  1  5300  3  1  2  0  4  1  5890";
char I2C_recvd[512] = {0};
char *recvd = I2C_recvd;
float mult =1.0;
uint8_t fw_major_version;
uint8_t fw_minor_version;
RobotCtrl *g_RC = NULL;
int g_start_statemachine_cycle = OFF;
int g_state_machine_error = 0;
uint32_t g_cycle_count = 0;
int motion_started = 0;
int g_edit_sm_number = 0;
float temp;
int generate_error(int return_value);

volatile unsigned char * SENSORS_STATE_VALUES[MAX_NUM_SENSORS]={&ARM_UP_DOWN_SENSOR_STATE,
																&SWING_INWARD_SENSOR_STATE,
																&SWING_OUTWARD_SENSOR_STATE,
																&GRIP_SENSOR_STATE,
																&IMM_AUTO_MANUAL_STATE,
																&SAFETY_DEVICE_STATE,
																&MOULD_OPEN_STATE,
																&EMERGENCY_STOP_IMM_STATE,
																&VACUUM_SUCTION_STATE,
																};

float * delay_timers[11] = {&parameters[MOVE_UP].delay_timer_val,
							&parameters[MOVE_DOWN].delay_timer_val,
							&parameters[MOVE_FORWARD].delay_timer_val,
							&parameters[MOVE_BACKWARD].delay_timer_val,
							&parameters[MOVE_SWING_IN].delay_timer_val,
							&parameters[MOVE_SWING_OUT].delay_timer_val,
							&parameters[MOVE_GRIP].delay_timer_val,
							&parameters[MOVE_RELEASE].delay_timer_val,
							&parameters[MOVE_VACUUM_GRIP].delay_timer_val,
							&parameters[MOVE_VACUUM_RELEASE].delay_timer_val
							};						

char g_parameter_names[9][10] = {"Vert UP",
								"Vert DN ",
								"Forward ",
								"Backwrd ",
								"Swng IN ",
								"Swg OUT ",
								"Grip    ",
								"Release ",								
								"Select  "
							 };	

/**#########################################################################################*/
key_code_count kcc;
/**#########################################################################################*/
TIMERS delays_for_operations;
Parameters parameters[MAX_NUM_PARAMETERS];

/**#########################################################################################*/
#if 1
MenuTopDisplay menu_top("1");
Menu menu_intro1("Main Menu");

Menu menu_automatic_operation("Automatic");
Menu menu_manual_operation("Manual");
Menu menu_mode_settings("Mode");

Menu menu_continuous_cycle("Cont. Cycle");
Menu menu_single_cycle("Single Cycle");

MenuComboBox menu_combo_box_1("Manual Opeartion","Operation","",3,1,11,0,g_parameter_names,9);
MenuInputsDisplay menu_inputs_display("Inputs","Inputs",SENSORS_STATE_VALUES);

MenuMultipleFloatDisplay menu_multiple_float_display("Floats","Floats","",3,10,5);
MenuFloatEntry menu_edit_time_entry("EDIT","EDIT","EDIT","",3,&temp,999.999,0.0,&mult);
MenuTeachProgram menu_teach_program("Teach", "Teach","Teach",g_parameter_names,8,5);
MenuRunProgram menu_run_program("Cont. Cycle","AUTO","",3,&g_edit_sm_number,1,20,0);
RobotCtrlDisplay menu_robot_ctrl_display("R_DISPLAY","DISPLAY");
#endif
/**#########################################################################################*/
ShaggyLCD *ptr_lcd;			//global pointer of shaggy_lcd
ShaggyLCD lcd_test;	
LCDMenu Root(menu_top, lcd_test);

int main(void) {
	uint32_t i, j, count;
	int toggle = 0,return_value = 0;;	
	uint32_t timer_count;
	uint32_t tmp = 0, tmp_status = 0;
	uint32_t delay_reset_count = 0;	
	
	/* Initialize debug via UART0
	 * – 115200bps
	 * – 8 data bit
	 * – No parity
	 * – 1 stop bit
	 * – No flow control
	 */
	
	LPC_SC->PCONP |= ( 1 << 15 ); // power up GPIO
	LPC_PINCON->PINSEL4 &= ~(0xFFFF); 				// Reset P2[0..7] = GPIO
	LPC_GPIO2->FIODIR   |=     0xFF;				// P2[0..7] = Outputs 
	LPC_GPIO2->FIOCLR 	 =     0xFF;				// Turn-OFF all LED
	debug_frmwrk_init();
	printf("\n SystemCoreClock = %d Hz\n", SystemCoreClock);	
	io_init();
	init_key();	
/**#########################################################################################*/		
	dt_set_delay(DELAY_HEARTBEAT, 2000);
	dt_set_delay(DELAY_KEY_TASK_SCAN, 50);		//keyscantask delay
	dt_set_delay(DELAY_KEY_TASK_GETKEY, 100);		//getkey delay	
	dt_set_delay(DELAY_IO_UPDATE, 100);
	dt_set_delay(DELAY_BLINK_DIGIT, 2000);
	dt_set_delay(DELAY_ANIMATE_DISPLAY, 10);
	dt_set_delay(DELAY_DISPLAY_UPDATE, 50);	
/**#########################################################################################*/			
	dt_init(SEC_MUL);	
/**#########################################################################################*/	
#if 1
	menu_top.setParent(menu_top);
	menu_top.addChild(menu_intro1);
	
	menu_intro1.addChild(menu_automatic_operation);
	menu_intro1.addChild(menu_manual_operation);
	menu_intro1.addChild(menu_mode_settings);		
	
	menu_automatic_operation.addChild(menu_run_program);
	menu_automatic_operation.addChild(menu_single_cycle);
	
	menu_manual_operation.addChild(menu_combo_box_1);	
	menu_manual_operation.addChild(menu_inputs_display);	
	menu_mode_settings.addChild(menu_teach_program);
	// menu_continuous_cycle.addChild(menu_run_program);
	
	menu_multiple_float_display.SetFloatEditMenu(&menu_edit_time_entry);	
	menu_teach_program.setRobotCtrlDisplay(&menu_robot_ctrl_display);
	menu_run_program.setRobotCtrlDisplay(&menu_robot_ctrl_display);
	menu_run_program.setMenuMultipleFloatDisplay(&menu_multiple_float_display);	
#endif	
/**#########################################################################################*/	
	for(delay_reset_count=0;delay_reset_count<DELAY_TOTAL_NUM;delay_reset_count++){
		dt_reset(delay_reset_count);
	}
	lcd_test.init();
	ptr_lcd = &lcd_test;
	
	Root.display();	
	LPC_GPIO0->FIODIR |= 1 << 26;
	LPC_GPIO0->FIOSET |= 1 << 26;
	
	SingleActingOperation sao1;	// used for add operation
	SingleActingOperation sao2;	// used for add operation
	SingleActingOperation sao3;	// used for add operation
	SingleActingOperation sao4;	// used for add operation
	SingleActingOperation sao5;	// used for add operation
	SingleActingOperation sao6;	// used for add operation
	RobotCtrl RC_store;
	RobotCtrl RC_load;
	
	// char temp[512];
	char *stored_string = I2C_recvd;	
	// i2ctest_wr();
	// i2cEEPROM_write32(0,myBuffer2);
	// write_Sequence_to_EEPROM(myBuffer2,sizeof(myBuffer2),0);	
	// write_Sequence_to_EEPROM(myBuffer1,sizeof(myBuffer1),0);	
	// for (i = 0; i < 3; i++) printf("\nMasterBuffer[%d] = %X\n", i, I2CMasterBuffer[i]);	
	// for (i = 3; i < Master_Buffer_BUFSIZE; i++) printf("MasterBuffer[%d] = %d\n", i, (I2CMasterBuffer[i]));

	// printf("\n");
	
	// i2ctest_rd();
	// read_Sequence_from_EEPROM(I2C_recvd,sizeof(I2C_recvd),0);
	// for(i=0;i<512;i++){
		// printf("\n RECVD[%d]: %d",i,(I2C_recvd[i])-48);
	// }
	// for (i = 0; i < Slave_Buffer_BUFSIZE; i++) printf("SlaveBuffer[%d] = %d\n", i, I2CSlaveBuffer[i]);

#if 1
	parameters[MOVE_UP].prev_state_input = 1;
	parameters[MOVE_UP].output_port = 0;
	parameters[MOVE_UP].direction = 0;
	parameters[MOVE_UP].output_num = MOVE_UP;
	parameters[MOVE_UP].post_state_output = 1;
	parameters[MOVE_UP].delay_timer_val = 05.3;
	parameters[MOVE_UP].parameter_name = g_parameter_names[MOVE_UP];	

	parameters[MOVE_DOWN].prev_state_input = 1;
	parameters[MOVE_DOWN].output_port = 0;
	parameters[MOVE_DOWN].direction = 1;
	parameters[MOVE_DOWN].output_num = MOVE_DOWN;
	parameters[MOVE_DOWN].post_state_output = 1;
	parameters[MOVE_DOWN].delay_timer_val = 05.6;
	parameters[MOVE_DOWN].parameter_name = g_parameter_names[MOVE_DOWN];

	parameters[MOVE_FORWARD].prev_state_input &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_FORWARD].output_port = 1;
	parameters[MOVE_FORWARD].direction = 1;
	parameters[MOVE_FORWARD].output_num = MOVE_FORWARD;
	parameters[MOVE_FORWARD].post_state_output &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_FORWARD].input_state_manual |= (1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_FORWARD].delay_timer_val = 05.10;
	parameters[MOVE_FORWARD].parameter_name = g_parameter_names[MOVE_FORWARD];

	parameters[MOVE_BACKWARD].prev_state_input &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_BACKWARD].output_port = 1;
	parameters[MOVE_BACKWARD].direction = 0;
	parameters[MOVE_BACKWARD].output_num = MOVE_BACKWARD;
	parameters[MOVE_BACKWARD].post_state_output &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_BACKWARD].input_state_manual |= (1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_BACKWARD].delay_timer_val = 5.231;
	parameters[MOVE_BACKWARD].parameter_name = g_parameter_names[MOVE_BACKWARD];

	parameters[MOVE_SWING_IN].prev_state_input = 1;
	parameters[MOVE_SWING_IN].output_port = 2;
	parameters[MOVE_SWING_IN].direction = 0;
	parameters[MOVE_SWING_IN].output_num = MOVE_SWING_IN;
	parameters[MOVE_SWING_IN].post_state_output = 1;
	parameters[MOVE_SWING_IN].delay_timer_val = 5.890;
	parameters[MOVE_SWING_IN].parameter_name = g_parameter_names[MOVE_SWING_IN];

	parameters[MOVE_SWING_OUT].prev_state_input = 1;
	parameters[MOVE_SWING_OUT].output_port = 2;
	parameters[MOVE_SWING_OUT].direction = 1;
	parameters[MOVE_SWING_OUT].output_num = MOVE_SWING_OUT;
	parameters[MOVE_SWING_OUT].post_state_output = 1;
	parameters[MOVE_SWING_OUT].delay_timer_val = 5.0;
	parameters[MOVE_SWING_OUT].parameter_name = g_parameter_names[MOVE_SWING_OUT];

	parameters[MOVE_GRIP].prev_state_input &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	// parameters[MOVE_GRIP].prev_state_input |= (1 << SENSOR_GRIP);
	// parameters[MOVE_GRIP].prev_state_input &= ~(1 << SENSOR_GRIP);
	parameters[MOVE_GRIP].output_port = 4;
	parameters[MOVE_GRIP].direction = 1;
	parameters[MOVE_GRIP].output_num = MOVE_GRIP;
	parameters[MOVE_GRIP].post_state_output &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	// parameters[MOVE_GRIP].post_state_output |= (1 << SENSOR_GRIP);
	// parameters[MOVE_GRIP].post_state_output &= ~(1 << SENSOR_GRIP);
	parameters[MOVE_GRIP].input_state_manual |= (1 << SENSOR_IMM_AUTO_MANUAL);
	// parameters[MOVE_GRIP].input_state_manual &= ~(1 << SENSOR_GRIP);
	parameters[MOVE_GRIP].delay_timer_val = 5.990;
	parameters[MOVE_GRIP].parameter_name = g_parameter_names[MOVE_GRIP];

	parameters[MOVE_RELEASE].prev_state_input &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	// parameters[MOVE_RELEASE].prev_state_input |= (1 << SENSOR_GRIP);
	// parameters[MOVE_RELEASE].prev_state_input &= ~(1 << SENSOR_GRIP);
	parameters[MOVE_RELEASE].output_port = 4;
	parameters[MOVE_RELEASE].direction = 0;
	parameters[MOVE_RELEASE].output_num = MOVE_RELEASE;
	parameters[MOVE_RELEASE].post_state_output &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	// parameters[MOVE_RELEASE].post_state_output |= (1 << SENSOR_GRIP);
	// parameters[MOVE_RELEASE].post_state_output &= ~(1 << SENSOR_GRIP);
	parameters[MOVE_RELEASE].input_state_manual |= (1 << SENSOR_IMM_AUTO_MANUAL);
	// parameters[MOVE_RELEASE].input_state_manual &= ~(1 << SENSOR_GRIP);
	// parameters[MOVE_RELEASE].input_state_manual |= (1 << SENSOR_GRIP);
	parameters[MOVE_RELEASE].delay_timer_val = 1.67;
	parameters[MOVE_RELEASE].parameter_name = g_parameter_names[MOVE_RELEASE];

	parameters[MOVE_VACUUM_GRIP].prev_state_input &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_VACUUM_GRIP].output_port = 3;
	parameters[MOVE_VACUUM_GRIP].direction = 1;
	parameters[MOVE_VACUUM_GRIP].output_num = MOVE_VACUUM_GRIP;
	parameters[MOVE_VACUUM_GRIP].post_state_output = 1;
	parameters[MOVE_VACUUM_GRIP].input_state_manual |= (1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_VACUUM_GRIP].input_state_manual &= ~(1 << SENSOR_VACUUM);
	parameters[MOVE_VACUUM_GRIP].delay_timer_val = 5.90;
	parameters[MOVE_VACUUM_GRIP].parameter_name = g_parameter_names[MOVE_VACUUM_GRIP];

	parameters[MOVE_VACUUM_RELEASE].prev_state_input &= ~(1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_VACUUM_RELEASE].output_port = 3;
	parameters[MOVE_VACUUM_RELEASE].direction = 0;
	parameters[MOVE_VACUUM_RELEASE].output_num = MOVE_VACUUM_RELEASE;
	parameters[MOVE_VACUUM_RELEASE].post_state_output = 1;
	parameters[MOVE_VACUUM_RELEASE].input_state_manual |= (1 << SENSOR_IMM_AUTO_MANUAL);
	parameters[MOVE_VACUUM_RELEASE].input_state_manual |= (1 << SENSOR_VACUUM);
	parameters[MOVE_VACUUM_RELEASE].delay_timer_val = 5.89;
	parameters[MOVE_VACUUM_RELEASE].parameter_name = g_parameter_names[MOVE_VACUUM_RELEASE];

#endif
	
	// GPIO_SetDir(0,20,0);
	// GPIO_SetDir(0,19,0);
	// LPC_GPIO0->FIODIR |= 0 << 0;
	// LPC_GPIO0->FIODIR |= 0 << 1;
	// LPC_GPIO0->FIOCLR |= 1 << 0;	
	// LPC_GPIO0->FIOCLR |= 1 << 1;	
	// int width = 5;
	printf("\nwassup");
	// sprintf(str,"%dabc",5);
	sprintf(str," %% ");
	printf(str);
	// printf("%%%dd",5);
	printf("\ndone");
		
	sao1.params = &parameters[MOVE_FORWARD];
	sao2.params = &parameters[MOVE_BACKWARD];
	sao3.params = &parameters[MOVE_GRIP];
	sao4.params = &parameters[MOVE_RELEASE];
	sao5.params = &parameters[MOVE_VACUUM_GRIP];
	sao6.params = &parameters[MOVE_VACUUM_RELEASE];
	
	RC_store.addOperation(&sao1);
	RC_store.addOperation(&sao2);
	RC_store.addOperation(&sao3);
	RC_store.addOperation(&sao4);
	RC_store.addOperation(&sao5);
	RC_store.addOperation(&sao6);
	
	RC_store.seqSaveMe(stored_string);
	printf("\n Store: %s",stored_string);
	
	RC_load.seqLoadMe(stored_string);
	
	I2CDeinit();
	while(1){
		// timer_count++;
		// timer_count = GPIO_ReadValue(0);
		// if( 0x01 & (timer_count >> 20)){
			// printf("\n hi 20");
		// }else{
			// printf("\n low 20");
		// }
		// timer_count = GPIO_ReadValue(0);
		// if( 0x01 & (timer_count >> 19)){
			// printf("\n hi 19");
		// }else{
			// printf("\n low 19");
		// }
		if(dt_is_timeup(DELAY_HEARTBEAT)){
			dt_reset(DELAY_HEARTBEAT);	
			if(toggle){
				// printf("printing.\n");
				// printf("T: %d",timer_count);
				toggle = 0;				
				// LPC_GPIO0->FIOSET |= 1 << 0;							
				// LPC_GPIO0->FIOSET |= 1 << 1;							
			}else{				
				// printf(",");
				// printf("T: %d",timer_count);				
				toggle = 1;
				// LPC_GPIO0->FIOCLR |= 1 << 0;				
				// LPC_GPIO0->FIOCLR |= 1 << 1;				
				// timer_count = 0;
			}
			// printf(".");
		}
#if 1		
		if (g_start_statemachine_cycle == ON && g_RC != NULL){	
			return_value = g_RC->exec();
			if(return_value > 1){
				generate_error(return_value);
			}
		}	
#endif		
		if(dt_is_timeup(DELAY_IO_UPDATE)){
			dt_reset(DELAY_IO_UPDATE);
			io_update(); //update inputs
		}
#if 1		
		
		if(dt_is_timeup(DELAY_DISPLAY_UPDATE)){
			dt_reset(DELAY_DISPLAY_UPDATE);
			Root.refreshNow();	
		}
#endif
		
#if 1		
		if(dt_is_timeup(DELAY_KEY_TASK_SCAN)){	//checking for go_home in menu system
			dt_reset(DELAY_KEY_TASK_SCAN);
			keyscantask();
			//Make Row High
			// try_keypad();
		}
		
		if(dt_is_timeup(DELAY_KEY_TASK_GETKEY)){	//checking for go_home in menu system
			dt_reset(DELAY_KEY_TASK_GETKEY);	
			if (keygetkey(0x0f, &kcc)){
				// printf("\nK:%d,c%d,ud%d", kcc.code,kcc.count,kcc.updown);//prints the value of the key pressed				
				if (kcc.updown == KEY_PRESSED){					
					switch(kcc.code){
						default:
							Root.keyPress(kcc);
						break;
					}
				}
				else{
				}
			}	
		}		
#endif
#if 0	
		//SPI Testing
		if(dt_is_timeup(DELAY_IO_UPDATE)){
			dt_reset(DELAY_IO_UPDATE);
			if(tmp){
				CS_Force(0);				
				SPI_SendData(LPC_SPI, 0x55);
				do
				{
					tmp_status = SPI_GetStatus(LPC_SPI);
				}while(!(SPI_CheckStatus(tmp_status,SPI_STAT_SPIF)));
				// while (!((LPC_SPI->SPSR) & SPI_SPSR_SPIF));
				for (count = 1000000; count; count--);
				SPI_SendData(LPC_SPI, 0x55);
				do
				{
					tmp_status = SPI_GetStatus(LPC_SPI);
				}while(!(SPI_CheckStatus(tmp_status,SPI_STAT_SPIF)));
				// while (!((LPC_SPI->SPSR) & SPI_SPSR_SPIF));
				for (count = 1000000; count; count--);
				CS_Force(1);
				tmp = 0;
			}else{
				CS_Force(0);				
				SPI_SendData(LPC_SPI, 0xAA);
				do
				{
					tmp_status = SPI_GetStatus(LPC_SPI);
				}while(!(SPI_CheckStatus(tmp_status,SPI_STAT_SPIF)));
				// while (!((LPC_SPI->SPSR) & SPI_SPSR_SPIF));
				for (count = 1000000; count; count--);
				SPI_SendData(LPC_SPI, 0xAA);
				do
				{
					tmp_status = SPI_GetStatus(LPC_SPI);
				}while(!(SPI_CheckStatus(tmp_status,SPI_STAT_SPIF)));
				// while (!((LPC_SPI->SPSR) & SPI_SPSR_SPIF));
				for (count = 1000000; count; count--);
				CS_Force(1);
				tmp = 1;
			}						
		}
#endif
	}
	return 0 ;
}

int get_current_input_status(uint32_t *curr_input_stat){
/*
	Location of sensor states as follows:
		0 => ARM_UP_DOWN_SENSOR_STATE (ARM_UP_STATE = 0, ARM_DOWN_STATE = 1, INITIAL_STATE_SENSOR = 0)
		1 => SWING_INWARD_SENSOR_STATE (SWING_IN_STATE = 0, INITIAL_STATE_SENSOR = 0)
		2 => SWING_OUTWARD_SENSOR_STATE (SWING_OUT_STATE = 0, INITIAL_STATE_SENSOR = 1)
		3 => GRIP_SENSOR_STATE (GRIP_SENSOR_STATE = 0,RELEASE_SENSOR_STATE = 1, INITIAL_STATE_SENSOR = 1)
		4 => VACUUM_SUCTION_STATE (INITIAL_STATE_SENSOR = 1)
		5 => IMM_AUTO_MANUAL_STATE
		6 => SAFETY_DEVICE_STATE
		7 => MOULD_OPEN_STATE
		8 => EMERGENCY_STOP_IMM_STATE
*/
	*curr_input_stat = 0;
	int sensor_num = 0;	
	for(sensor_num = 0;sensor_num < MAX_NUM_SENSORS;sensor_num++){
		if(*(SENSORS_STATE_VALUES[sensor_num])){
			*curr_input_stat |= 1 << sensor_num;			
		}else{			
			*curr_input_stat &= ~(1 << sensor_num);
		}
	}	
	return 1;
}

int generate_error(int return_value){
	switch(return_value){
		case ARM_ISSUE:
			g_state_machine_error = ARM_ISSUE;
			// print_string("\nARM_ISSUE");
			break;
		case SWING_IN_ISSUE:
			g_state_machine_error = SWING_IN_ISSUE;
			// print_string("\nSWING_IN_ISSUE");
			break;
		case SWING_OUT_ISSUE:
			g_state_machine_error = SWING_OUT_ISSUE;
			// print_string("\nSWING_OUT_ISSUE");
			break;				
		case GRIPPER_ISSUE:
			g_state_machine_error = GRIPPER_ISSUE;
			// print_string("\nGRIPPER_ISSUE");
			break;
		case VACUUM_ISSUE:
			g_state_machine_error = VACUUM_ISSUE;
			// print_string("\nVACUUM_ISSUE");
			break;
		case IMM_AUTO_MANUAL_ISSUE:	
			g_state_machine_error = IMM_AUTO_MANUAL_ISSUE;
			// print_string("\nIMM_AUTO_MANUAL_ISSUE");		
			break;
		case SAFETY_DEVICE_ISSUE:
			g_state_machine_error = SAFETY_DEVICE_ISSUE;
			// print_string("\nSAFETY_DEVICE_ISSUE");
			break;
		case MOLD_OPEN_ISSUE:
			g_state_machine_error = MOLD_OPEN_ISSUE;
			// print_string("\nMOLD_OPEN_ISSUE");
			break;
		case E_STOP_ISSUE:
			g_state_machine_error = E_STOP_ISSUE;
			// print_string("\nE_STOP_ISSUE");
			break;			
		default:			
			break;
	}
	return 1;
}

int write_Sequence_to_EEPROM(char *s, int size, int seq_num){
	// seqnum is the num of the sequence to write to...
	//Start breaking the data into 32byte chunks
	// unsigned int i = 0;
	int id = 0;
	int chunkSize = 32;
	int arr_idx = 0;
	int retval = 0;
	char temp_str[33] = {0};
	char read_str[33] = {0};
	
	unsigned char chunk_size = 32;
	unsigned int seq_eeprom_start_offset = 0;
	unsigned int seq_size = 512;
	unsigned int seq_addr = seq_num*seq_size + seq_eeprom_start_offset;
	// printf("SeqAddr: %d\n",seq_addr);
	
	unsigned int write_addr = seq_addr;
	
	memset(recvd,'0',sizeof(I2C_recvd));
	sprintf(I2C_recvd,"%s",s);
	
	for (id = 0;id < 16;id++){
		arr_idx = id *chunkSize;
		memcpy(temp_str,&I2C_recvd[arr_idx],chunk_size);
		temp_str[chunk_size] = '\0';
		// printf("Wr%d:%s\n",id,temp_str);
		write_addr = seq_addr + arr_idx;
		retval = i2cEEPROM_write32(write_addr,temp_str);
		if (retval !=0){			
			// printf("Wrong: %d\n",id);
			break;
		}
		//read it back...
		my_delay(50);
		i2cEEPROM_read32(write_addr,read_str,chunk_size);
		read_str[chunk_size] = '\0';
		// printf("Rd%d:%s\n",id,read_str);
	}	
	I2CDeinit();
	// my_delay(10);
	return retval;
}

int read_Sequence_from_EEPROM(char *s, int size, int seq_num){
	// seq_num is the num of the seq to load...
	// start getting the data for the sequence in chunks of 32 bytes..
	// unsigned int i = 0;
	int id = 0;
	int chunkSize = 32;
	int arr_idx = 0;
	int retval = 0;
	char recvd_str[32];
	char *data = recvd_str;
	
	unsigned char chunk_size = 32;
	unsigned int seq_eeprom_start_offset = 0;
	unsigned int seq_size = 512;
	unsigned int seq_addr = seq_num*seq_size + seq_eeprom_start_offset;
	
	unsigned int read_addr = seq_addr;
	
	memset(s,'0',size);
	s[0] = '\0';
	for (id = 0;id < 16;id++){
		arr_idx = id *chunkSize;		
		read_addr = seq_addr + arr_idx;
		memset(data,'0',sizeof(recvd_str));
		retval = i2cEEPROM_read32(read_addr,recvd_str,chunk_size); 
		if (retval != 0){			
			// printf("Wrong: %d\n",id);
			break;
		}		
		//read it back...		
		recvd_str[chunk_size] = '\0';
		// printf("Rd%d:%s\n",id,recvd_str);
		strcat(s,recvd_str);
	}
	// for(i=0;i<seq_size;i++){
		// printf("RECVD[%d]: %d\n",i,(s[i])-48);
	// }
	// printf("final string = %s",s);
	I2CDeinit();
	// my_delay(10);
	return retval;
	
}
