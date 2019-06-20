#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "DummyOperation.h"

DummyOperation::DummyOperation():RobotOperations()
{
	type = RO_TYPE_DUMMY;
	number = 5;
};

DummyOperation::DummyOperation(int n):RobotOperations()
{
	type = RO_TYPE_DUMMY;
	number = n;
};

DummyOperation::DummyOperation(int n, const char* s):RobotOperations()
{
	type = RO_TYPE_DUMMY;
	number = n;
	strcpy(operationString, s);
};

RO_EXEC_TYPES DummyOperation::ExecOperation()
{
	PrintMe();
	return RO_EXEC_DONE;
}

void DummyOperation::PrintMe(void)
{
	printf("	Type: %d Number: %d Operation: %s\n", type, number, operationString);
}
