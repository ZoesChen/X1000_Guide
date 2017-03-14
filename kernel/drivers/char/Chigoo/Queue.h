//---------------------------------------------------------------------------

#ifndef QueueH
#define QueueH

typedef unsigned char      _U8;
typedef unsigned short int _U16;



#ifdef __cplusplus
extern "C" {
#endif

 #define ENQ_OK    0   /**< Enqueue success  */
 #define ENQ_FAIL  1   /**< Enqueue failure  */
 #define DEQ_OK    0   /**< Dequeue success  */
 #define DEQ_FAIL  1   /**< Dequeue failure  */
 #define PEEK_OK   0   /**< Peek success     */
 #define PEEK_FAIL 1   /**< Peek failure     */
 
 
typedef struct
 {
   _U8*  in_p;      /**< Pointer to input location of circular queue */
   _U8*  out_p;     /**< Pointer to output location */
   _U8*  start_p;   /**< Pointer to start of the queue buffer */
   _U8*  end_p;     /**< Pointer to location after the end of the queue */
   _U16  max_size;  /**< Max size of the queue */
   _U16  obj_size;
   _U16  size;      /**< Current size of the queue */
 } queue_mgmt_t;
 
void Init_Qeue(_U8* QueueBuffer,queue_mgmt_t* queue_mgmt_p,_U8 obj_size,_U16 MaxSize); 
void ClearQeue(queue_mgmt_t* queue_mgmt_p);
 _U8 EnQeue(_U8 *elt_p, queue_mgmt_t* queue_mgmt_p);
 _U8 DeQeue(_U8 *elt_p, queue_mgmt_t* queue_mgmt_p);
 _U8 PeekQeue(_U8* elt_p, queue_mgmt_t* queue_mgmt_p);
 _U8 GetQueueSize(queue_mgmt_t* queue_mgmt_p);
 
#ifdef __cplusplus
}
#endif

#endif
