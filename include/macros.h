#ifndef _MACROS_H
#define _MACROS_H

#define ARRAYCOUNT(a) (sizeof(a) / sizeof(a[0]))
#define ARRAYCOUNT_S(a) (long)ARRAYCOUNT(a)
#define OFFSETOF(type, field) ((unsigned long)&(((type*)0)->field))

#endif
