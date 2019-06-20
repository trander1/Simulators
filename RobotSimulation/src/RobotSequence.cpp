#include <stdio.h>
#include <string.h>

#include "RobotSequence.h"

RobotSequence::RobotSequence()
{
	numTotalOperations=0;
}

void RobotSequence::AddOperation(RobotOperations *o)
{
	if (rootOper != NULL)
	{		
		rootOper->AddNextOperation(o);
	}
	else
	{		
		rootOper = o;
	}
	numTotalOperations ++;
}

int RobotSequence::DeleteOperations()
{
	RobotOperations *next;
	RobotOperations *ro = rootOper;
	int deleted_items = 0;
	if(rootOper != NULL)
	{
		next = rootOper->GetNextOperation();
		do{
			++deleted_items;
			delete ro;		
			ro = next;
			--numTotalOperations;
			if(next!=NULL)
				next = ro->GetNextOperation();			
		}
		while (next != NULL);
		rootOper = NULL;
		curOper = NULL;
	}
	return deleted_items;
}


void RobotSequence::PrintMe(void)
{
	printf("Sequence: \n");
	printf("\tNumOper: %d\n",numTotalOperations);
	RobotOperations *iter;
	int idx = 0;
	for(iter=rootOper,idx=0; iter != NULL; iter=iter->GetNextOperation(),idx++)
	{
		printf("\tOper%2d: ",idx);
		iter->PrintMe();
		printf("\n");
	}
}

