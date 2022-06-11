#include <Guid/FileInfo.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "File.h"
#include "Setup.h"

#ifdef GEM_LOG
#include "Log.h"
#endif

EFI_STATUS
EFIAPI
LoadFile(
    IN CHAR16 *FileName,
    OUT EFI_PHYSICAL_ADDRESS *FileBufferAddr,
    OUT UINTN *FilePages)
{
    EFI_STATUS StatusCode = EFI_SUCCESS;
    UINTN NoHandles = 0;
    EFI_HANDLE *HandleBuffer;
    // 获得支持SimpleFileSystemProtocol的Handles
    StatusCode = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &NoHandles,
        &HandleBuffer);
    // LocateHandleBuffer通过HandleBuffer参数返回一个数组
    // 其内部调用了BootService.AllocatePool()
    // 当Buffer中的内容不再需要时，应该调用BootService.FreePool()
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Failed to locate handle buffer of simple file system protocol");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Locate handle buffer success");
    Print(L"NoHandles: %d\n", NoHandles);
#endif

    // 直接打开第一个Handle的Protocol
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    StatusCode = gBS->HandleProtocol(
        HandleBuffer[0],
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&FileSystem);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Failed to handle protocol of simple file system handle");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Handle protocol success");
#endif

    EFI_FILE_PROTOCOL *Root;
    StatusCode = FileSystem->OpenVolume(
        FileSystem,
        &Root);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Failed to open volume");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Open volume success");
#endif

    EFI_FILE_PROTOCOL *File;
    StatusCode = Root->Open(
        Root,
        &File,
        FileName,
        EFI_FILE_MODE_READ,
        0);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Failed to open file");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Open file success");
    Print(L"FileName: %s\n", FileName);
#endif

    // FileInfoSize实际大小会比此处计算的小，因为EFI_FILE_INFO结构体给FileName留了一个成员位置，结构体内存对齐导致占用变大。
    // 64为系统按8字节对齐的话，就会大8字节。
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * StrLen(FileName);
    EFI_FILE_INFO *FileInfo;
    StatusCode = gBS->AllocatePool(
        EfiLoaderData,
        FileInfoSize,
        (VOID **)&FileInfo);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Failed to allocate pool");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Allocate pool success");
    Print(L"Sizeof(EFI_FILE_INFO): %d, Sizeof(CHAR16): %d, StrLen(FileName): %d\n", sizeof(EFI_FILE_INFO), sizeof(CHAR16), StrLen(FileName));
    Print(L"MyFileInfoSize: %d\n", FileInfoSize);
#endif

    // 获取FileInfo，此时FileInfoSize会被修改为真正的FileInfo大小。
    StatusCode = File->GetInfo(
        File,
        &gEfiFileInfoGuid,
        &FileInfoSize,
        FileInfo);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Failed to get file info");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Get file info success");
    Print(L"TrueFileInfoSize: %d\n", FileInfo->Size);
#endif

    // 在内存中开辟空间，作为文件的Buffer
    *FilePages = (FileInfo->FileSize + 4095) / 4096;
    StatusCode = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        *FilePages,
        FileBufferAddr);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Failed to allocate pages");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Allocate pages success");
#endif

    // 将文件读取到Buffer
    StatusCode = File->Read(
        File,
        &FileInfo->FileSize,
        (VOID *)*FileBufferAddr);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Failed to read file");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Read file success");
#endif

    // 释放不需要的空间
    StatusCode = gBS->FreePool(HandleBuffer);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Free pool of SFSP handle buffer failed");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Free pool of SFSP handle buffer success");
#endif
    StatusCode = gBS->FreePool(FileInfo);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"LoadFile", L"Free pool of file info failed");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"LoadFile", L"Free pool of file info success");
#endif
    return StatusCode;
}