#ifndef _GEM_BOOTLOADER_FILE_
#define _GEM_BOOTLOADER_FILE_

#include <Uefi.h>
#include <Protocol/SimpleFileSystem.h>

#include "Setup.h"

/** 将文件加载到Buffer中。
 * 内部调用了AllocatePages，记得调用FreePages释放。 */
EFI_STATUS
EFIAPI
LoadFile(
    IN CHAR16 *FileName,
    OUT EFI_PHYSICAL_ADDRESS *FileBufferAddr,
    OUT UINTN *FilePages);

#endif