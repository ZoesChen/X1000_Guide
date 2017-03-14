#ifndef __IPU_H__
#define __IPU_H__

extern int init_ipu_env();
extern int jz47_put_image_with_ipu (unsigned char *y, unsigned char *u, unsigned char *v, int img_width, int img_height);

#endif /* __IPU_H__ */

