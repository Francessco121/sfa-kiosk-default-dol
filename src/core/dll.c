#include "dolphin/types.h"
#include "core/dll.h"
#include "core/filesystem.h"
#include "core/memory.h"

extern void fn_80077A10(void **dest, s32 idOrIdx, s32 exportCount);
extern void fn_8018FE5C(const char *fmt, ...);
extern void fn_8007BDA4(void *address);
extern void fn_8018FD84(const char *fmt, ...);

extern void *lbl_802E8B60[764];

u32 *gFile_DLLSIMPORTTAB;
s32 gDLLCount;
DLLTab *gFile_DLLS_TAB;
s32 gLoadedDLLCount;
DLLState *gLoadedDLLList;

void init_dll_system(void) {
    queue_alloc_load_file((void**)&gFile_DLLS_TAB, 0x43);
    queue_alloc_load_file((void**)&gFile_DLLSIMPORTTAB, 0x44);
    
    gDLLCount = 2; 
    while (((s32*)gFile_DLLS_TAB)[gDLLCount * 2] != -1) {
        ++gDLLCount;
    }

    gLoadedDLLList = (DLLState*)mmAlloc(sizeof(DLLState) * MAX_LOADED_DLLS, 4, "dllTab");

    gLoadedDLLCount = MAX_LOADED_DLLS;
    while (gLoadedDLLCount != 0) {
        gLoadedDLLList[--gLoadedDLLCount].tabidx = DLL_NONE;
    }
}

void *dll_load_deferred(u16 idOrIdx, u16 exportCount) {
    DLLFile *dll;
    DLLState *state;
    void *dllInterfacePtr;

    dllInterfacePtr = NULL;
    
    if (!idOrIdx) {
        return NULL;
    }

    fn_80077A10(&dllInterfacePtr, idOrIdx, exportCount);

    state = DLL_INTERFACE_TO_STATE(dllInterfacePtr);

    if (state->refCount == 1) {
        // A DLL interface is a pointer to a DLL state exports field.
        // Dereferencing the state exports field gives us a pointer to the DLL file exports array.
        // Using the file exports array address, we can get the DLL file instance.
        dll = DLL_EXPORTS_TO_FILE(*(void**)dllInterfacePtr);
        if (dll->ctor != NULL) {
            dll->ctor(dll);
        }
    }

    return dllInterfacePtr;
}

void* dll_load(u16 arg0, u16 arg1, s32 arg2) {
    DLLFile* temp_r29;
    u32 var_r31;
    s32 sp10;
    void **tbl;
    u32** temp_r27;

    tbl = lbl_802E8B60;
    if (arg0 >= 0x8000) {
        arg0 -= 0x8000;
        arg0 += gFile_DLLS_TAB->header.bank4;
    } else if (arg0 >= 0x2000) {
        arg0 -= 0x2000;
        arg0 += gFile_DLLS_TAB->header.bank2 + 1;
    } else if (arg0 >= 0x1000) {
        arg0 -= 0x1000;
        arg0 += gFile_DLLS_TAB->header.bank1 + 1;
    }
    
    arg0 = arg0 - 1;
    
    for (var_r31 = 0; var_r31 < gLoadedDLLCount; var_r31++) {
        if (arg0 == gLoadedDLLList[var_r31].tabidx) {
            gLoadedDLLList[var_r31].refCount += 1;
            return &gLoadedDLLList[var_r31].vtblPtr;
        }
    }

    temp_r29 = tbl[arg0];
    if (temp_r29 == NULL) {
        return NULL;
    }
    
    if (temp_r29->exportCount < arg1) {
        fn_8018FE5C("DLLS: warning DLL entrypoint mismatch, dll %d (%d/%d).\n", 
            arg0, temp_r29->exportCount, arg1);
    }
    
    for (var_r31 = 0; var_r31 < gLoadedDLLCount; var_r31++) {
        if (gLoadedDLLList[var_r31].tabidx == -1) {
            break;
        }
    }
    
    if (var_r31 == gLoadedDLLCount) {
        if (gLoadedDLLCount == 0x80) {
            fn_8018FD84("DLLS: Maximum DLL's loaded, %d.\n", gLoadedDLLCount);
            fn_8007BDA4(temp_r29);
            return NULL;
        }
        gLoadedDLLCount += 1;
    }

    gLoadedDLLList[var_r31].tabidx = arg0;
    gLoadedDLLList[var_r31].vtblPtr = DLL_FILE_TO_EXPORTS(temp_r29);
    gLoadedDLLList[var_r31].end = (void*)((u32)temp_r29 + sp10);
    gLoadedDLLList[var_r31].refCount = 1;
    temp_r27 = &gLoadedDLLList[var_r31].vtblPtr;
    
    if ((arg2 != 0) && (temp_r29->ctor != NULL)) {
        temp_r29->ctor(temp_r29);
    }
    
    return temp_r27;
}

static char str_802e97ac[] = "DLLS: Load failed, DLL %d currently executing.\n";

s32 dll_unload(void* dllInterfacePtr) {
    u16 idx;
    DLLFile* dll;

    idx = (u8*)dllInterfacePtr - (u8*)&gLoadedDLLList->vtblPtr;
    if (idx & 0xF) {
        fn_8018FD84("DLLS: free fail, DLL not loaded.\n");
        return FALSE;
    }
    idx /= 16;
    if (idx >= gLoadedDLLCount) {
        fn_8018FD84("DLLS: free fail, DLL not loaded.\n");
        return FALSE;
    }
    gLoadedDLLList[idx].refCount--;
    if (gLoadedDLLList[idx].refCount == 0) {
        dll = DLL_EXPORTS_TO_FILE(gLoadedDLLList[idx].vtblPtr);
        if (dll->dtor != NULL) {
            dll->dtor(dll);
        }
        gLoadedDLLList[idx].tabidx = -1;
        while (gLoadedDLLCount != 0) {
            if (gLoadedDLLList[gLoadedDLLCount - 1].tabidx != -1) {
                break;
            }
            gLoadedDLLCount -= 1;
        }
        return TRUE;
    }
    return FALSE;
}

static char str_802e9800[] = "warning: using default DLL entry point.\n";
static char str_802e982c[] = "DLL %d usage %d %08x:%08x\n";
