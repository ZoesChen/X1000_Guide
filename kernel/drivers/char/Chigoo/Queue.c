#include "Queue.h"
void Init_Qeue(_U8* QueueBuffer,queue_mgmt_t* queue_mgmt_p,_U8 obj_size,_U16 MaxSize)
 {  
   queue_mgmt_p->max_size  = MaxSize/obj_size;
   queue_mgmt_p->in_p      = QueueBuffer;
   queue_mgmt_p->out_p     = QueueBuffer;
   queue_mgmt_p->start_p   = QueueBuffer;
   queue_mgmt_p->end_p     = QueueBuffer + queue_mgmt_p->max_size *obj_size;
   queue_mgmt_p->obj_size  = obj_size;
   queue_mgmt_p->size      = 0;
 }

void ClearQeue(queue_mgmt_t* queue_mgmt_p)
{
     queue_mgmt_p->in_p      =  queue_mgmt_p->start_p;
     queue_mgmt_p->out_p     =  queue_mgmt_p->start_p;
     queue_mgmt_p->size      = 0;
}
 _U8 EnQeue(_U8 *elt_p, queue_mgmt_t* queue_mgmt_p)
 {
   _U16 i;
   _U8  Result;
  
   if(queue_mgmt_p->size < queue_mgmt_p->max_size)
   {

        for (i=0;i<queue_mgmt_p->obj_size;i++)
           *(_U8 *)(queue_mgmt_p->in_p+i) = *elt_p++;
           
        queue_mgmt_p->in_p    = queue_mgmt_p->in_p +queue_mgmt_p->obj_size;
        if(queue_mgmt_p->in_p == queue_mgmt_p->end_p)
        {
               queue_mgmt_p->in_p = queue_mgmt_p->start_p;
        }
        queue_mgmt_p->size++; // update size last
        
        Result=ENQ_OK;
   }
   else
   {
        Result=ENQ_FAIL;
   }
   	
   return Result;
 }

 _U8 DeQeue(_U8 *elt_p, queue_mgmt_t* queue_mgmt_p)
 {
  _U16 i;
  _U8  Result;
  
  
  if(queue_mgmt_p->size >0)
  {
     for (i=0;i<queue_mgmt_p->obj_size;i++)
         *elt_p++= *(_U8*)(queue_mgmt_p->out_p+i);
	 
     queue_mgmt_p->out_p = queue_mgmt_p->out_p + queue_mgmt_p->obj_size;

     if(queue_mgmt_p->out_p == queue_mgmt_p->end_p)
     {
       queue_mgmt_p->out_p = queue_mgmt_p->start_p;
     }

     queue_mgmt_p->size--; // update size last

  	 Result=DEQ_OK;
  }
  else
  {
  	 Result=DEQ_FAIL;
  }

  return Result;
 }

_U8 PeekQeue(_U8* elt_p, queue_mgmt_t* queue_mgmt_p)
 {
  _U16 i;
  _U8  Result;

  if(queue_mgmt_p->size>0)
  {
     
     for (i=0;i<queue_mgmt_p->obj_size;i++)
         *elt_p++= *(queue_mgmt_p->out_p+i);
     Result=PEEK_OK;
     
  }
  else  
  {
  	
  	 Result=PEEK_FAIL;
  }
  return Result;
 }
_U8 GetQueueSize(queue_mgmt_t* queue_mgmt_p)
{
	  _U8  Result;
    
    Result=queue_mgmt_p->size;
    return Result;
}
