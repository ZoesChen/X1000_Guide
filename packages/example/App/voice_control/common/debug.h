#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <stdlib.h>

#define IOT_WARNIG printf

#define DEBUG_LINE(aaa, bbb...) printf("%d %s %s\n", __LINE__, __FILE__,  __FUNCTION__)


#endif	/* __DEBUG_H__ */
