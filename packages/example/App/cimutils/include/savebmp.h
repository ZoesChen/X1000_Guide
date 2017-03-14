#ifndef __SAVE_BMP_H_
#define __SAVE_BMP_H_

#include "headers.h"

typedef unsigned short	WORD;	//2 Bytes
typedef unsigned long	DWORD;	//4 Bytes

#pragma pack(push)
#pragma pack(2)
typedef struct
{
	unsigned short	bfType;		/* "BM"(0x4D42) */
	unsigned long	bfSize;		/*  */
	unsigned short	bfReserved1;	/* */
	unsigned short	bfReserved2;	/* */
	unsigned long	bfoffBits;
} BITMAPFILEHEADER;

typedef struct
{
	unsigned long   biSize;		/*   size   of   BITMAPINFOHEADER   */
	unsigned long   biWidth;	/*   Î»Í¼¿í¶È(ÏñËØ)   */
	unsigned long   biHeight;	/*   Î»Í¼¸ß¶È(ÏñËØ)   */
	unsigned short  biPlanes;	/*   Ä¿±êÉè±¸µÄÎ»Æ½ÃæÊý,   ±ØÐëÖÃÎª1   */
	unsigned short  biBitCount;	/*   Ã¿¸öÏñËØµÄÎ»Êý,   1,4,8»ò24   */
	unsigned long   biCompress;	/*   Î»Í¼ÕóÁÐµÄÑ¹Ëõ·½·¨,0=²»Ñ¹Ëõ   */
	unsigned long   biSizeImage;	/*   Í¼Ïñ´óÐ¡(×Ö½Ú)   */
	unsigned long   biXPelsPerMeter;	/*   Ä¿±êÉè±¸Ë®Æ½Ã¿Ã×ÏñËØ¸öÊý   */
	unsigned long   biYPelsPerMeter;	/*   Ä¿±êÉè±¸´¹Ö±Ã¿Ã×ÏñËØ¸öÊý   */
	unsigned long   biClrUsed;		/*   Î»Í¼Êµ¼ÊÊ¹ÓÃµÄÑÕÉ«±íµÄÑÕÉ«Êý   */
	unsigned long   biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

struct bmp_bgr {
	__u8 blue;
	__u8 green;
	__u8 red;
};

#endif /* __SAVE_BMP_H_ */
