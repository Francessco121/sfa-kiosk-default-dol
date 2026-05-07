#ifndef _SYS_MEMORY_H
#define _SYS_MEMORY_H

#include "types.h"

void *mmAlloc(s32 size, s32 tag, const char *name);

#endif
