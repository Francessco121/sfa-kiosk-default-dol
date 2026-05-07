#ifndef _SYS_DLL_H
#define _SYS_DLL_H

#include "types.h"
#include "macros.h"

#define MAX_LOADED_DLLS 128
#define DLL_NONE (-1)

typedef struct {
	s32 offset;
	s32 bssSize;
} DLLTabEntry;

typedef struct {
/*0000*/	s32 bank1; // modgfx
/*0004*/	s32 bank2; // projgfx
/*0008*/	s32 bank3; // unused, always 0
/*000C*/	s32 bank4; // objects
} DLLTabHeader;

typedef union {
/*0000*/	DLLTabHeader header;
/*0010*/	DLLTabEntry entries[1];
} DLLTab;

typedef struct {
/*0000*/	s32 tabidx;
/*0004*/	u32 refCount;
/*0008*/	u32 *vtblPtr;
/*000C*/	void *end;
} DLLState;

struct DLLFile;

typedef void (*DLLFunc)(struct DLLFile *self);

typedef struct DLLFile
{
	u32 code;
	u32 data;
	s32 rodata;
	u16 exportCount;
	DLLFunc ctor;
	DLLFunc dtor;
	// Exports table begins here
	// After relocation, the exports table becomes a vtable
} DLLFile;

#define DLL_FILE_TO_EXPORTS(dllFile) ((u32*)((u32)dllFile + sizeof(DLLFile)))
#define DLL_EXPORTS_TO_FILE(exports) ((DLLFile*)((u32)exports - sizeof(DLLFile)))

// Note: A DLL interface IS a pointer to a DLL state vtblPtr field (DLLState.vtblPtr)
#define DLL_INTERFACE_TO_STATE(interfacePtr) ((DLLState*)((u32)interfacePtr - OFFSETOF(DLLState, vtblPtr)))

void init_dll_system(void);

#endif
