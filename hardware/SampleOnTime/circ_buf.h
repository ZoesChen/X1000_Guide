/*
 * See Documentation/circular-buffers.txt for more information.
 */

#ifndef _LINUX_CIRC_BUF_H
#define _LINUX_CIRC_BUF_H 1

struct circbuf {
	char * buffer;
	int posWrite;
	int posRead;
	int nlen;

	pthread_mutex_t lock;
	pthread_cond_t notEmpty;
	pthread_cond_t notFull;

	const char *name;
};


/* Return count in buffer.  */
#define CIRC_CNT(head,tail,size) (((head) - (tail)) & ((size)-1))

/* Return space available, 0..size-1.  We always leave one free char
   as a completely full buffer has head == tail, which is the same as
   empty.  */
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))

/* Return count up to the end of the buffer.  Carefully avoid
   accessing head and tail more than once, so they can change
   underneath us without returning inconsistent results.  */
#define CIRC_CNT_TO_END(head,tail,size) \
	({int end = (size) - (tail); \
	  int n = ((head) + end) & ((size)-1); \
	  n < end ? n : end;})

/* Return space available up to the end of the buffer.  */
#define CIRC_SPACE_TO_END(head,tail,size) \
	({int end = (size) - 1 - (head); \
	  int n = (end + (tail)) & ((size)-1); \
	  n <= end ? n : end+1;})


int circbuf_init(struct circbuf *cbuf, char *buffer, int len, const char *name);
int circbuf_destroy(struct circbuf *cbuf);
int circbuf_is_full(struct circbuf *cbuf);
int circbuf_is_empty(struct circbuf *cbuf);
int circbuf_available(struct circbuf *cbuf);
int circbuf_available_to_end(struct circbuf *cbuf);
int circbuf_cnt(struct circbuf *cbuf);
int circbuf_cnt_to_end(struct circbuf *cbuf);
int circbuf_write(struct circbuf *cbuf, char *buf, int len);
int circbuf_block_write(struct circbuf *cbuf, char *buf, int len);
int circbuf_read(struct circbuf *cbuf, char *buf, int len);
int circbuf_block_read(struct circbuf *cbuf, char *buf, int len);


#endif /* _LINUX_CIRC_BUF_H  */



