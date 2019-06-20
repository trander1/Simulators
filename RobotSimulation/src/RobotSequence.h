#ifndef _ROBOT_SEQUENCE_H_
#define _ROBOT_SEQUENCE_H_
#include <stdint.h>

#include "RobotOperations.h"

class RobotSequence
{
	private:
		int numTotalOperations;
	
	public:
		RobotSequence();
		
		RobotOperations *rootOper;
		RobotOperations *curOper;
		
		void AddOperation(RobotOperations *o);
		int DeleteOperations();
		
		void PrintMe();
};

#endif
