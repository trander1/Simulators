#include <stdio.h>

#include "RobotCtrl.h"
#include "DummyOperation.h"

#define NUM_TOTAL_ITERATIONS	10

int main()
{
	RobotCtrl *ctrl = new RobotCtrl;
	// Create new operations
	DummyOperation *dummy = new DummyOperation(0, "Operation1");
	DummyOperation *dummy1 = new DummyOperation(1, "Operation2");
	DummyOperation *dummy2 = new DummyOperation(2, "Operation3");
	DummyOperation *dummy3 = new DummyOperation(3, "Operation4");
	
	// create a sequence of operations
	ctrl->AddOperation(dummy);
	ctrl->AddOperation(dummy1);
	ctrl->AddOperation(dummy2);
	ctrl->AddOperation(dummy3);
	ctrl->PrintMe();	// show the list
	
	// execute the operations in the order the sequence was created
	int iterations = 0;
	while(iterations++ < NUM_TOTAL_ITERATIONS)
	{
		ctrl->ExecOperation();
		printf("iterations %d \n", iterations);
	}
	
	return 0;
}