#include <stdio.h>
#include <stdlib.h>
#include "circ_buf.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>



int circbuf_init(struct circbuf *cbuf, char *buffer, int len, const char *name)
{
	cbuf->buffer = buffer;
	cbuf->posWrite = cbuf->posRead = 0;
	cbuf->nlen = len;
	pthread_mutex_init(&cbuf->lock, NULL);
	pthread_cond_init(&cbuf->notEmpty, NULL);
	pthread_cond_init(&cbuf->notFull, NULL);

	cbuf->name = name;

	return 0;
}
int circbuf_destroy(struct circbuf *cbuf)
{
	pthread_mutex_destroy(&cbuf->lock);
	pthread_cond_destroy(&cbuf->notEmpty);
	pthread_cond_destroy(&cbuf->notFull);
	return 0;
}

int circbuf_is_full(struct circbuf *cbuf)
{
	return CIRC_SPACE(cbuf->posWrite, cbuf->posRead, cbuf->nlen) == 0 ? 1 : 0;
}

int circbuf_is_empty(struct circbuf *cbuf)
{
	return CIRC_CNT(cbuf->posWrite, cbuf->posRead, cbuf->nlen) == 0 ? 1 : 0;
}

int circbuf_available(struct circbuf *cbuf)
{
	return CIRC_SPACE(cbuf->posWrite, cbuf->posRead, cbuf->nlen);
}
int circbuf_available_to_end(struct circbuf *cbuf)
{
	return CIRC_SPACE_TO_END(cbuf->posWrite, cbuf->posRead, cbuf->nlen);
}

int circbuf_cnt(struct circbuf *cbuf)
{
	return CIRC_CNT(cbuf->posWrite, cbuf->posRead, cbuf->nlen);
}
int circbuf_cnt_to_end(struct circbuf *cbuf)
{
	return CIRC_CNT_TO_END(cbuf->posWrite, cbuf->posRead, cbuf->nlen);
}


/* write len bytes from buf to cbuf, return nbytes written to cbuf*/
int circbuf_write(struct circbuf *cbuf, char *buf, int len)
{
	int avail; // available space in circbuf;
	int nwrite; // how many bytes can be written a time;
	int ncnt;
	char *pbuf = buf;
	struct timespec timeout;

	/* block write */
	pthread_mutex_lock(&cbuf->lock);
	while(circbuf_is_full(cbuf)) {
		timeout.tv_sec=time(0);
		timeout.tv_nsec=256000;
		// wait until circbuf is not Full, with lock.
//		pthread_cond_wait(&cbuf->notFull, &cbuf->lock);
		pthread_cond_timedwait(&cbuf->notFull, &cbuf->lock, &timeout);
	}


	avail = circbuf_available(cbuf);
	if(avail > len) {
		ncnt = len;
	} else {
		ncnt = avail;
	}


	nwrite = ncnt;
	while(nwrite > 0) {
		int cnt = 0;
		int avail2end = circbuf_available_to_end(cbuf);

		if(nwrite > avail2end) {
			cnt = avail2end;
		} else {
			cnt = nwrite;
		}

		memcpy(cbuf->buffer + cbuf->posWrite, pbuf, cnt);

		// update write pos
		nwrite -= cnt;
		cbuf->posWrite += cnt;
		cbuf->posWrite %= cbuf->nlen;
		pbuf += cnt;

	}


	// send signal to possible thread. that circbuf is no longer empyty.
	pthread_cond_signal(&cbuf->notEmpty);

	pthread_mutex_unlock(&cbuf->lock);


	return ncnt;
}
int circbuf_block_write(struct circbuf *cbuf, char *buf, int len)
{
	int ncnt = 0;
	int ret;
	char *pbuf = buf;

	ncnt = len;
	while(ncnt > 0) {
		ret = circbuf_write(cbuf, pbuf, ncnt);
		ncnt -= ret;
		pbuf += ret;
	}

	return len - ncnt;
}

/* read nbytes from cbuf to buf, return nbytes read from cbuf */
int circbuf_read(struct circbuf *cbuf, char *buf, int len)
{
	int ncnt;
	int nread;
	int avail;	// available data in circbuf;
	char *pbuf = buf;
	/* block read */
	pthread_mutex_lock(&cbuf->lock);
	// wait until not empty;
	while(circbuf_is_empty(cbuf)) {
		pthread_cond_wait(&cbuf->notEmpty, &cbuf->lock);
	}


	// read data from cbuf;
	avail = circbuf_cnt(cbuf);
	if(avail > len) {
		nread = len;
	} else {
		nread = avail;
	}

	ncnt = nread;

	// read all available data.
	while(nread > 0) {
		int cnt;
		int cnt2end = circbuf_cnt_to_end(cbuf);

		if(nread > cnt2end) {
			cnt = cnt2end;
		} else {
			cnt = nread;
		}

		memcpy(pbuf, cbuf->buffer + cbuf->posRead, cnt);

		nread -= cnt;
		cbuf->posRead += cnt;
		cbuf->posRead %= cbuf->nlen;
		pbuf += cnt;
	}

	// signal producer
	pthread_cond_signal(&cbuf->notFull);

	pthread_mutex_unlock(&cbuf->lock);


	return ncnt;
}

int circbuf_block_read(struct circbuf *cbuf, char *buf, int len)
{
	int ncnt = 0;
	int ret = 0;
	char *pbuf = buf;

	ncnt = len;
	while(ncnt > 0) {
		ret = circbuf_read(cbuf, pbuf, ncnt);

		pbuf += ret;
		ncnt -= ret;
	}

	return len - ncnt;
}


