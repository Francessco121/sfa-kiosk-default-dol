#ifndef _SYS_PRINT_H
#define _SYS_PRINT_H

#include "dolphin/types.h"

int diPrintf(const char *format, ...);
void warnPrintf(const char *format, ...);
void errorPrintf(const char *format, ...);

#endif
