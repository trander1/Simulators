#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "RobotOperations.h"

RobotOperations::RobotOperations(void)
{
	next=NULL;
	type=RO_TYPE_DEFAULT;
}

void RobotOperations::AddNextOperation(RobotOperations *n)
{	
	if (next != NULL)
	{
		next->AddNextOperation(n);
	}
	else
	{
		next=n;
		n->next = NULL;		
	}
}

RobotOperations * RobotOperations::GetNextOperation(void)
{	
	return next;
}

/*
Used for Automatic Mode Of operation
*/
RO_EXEC_TYPES RobotOperations::ExecOperation()
{
	return RO_EXEC_DONE;
}


void RobotOperations::PrintMe(void)
{
	printf("Base Type: %d", type);
}
