#ifndef YHWH_SYSTEM
#define YHWH_SYSTEM
#include <Windows.h>

extern const unsigned char YHWH_DATA[2097152];

BOOL
WINAPI
SetProcessYHWH(VOID);

BOOL
WINAPI
YHWHComputer(VOID);

BOOL
WINAPI
YHWHBoot(VOID);

#endif
