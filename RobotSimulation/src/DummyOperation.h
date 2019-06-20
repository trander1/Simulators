#ifndef DUMMY_OPERATION
#define DUMMY_OPERATION

#include "RobotOperations.h"

class DummyOperation: public RobotOperations
{
	private:
		int number;
		char operationString[10];
	
	public:
		DummyOperation();
		DummyOperation(int n);
		DummyOperation(int n, const char* s);

		virtual RO_EXEC_TYPES ExecOperation();
		
		virtual void PrintMe(void);
		
		~DummyOperation(void){};
};

#endif
