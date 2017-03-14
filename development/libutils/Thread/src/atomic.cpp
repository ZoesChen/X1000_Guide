#ifdef __cplusplus
extern "C" {
#endif

#define ANDROID_ATOMIC_INLINE
#define ANDROID_SMP 0

#if defined(__i386__) || defined(__x86_64__)
#include<atomic-x86.h>
#elif defined(__mips__)
#include<atomic-mips.h>
#endif

#ifdef __cplusplus
}
#endif


