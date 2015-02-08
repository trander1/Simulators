#include "queue.h"

Queue::Queue(){}

Queue::Queue(int size) {
    front = back = -1;
    Queue::size = size - 1;
    array = new int[size];
    count = 0;
    
    for(int i = 0; i <= size; i++) {
    	array[i] = 0;
    }
}


void Queue::enqueue(int item=0) {
    if (front == 0 && back == size || front == back + 1) {
        printf( "Queue is full\n");
    } 
	else if (front == -1 && back == -1) {
	    front = 0;
	    back = 0;
	    array[front] = item;
	    count++;
    }
    else if (back == size) {
    	back = 0;
    	array[back] = item;
    	count++;
    }
    else {
    	back++;
    	array[back] = item;
    	count++;
    }
}

int Queue::dequeue() {
	int retval=0;
    if (front == -1 && back == -1) {
        printf( "Queue is empty\n");
    }
    else {
        if (front == back) {
	    retval=array[front];
		array[front] = 0;
	    front = -1;
	    back = -1;
	    count--;
		}
		else if (front == size) {
			retval=array[front];
			array[front] = 0;
			front = 0;
			count--;
		}
		else {
			retval=array[front];
			array[front] = 0;
			front++;
			count--;
		}
    }
	return retval;
}

void Queue::show() {
    if (count == 0) {
        printf( "Queue is empty\n");
    } else {
        for(int i = 0; i < size + 1; i++)
            printf("%d ,", array[i] );
        printf("\n");
    }
}

#if 0
int main() {
    int keyCode = 0, size = 0;
    int tmpInput;
    
    printf( "Enter size of queue: ");
    scanf("%d",&size);
    
    Queue queue(size);

    while (true) {
        cout << "Enter 1 for add, 2 for deletion, 3 for display, 4 for exit.\n";
        cin >> keyCode;

        switch (keyCode) {
        case 1:
            cout << "Enter number: ";
            cin >> tmpInput;
            queue.enqueue(tmpInput);
            break;
        case 2:
            queue.dequeue();
            break;
        case 3:
            queue.show();
            break;
        case 4:
            exit(0);
            break;
        default:
            break;
        }
    }
}
#endif
