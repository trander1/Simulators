#ifndef ROBOT_CTRL
#define ROBOT_CTRL
#include <stdint.h>

#include "RobotSequence.h"
#include "RobotOperations.h"

#define MAX_PROG_STORAGE_BYTES 1024

class RobotCtrl 
{
	private:
		RobotSequence sequence;	// final sequence to be used
		RO_EXEC_TYPES executionStatus;
		
		int deleteOperations();
	
	public:
		RobotCtrl(void);
		
		void AddOperation(RobotOperations *o);
		RO_EXEC_TYPES ExecOperation();
		
		void PrintMe(void);
		
		RobotOperations* GetRootOperation();
		~RobotCtrl(void);		
};

#endif
