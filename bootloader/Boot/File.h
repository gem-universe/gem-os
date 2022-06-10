#ifndef _GEM_BOOTLOADER_FILE_
#define _GEM_BOOTLOADER_FILE_

#include <Uefi.h>
#include <Guid/FileInfo.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>

#include "Setup.h"

EFI_STATUS
EFIAPI
LoadFile(
    IN CHAR16 *FileName,
    OUT EFI_PHYSICAL_ADDRESS *FileAddr
);

EFI_STATUS
EFIAPI
GetFileHandle(
    IN EFI_HANDLE ImageHandle,
    IN CHAR16 *FileName,
    OUT EFI_FILE_PROTOCOL **FileHandle
);

EFI_STATUS
EFIAPI
ReadFile(
    IN EFI_FILE_PROTOCOL *File,
    OUT EFI_PHYSICAL_ADDRESS *FileBase
);

#endif