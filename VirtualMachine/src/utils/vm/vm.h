#ifndef _VM_H_
#define _VM_H_

// #include "debug_frmwrk.h"
// #include "input_output.h"
#include "global.h"
#include "stack.h"

// #define	END		0x0000
// #define	LD		0x0001
// #define	LDI		0x0002
// #define	LDP		0x0003
// #define	LDF		0x0004
// #define	OUT		0x0005
// #define	AND		0x0007
// #define	ANDP	0x0008
// #define	ANDF	0x0009
// #define	ANI		0x000A
// #define	ANB		0x000B
// #define	OR		0x000C
// #define	ORP		0x000D
// #define	ORF		0x000E
// #define	ORI		0x000F
// #define	ORB		0x0010
// #define	SET		0x0011
// #define	RST		0x0012
// #define	MCS		0x0013
// #define	MCR		0x0014

enum OPCODES {	
	END=0x00,
	LD,
	LDI,
	LDP,
	LDF,
	OUT,
	AND,
	ANDP,
	ANDF,
	ANI,
	ANB,
	OR,
	ORP,
	ORF,
	ORI,
	ORB,
	SET,
	RST,
	MCS,
	MCR,
};

#define VM_START 	1
#define VM_STOP 	0

/*Return codes for state machines...*/
enum VM_RET{
	VM_RESET = 0,
	VM_STARTED = 1,
	VM_DONE = 2,
	VM_ERROR = -1,
	VM_ABORTED = 3,
};

class Virtual_Machine{
	public:	
		stackT VMstack;			/* A stack to hold characters. */
		stackT LOGICstack;			/* A stack to hold characters. */
		int running;			/* the VM runs until this flag becomes 0 */
		uint32_t sensor_state_data;			/* input state data */
		uint32_t sensor_rise_flag_data;		/* input rise flag data */
		uint32_t sensor_fall_flag_data;		/* input fall flag data */
		
		Virtual_Machine(void);
		
		virtual int VM_started();
		virtual int VM_start();
		virtual int VM_stop();
		
		virtual void on_program_end();
		virtual void run();

	private:
		int pc;					/* program counter */
		int16_t acc;			/* accumulator */
		int16_t regA;			/* local register for operations*/
		uint16_t instrNum;		/* instruction fields */
		int16_t imm;			/* locally used for storing data*/ 
		int mcs_flag;
		
		int8_t fetch8();
		int16_t fetch16();
		void decode(int16_t);
		void eval();
		
		uint16_t byte2uint16(uint8_t , uint8_t );
		uint8_t uint16_high_byte(uint16_t );
		uint8_t uint16_low_byte(uint16_t );
};

#endif