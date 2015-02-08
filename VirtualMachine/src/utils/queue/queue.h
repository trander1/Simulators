#ifndef QUEUE_h
#define QUEUE_h

#include<stdio.h>
#include<stdlib.h>


class Queue{
    private:
        int front, back, size, count;
        int *array;
    public:
		Queue();
        Queue(int);
        void enqueue(int);
        void show();
        // void dequeue();
        int dequeue();
};

extern Queue Q1;
#endif