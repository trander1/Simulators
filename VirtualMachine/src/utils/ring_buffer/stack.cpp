/*
 *          File: stack.c
 *        Author: Robert I. Pitts <rip@cs.bu.edu>
 * Last Modified: March 7, 2000
 *         Topic: Stack - Array Implementation
 * ----------------------------------------------------------------
 *
 * This is an array implementation of a character stack.
 */

#include "stack.h"       

/************************ Function Definitions **********************/

void StackInit(stackT *stackP, int maxSize) {
	stackElementT *newContents;
#ifdef USE_MALLOC
	/* Allocate a new array to hold the contents. */
	newContents = (stackElementT *)malloc(sizeof(stackElementT) * maxSize);
	if (newContents == NULL) {
		/* Exit, returning error code. */
	}
	stackP->contents = newContents;
#else
	stackP->contents = stackP->aContents;
#endif
	stackP->maxSize = maxSize;
	stackP->top = -1;  /* I.e., empty */
}

void StackDestroy(stackT *stackP) {
#ifdef USE_MALLOC
	/* Get rid of array. */
	free(stackP->contents);
	stackP->contents = NULL;
#endif 
	stackP->maxSize = 0;
	stackP->top = -1;  /* I.e., empty */
}

void StackPush(stackT *stackP, stackElementT element) {
	if (StackIsFull(stackP)) {
		fprintf(stderr, "Can't push element on stack: stack is full.\n");
		exit(1);  /* Exit, returning error code. */
	}
	/* Put information in array; update top. */
	stackP->contents[++stackP->top] = element;
}

stackElementT StackPop(stackT *stackP) {
	if (StackIsEmpty(stackP)) {
		fprintf(stderr, "Can't pop element from stack: stack is empty.\n");
		exit(1);  /* Exit, returning error code. */
	}
	return stackP->contents[stackP->top--];
}

stackElementT StackGetTop(stackT *stackP) {
	if (StackIsEmpty(stackP)) {
		fprintf(stderr, "Can't get top element from stack: stack is empty.\n");
		exit(1);  /* Exit, returning error code. */
	}
	return stackP->contents[stackP->top];
}

int StackIsEmpty(stackT *stackP) {
	return stackP->top < 0;
}

int StackIsFull(stackT *stackP) {
	return stackP->top >= stackP->maxSize - 1;
}
