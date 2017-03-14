#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int IsQueueFree(queue *q)
{
	return (q->end == NULL) ? 1 : 0;
}

int Push(int data, queue *q)  
{  
    // Construct a new node  
    node *nnode = (node *)malloc(sizeof(node));  
    nnode->data = data;  
    nnode->prev = NULL;  
    nnode->next = NULL;  
  
    // If the queue is not empty  
    if (q->end != NULL)  
    {  
        // Set the new node to be the last one;  
        q->end->next = nnode;  
        nnode->prev = q->end;  
    }  
    else  
    {  
        q->front = nnode;  
    }  
  
    q->end = nnode;  
  
    return 0;  
}  

int Pop(queue *q)  
{  
    // If the queue is null, return  
    if (q->front == NULL)  
        return -1;  
  
    int temp;  
  
    if (q->front->next != NULL)  
    {  
        // Make the second one to be first one  
        q->front->next->prev = NULL;  
        temp = q->front->data;  
  
        free(q->front);  
        q->front = q->front->next;  
    }  
    else  
    {  
        temp = q->front->data;  
  
        free(q->front);  
        q->front = NULL;  
        q->end = NULL;  
    }  

    return temp;  
}  
