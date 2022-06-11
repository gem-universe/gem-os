#ifndef _GEM_BOOTLOADER_LOG_
#define _GEM_BOOTLOADER_LOG_

#include <Uefi.h>

VOID LogStatusCode(
    EFI_STATUS StatusCode,
    const CHAR16 *FuncName,
    const CHAR16 *Info);
#endif