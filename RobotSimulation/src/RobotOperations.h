#ifndef ROBOT_OPERATIONS
#define ROBOT_OPERATIONS
#include <stdint.h>

typedef enum 
{
	RO_EXEC_ERROR=-1,
	RO_EXEC_RUNNING=0,
	RO_EXEC_DONE=1,
	RO_EXEC_CYCLE_CONTINOUS=2,
	RO_EXEC_CYCLE_STOP=3,
}RO_EXEC_TYPES;

typedef enum 
{
	RO_TYPE_DEFAULT=0,
	RO_TYPE_DUMMY,
	// additonal operations that could be developed
	RO_TYPE_SINGLE_OPERATION,	
	RO_TYPE_DOUBLE_OPERATION,
	RO_TYPE_ARM_DOWN_OPERATION,
	RO_TYPE_ARM_UP_OPERATION,
	RO_TYPE_ARM_FWD_OPERATION,
	RO_TYPE_ARM_BACK_OPERATION,
	RO_TYPE_SWING_IN_OPERATION,	
	RO_TYPE_SWING_OUT_OPERATION,	
	RO_TYPE_GRIP_CLOSE_OPERATION,
	RO_TYPE_GRIP_RELEASE_OPERATION,
	RO_TYPE_MOLD_OPEN_OPERATION,
	RO_TYPE_MOLD_CLOSE_OPERATION,
	RO_TYPE_TOTAL_NUM,
}RO_TYPES;

class RobotOperations 
{
	private:
		RobotOperations *next;		// this is the pointer to the next operation.
	
	public:
		RobotOperations(void);		// constructor to initialize private variables
		
		int type;					// type of operation
		
		/* This function is called for executing this operation.
		Return Value: 
			0: still working, 
			1: Done
			-1: error
		*/	
		virtual RO_EXEC_TYPES ExecOperation();	
		
		virtual void AddNextOperation(RobotOperations *n);
		virtual RobotOperations * GetNextOperation(void);
		
		virtual void PrintMe(void);
		
		virtual ~RobotOperations(void){};
};

#endif
