#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>

#undef grab_dbg
extern int debug_level;
#define grab_dbg(x...)				\
	({					\
		if (debug_level > 0)		\
			fprintf(stdout, x);	\
	})
