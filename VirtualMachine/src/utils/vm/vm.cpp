#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "vm.h"
#include "program.h"


uint16_t Virtual_Machine::byte2uint16(uint8_t high_byte, uint8_t low_byte){
	return (uint16_t)high_byte<<8 | (uint16_t)low_byte;
}

uint8_t Virtual_Machine::uint16_high_byte(uint16_t uint16){
	return (uint8_t)(uint16>>8);
}

uint8_t Virtual_Machine::uint16_low_byte(uint16_t uint16){
	return (uint8_t)(uint16 & 0x00FF);
}

Virtual_Machine::Virtual_Machine(void){	
	pc=0;	
	imm=0;	
	acc=0;
	regA=0;	
	instrNum=0;
	mcs_flag=0;
	sensor_state_data=0;
	sensor_rise_flag_data=0;
	sensor_fall_flag_data=0;	
	StackInit(&VMstack, SIZE_COMBUF_B);	
	// StackInit(&LOGICstack, SIZE_COMBUF_B);	
}

/* VM start status */
int Virtual_Machine::VM_started(){
	return running;
}

/* VM start */
int Virtual_Machine::VM_start(){
	running=VM_START;
	return running;
}

/* VM stop */
int Virtual_Machine::VM_stop(){
	running=VM_STOP;
	return running;
}

/* fetch the next byte from the program */
int8_t Virtual_Machine::fetch8(){	
	return program[ pc++ ];
}

/* fetch the next word from the program */
int16_t Virtual_Machine::fetch16(){
	uint8_t high_byte=program[ pc++ ];
	uint8_t low_byte=program[ pc++ ];
	return byte2uint16(high_byte,low_byte);
}

/* decode a word */
void Virtual_Machine::decode( int16_t instr ){	
	instrNum = (instr & 0xFF );	
}

/* evaluate the last decoded instruction */
void Virtual_Machine::eval(){
	int16_t temp, temp1;	
	switch( instrNum ){
		case 0x00:
			instrNum=fetch8();		// get the required data			
			switch( instrNum ){					
				case LD:
					/* LD */
					imm=fetch16();
					acc = (sensor_state_data & (1<<imm));
					acc >>= imm;					
					printf( "LD X%d\n", imm );
					break;
				case LDP:
					/* LDP */
					imm=fetch16();					
					acc = (sensor_rise_flag_data & (1<<imm));
					acc >>= imm;					
					printf( "LDP X%d\n", imm );
					break;
				case LDF:
					/* LDF */
					imm=fetch16();
					acc = (sensor_fall_flag_data & (1<<imm));
					acc >>= imm;					
					printf( "LDF X%d\n", imm );
					break;
				case LDI:
					/* LDI */
					imm=fetch16();
					acc = ~(sensor_state_data & (1<<imm));
					acc >>= imm;					
					printf( "LDI X%d\n", imm );		
					break;	
				case AND:
					/* AND */
					imm=fetch16();
					regA = (sensor_state_data & (1<<imm));
					regA >>= imm;					
					acc &= regA;
					StackPush(&VMstack,acc);
					printf( "AND X%d\n", imm );
					break;
				case ANDP:
					/* ANDP */
					imm=fetch16();
					regA = (sensor_rise_flag_data & (1<<imm));
					regA >>= imm;
					printf( "ANDP X%d\n", imm );
					break;
				case ANDF:
					/* ANDP */
					imm=fetch16();
					regA = (sensor_fall_flag_data & (1<<imm));
					regA >>= imm;
					printf( "ANDF X%d\n", imm );
					break;
				case ANI:
					/* ANI */
					imm=fetch16();
					regA = ~(sensor_state_data & (1<<imm));
					regA >>= imm;					
					printf( "ANI X%d\n", imm );
					break;
				case ANB:
					/* ANB */					
					printf( "ANB \n" );
					break;
				case OR:
					/* OR */
					imm=fetch16();
					regA = (sensor_state_data & (1<<imm));
					regA >>= imm;					
					printf( "OR X%d\n", imm );
					break;
				case ORF:
					/* ORF */
					imm=fetch16();
					regA = (sensor_fall_flag_data & (1<<imm));
					regA >>= imm;					
					printf( "ORF X%d\n", imm );
					break;
				case ORP:
					/* ORF */
					imm=fetch16();
					regA = (sensor_rise_flag_data & (1<<imm));
					regA >>= imm;					
					printf( "ORF X%d\n", imm );
					break;
				case ORI:
					/* ORI */
					imm=fetch16();
					regA = ~(sensor_state_data & (1<<imm));
					regA >>= imm;					
					printf( "ORI X%d\n", imm );
					break;
				case ORB:
					/* ORI */
					acc=0;
					acc|=StackPop(&VMstack);
					acc|=StackPop(&VMstack);
					StackPush(&VMstack,acc);
					printf( "ORB \n");
					break;
				case SET:
					/* SET */
					imm=fetch16();
					printf( "SET Y%d\n", imm );
					break;
				case RST:
					/* RST */
					imm=fetch16();
					printf( "RST Y%d\n", imm );
					break;
				case MCS:
					/* MCS */
					mcs_flag++;					
					printf( "MCS \n" );
					break;
				case MCR:{
					/* MCR */	
					mcs_flag--;
					printf( "MCR \n" );
					break;
				}	
				case OUT:
					/* OUT */
					imm=fetch16();					
					if(StackGetTop(&VMstack))
						printf( "OUT Y%d\n", imm );
						// io_set_output(imm,1);	// set the required output				
					break;
				case END:
					/* end */
					printf( "end%d\n", 1 );
					running = 0;
					on_program_end();
					break;	
			}		
			break;
		default:
			printf("wrong code\n");
			break;
	}
}

/* reset respetive variables on program end */
void Virtual_Machine::on_program_end(){	
	sensor_state_data=0;	// get sensor data
	sensor_rise_flag_data=0;	// get sensor data
	sensor_fall_flag_data=0;	// get sensor data
	pc=0;			// start the program from 0 instruction
	acc=0;
	while (!StackIsEmpty(&VMstack)) {
		printf("SPop=%d\n", StackPop(&VMstack));
	}
}

/* Constantly running */
void Virtual_Machine::run(){
	sensor_state_data=0x08;		// get sensor data
	sensor_rise_flag_data=0;	// get sensor data
	sensor_fall_flag_data=0;	// get sensor data
	while( running ){		
		decode( fetch8() );
		eval();
	}	
}

// int main( int argc, const char * argv[] ){
	// run();
	// while(1);
	// return 0;
// }