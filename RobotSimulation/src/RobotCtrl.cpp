#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "RobotCtrl.h"

RobotCtrl::RobotCtrl(void){	
	sequence.rootOper = NULL;
	sequence.curOper = NULL;
		
	executionStatus = RO_EXEC_DONE;
}

RobotCtrl::~RobotCtrl(void)
{
	deleteOperations();	// delete the operations
}

void RobotCtrl::AddOperation(RobotOperations *o)
{
	sequence.AddOperation(o);	// add new operations to the sequence
}

RO_EXEC_TYPES RobotCtrl::ExecOperation()
{
	executionStatus = RO_EXEC_DONE;

	if (sequence.curOper != NULL)	// start only when we know there's something to execute
	{
		executionStatus = sequence.curOper->ExecOperation();	// execute the current operation				
		if (executionStatus == RO_EXEC_DONE)
		{
			sequence.curOper = sequence.curOper->GetNextOperation();	// get the next operation in the sequence
			if (sequence.curOper == NULL)
			{
				sequence.curOper = GetRootOperation();		// start the process again
			}
		}
		else if(executionStatus > RO_EXEC_DONE)
		{
			return executionStatus;
		}
		else if(executionStatus == RO_EXEC_RUNNING)	// keep running the same operation when required
		{
			return executionStatus;
		}
		else
		{	
			// this could be the error state..
		}
	}
	else
	{
		sequence.curOper = GetRootOperation();	// first time when we set the root operation as the current operation
	}

	return executionStatus;
}

void RobotCtrl::PrintMe()
{
	sequence.PrintMe();
}

RobotOperations* RobotCtrl::GetRootOperation(void)
{
	return sequence.rootOper;
}

int RobotCtrl::deleteOperations()
{
	int ret;
	ret = sequence.DeleteOperations();
	return ret;
}


