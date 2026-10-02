#include "dolphin/types.h"
#include "core/dll.h"
#include "core/asset.h"
#include "core/memory.h"
#include "core/pi.h"
#include "core/print.h"

extern void *lbl_802E8B60[764];

u32 *gFile_DLLSIMPORTTAB;
s32 gDLLCount;
DLLTab *gFile_DLLS_TAB;
s32 gLoadedDLLCount;
DLLState *gLoadedDLLList;

void dllInit(void) {
    assetRomLoad((void**)&gFile_DLLS_TAB, DLLS_TAB);
    assetRomLoad((void**)&gFile_DLLSIMPORTTAB, DLLSIMPORTTAB_BIN);
    
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

void *dllLoad(u16 idOrIdx, u16 exportCount) {
    DLLFile *dll;
    DLLState *state;
    void *dllInterfacePtr;

    dllInterfacePtr = NULL;
    
    if (!idOrIdx) {
        return NULL;
    }

    assetLoadDLL(&dllInterfacePtr, idOrIdx, exportCount);

    state = DLL_INTERFACE_TO_STATE(dllInterfacePtr);

    if (state->refCount == 1) {
        // A DLL interface is a pointer to a DLL state exports field.
        // Dereferencing the state exports field gives us a pointer to the DLL file exports array.
        // Using the file exports array address, we can get the DLL file instance.
        dll = DLL_EXPORTS_TO_FILE(*(void**)dllInterfacePtr);
        if (dll->ctor) {
            dll->ctor(dll);
        }
    }

    return dllInterfacePtr;
}

void* dllLoadActual(u16 idOrIdx, u16 exportCount, s32 bRunConstructor) {
    DLLFile* dll;
    u32 i;
    s32 totalSize;
    void **tbl;
    u32** interfacePtr;

    tbl = lbl_802E8B60;
    // Convert ID to tab index
    if (idOrIdx >= 0x8000) {
        idOrIdx -= 0x8000;
        idOrIdx += gFile_DLLS_TAB->header.bank4;
    } else if (idOrIdx >= 0x2000) {
        idOrIdx -= 0x2000;
        idOrIdx += gFile_DLLS_TAB->header.bank2 + 1;
    } else if (idOrIdx >= 0x1000) {
        idOrIdx -= 0x1000;
        idOrIdx += gFile_DLLS_TAB->header.bank1 + 1;
    }
    
    idOrIdx = idOrIdx - 1;
    
    // Check if DLL is already loaded, and if so, increment the reference count
    for (i = 0; i < gLoadedDLLCount; i++) {
        if (idOrIdx == gLoadedDLLList[i].tabidx) {
            gLoadedDLLList[i].refCount += 1;
            return &gLoadedDLLList[i].vtblPtr;
        }
    }

    dll = tbl[idOrIdx];
    if (!dll) {
        return NULL;
    }
    
    if (dll->exportCount < exportCount) {
        errorPrintf("DLLS: warning DLL entrypoint mismatch, dll %d (%d/%d).\n", 
            idOrIdx, dll->exportCount, exportCount);
    }
    
    // Find an open slot in the DLL list
    for (i = 0; i < gLoadedDLLCount; i++) {
        if (gLoadedDLLList[i].tabidx == DLL_NONE) {
            break;
        }
    }
    
    // If no open slots were available, try to add a new slot
    if (i == gLoadedDLLCount) {
        if (gLoadedDLLCount == MAX_LOADED_DLLS) {
            warnPrintf("DLLS: Maximum DLL's loaded, %d.\n", gLoadedDLLCount);
            mmFree(dll); // @bug: DLLs are not allocated in this build!
            return NULL;
        }
        gLoadedDLLCount += 1;
    }

    gLoadedDLLList[i].tabidx = idOrIdx;
    gLoadedDLLList[i].vtblPtr = DLL_FILE_TO_EXPORTS(dll);
    gLoadedDLLList[i].end = (void*)((u32)dll + totalSize);
    gLoadedDLLList[i].refCount = 1;
    // A pointer to the vtable pointer is the interface of the DLL
    interfacePtr = &gLoadedDLLList[i].vtblPtr;
    
    if ((bRunConstructor != 0) && (dll->ctor)) {
        dll->ctor(dll);
    }
    
    return (void*)interfacePtr;
}

static char str_802e97ac[] = "DLLS: Load failed, DLL %d currently executing.\n";

s32 dllFree(void* dllInterfacePtr) {
    u16 idx;
    DLLFile* dll;

    idx = (u8*)dllInterfacePtr - (u8*)&gLoadedDLLList->vtblPtr;
    if (idx & 0xF) {
        warnPrintf("DLLS: free fail, DLL not loaded.\n");
        return FALSE;
    }
    idx /= 16;
    if (idx >= gLoadedDLLCount) {
        warnPrintf("DLLS: free fail, DLL not loaded.\n");
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
