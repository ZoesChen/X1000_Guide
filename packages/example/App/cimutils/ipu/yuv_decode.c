#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct macroblock {
	unsigned char Y1[64];
	unsigned char Y2[64];
	unsigned char Y3[64];
	unsigned char Y4[64];
	unsigned char Cb[64];
	unsigned char Cr[64];
};

#define IMG_WIDTH 800
#define IMG_HEIGHT 600
#define RGB_SIZE (IMG_WIDTH * IMG_HEIGHT * 3)
#define MAXLEN (64 * 6)

int yuv_decode(unsigned char *y_buff, unsigned char *u_buff, unsigned char *v_buff)
{
	FILE *yuv_file;

	struct macroblock macroblock;

	yuv_file = fopen("YCbCrDat", "r");

	//	unsigned char *y_buff = malloc(IMG_WIDTH * IMG_HEIGHT);
	//	unsigned char *u_buff = malloc((IMG_WIDTH / 2) * (IMG_HEIGHT / 2));
	//	unsigned char *v_buff = malloc((IMG_WIDTH / 2) * (IMG_HEIGHT / 2));
	memset(y_buff, 0, IMG_WIDTH * IMG_HEIGHT);
	memset(u_buff, 0, (IMG_WIDTH / 2) * (IMG_HEIGHT / 2));
	memset(v_buff, 0, (IMG_WIDTH / 2) * (IMG_HEIGHT / 2));

	int i, j;
	int sx, sy;
	int x, y;
	int line = 0;
	int lmacro = 0;

#define MACRO_LINES ((IMG_HEIGHT + 15) / 16)
#define MACRO_PER_LINE ((IMG_WIDTH + 15) / 16)

	/* packed to planar */
	printf("convert packed to planar......\n");
	for (line = 0; line < MACRO_LINES; line++) {
		for (lmacro = 0; lmacro < MACRO_PER_LINE; lmacro ++) {
			memset(&macroblock, 0, sizeof(struct macroblock));
			int rc =fread((void *)&macroblock, sizeof(unsigned char),
				      sizeof(struct macroblock), yuv_file);
			if ( rc != MAXLEN ) {
				if (feof(yuv_file))
					goto success_out;
				fclose(yuv_file);
				printf("fread error!!! rc = %d\n", rc);
				return -1;
			}

#define PROCESS_8_8(block_field)					\
			for (j = 0; j < 8; j++) /* a 8pixel line one time */ \
				for (i = 0; i < 8; i++) {		\
					x = i + sx * 8;			\
					y = j + sy * 8;			\
									\
					if ((x < IMG_WIDTH) && (y <IMG_HEIGHT))	\
						*(y_buff + y * IMG_WIDTH + x) = *(macroblock.block_field + j * 8 + i); \
				}

			/* Y1 */
			sx = lmacro * 2;
			sy = line * 2;
			PROCESS_8_8(Y1);

			/* Y2 */
			sx = lmacro * 2  + 1;
			sy = line * 2;
			PROCESS_8_8(Y2);

			/* Y3 */
			sx = lmacro * 2;
			sy = line * 2 + 1;
			PROCESS_8_8(Y3);

			/* Y4 */
			sx = lmacro * 2 + 1;
			sy = line * 2 + 1;
			PROCESS_8_8(Y4);

			/* Cb && Cr */
			for (j = 0; j < 8; j++)
				for (i = 0; i < 8; i++) {
					x = i + lmacro * 8;
					y = j + line * 8;

					if ((x < (IMG_WIDTH / 2)) && (y < (IMG_HEIGHT / 2))) {
						*(u_buff + y * (IMG_WIDTH / 2) + x) = *(macroblock.Cb + j * 8 + i);
						*(v_buff + y * (IMG_WIDTH / 2) + x) = *(macroblock.Cr + j * 8 + i);
					}
				}
		}
	}

 success_out:
	return 0;
}
