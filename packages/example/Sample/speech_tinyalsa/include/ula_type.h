#ifndef __ULA_TYPE_H__
#define __ULA_TYPE_H__

typedef char            S8;
typedef unsigned char   U8;
typedef short int       S16;
typedef unsigned short  U16;
typedef int             S32;
typedef unsigned int    U32;
typedef void            VOID;
typedef double          DOUBLE;
typedef float           FLOAT;

static const U8 DOUBLE_SZ = sizeof(DOUBLE);
static const U8 S32_SZ = sizeof(S32);
static const U8 S16_SZ = sizeof(S16);
static const U8 S8_SZ = sizeof(S8);
#endif