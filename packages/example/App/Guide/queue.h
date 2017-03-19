#ifndef _GUIDE_QUEUE_H_
#define _GUIDE_QUEUE_H_
#include "key.h"

typedef struct _node{  
    CMDMSG *data;  
    struct _node *prev;  
    struct _node *next;  
} node;  
  
typedef struct _queue{  
    node *front;  
    node *end;  
} queue;

int IsQueueFree(queue *q);
int Push(CMDMSG * , queue *);
CMDMSG *Pop(queue *q) ;


#endif
