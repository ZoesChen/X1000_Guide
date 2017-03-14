#ifndef _GUIDE_KEY_H_
#define _GUIDE_KEY_H_

#define KEY_DEVICE "/dev/input/event0"
#define X86_ESC   1
#define X86_KEY1 2
#define X86_KEY2 3
#define X86_KEY3 4
#define X86_KEY4 5
#define X86_KEY5 6
#define X86_KEY6 7
#define X86_KEY7 8
#define X86_KEY8 9
#define X86_KEY9 10
#define X86_ZERO 11
#define X86_KEY_A 30

int OpenKeyDev();
int ReadKey(int *keyCode);
void CloseKeyDev();
#endif
