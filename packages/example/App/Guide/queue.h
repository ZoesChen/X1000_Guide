#ifndef _GUIDE_QUEUE_H_
#define _GUIDE_QUEUE_H_

typedef struct _node{  
    int data;  
    struct _node *prev;  
    struct _node *next;  
} node;  
  
typedef struct _queue{  
    node *front;  
    node *end;  
} queue;  

int IsQueueFree(queue *q);
int Push(int , queue *);
int Pop(queue *);

#endif
