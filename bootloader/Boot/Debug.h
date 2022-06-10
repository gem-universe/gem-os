#ifndef _GEM_BOOTLOADER_DEBUG_
#define _GEM_BOOTLOADER_DEBUG_

#include <Uefi.h>

VOID DebugStatusFormat(
    EFI_STATUS StatusCode,
    const CHAR16 *FuncName,
    const CHAR16 *Info);
#endif