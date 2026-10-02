#ifndef _SYS_ASSET_H
#define _SYS_ASSET_H

#include "dolphin/types.h"

void assetRomLoad(void **dest, s32 fileId);
void assetLoadDLL(void **dest, s32 idOrIdx, s32 exportCount);

#endif
