/*
 *          File: stacktest.c
 *        Author: Robert I. Pitts <rip@cs.bu.edu>
 * Last Modified: March 7, 2000
 *         Topic: Stack - Array Implementation
 * ----------------------------------------------------------------
 *
 * OVERVIEW:
 * =========
 * This program tests the "stack of characters" module.
 */

#include <stdio.h>
#include <string.h>  /* for strlen() */
#include "stack.h"

int main(void)
{
  stackT stack;    /* A stack to hold characters. */
  char str[101];   /* String entered by user. */
  char *traverse;  /* Pointer used to traverse the string. */

  /*
   * Get a string from the user--don't enter more
   * than 100 characters!
   */
  printf("Enter a string: ");

  gets(str);  /* Read line, discard newline. */

  /*
   * Initialize the stack.  Make it at least
   * big enough to hold the string we read in.
   */
  StackInit(&stack, strlen(str));

  /*
   * Traverse the string and put each
   * character on the stack.
   */

  for (traverse = str; *traverse != '\0'; traverse++) {
    StackPush(&stack, *traverse);
  }

  /*
   * Pop each of the characters off of
   * the stack and print them out.
   */

  printf("\nPopped characters are: ");

  while (!StackIsEmpty(&stack)) {
    printf("%c", StackPop(&stack));
  }

  printf("\n");

  StackDestroy(&stack);

  return 0;
}
