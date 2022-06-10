// #include <Library/UefiBootServicesTableLib.h>

#include "File.h"
#include "Setup.h"

#ifdef DEBUG
#include "Debug.h"
#endif

EFI_STATUS
EFIAPI
LoadFile(
    IN CHAR16 *FileName,
    OUT EFI_PHYSICAL_ADDRESS *FileAddr)
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
#ifdef DEBUG
        DebugStatusFormat(StatusCode, L"LoadFile", L"Failed to locate handle buffer of simple file system protocol");
#endif
        return StatusCode;
    }
#ifdef DEBUG
    DebugStatusFormat(StatusCode, L"LoadFile", L"Locate handle buffer success");
    Print(L"NoHandles: %n\n", NoHandles);
#endif

    // 直接打开第一个Handle的Protocol
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    StatusCode = gBS->HandleProtocol(
        HandleBuffer[0],
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&FileSystem);
    if (EFI_ERROR(StatusCode))
    {
#ifdef DEBUG
        DebugStatusFormat(StatusCode, L"LoadFile", L"Failed to handle protocol of simple file system handle");
#endif
        return StatusCode;
    }
#ifdef DEBUG
    DebugStatusFormat(StatusCode, L"LoadFile", L"Handle protocol success");
#endif

    EFI_FILE_PROTOCOL *Root;
    StatusCode = FileSystem->OpenVolume(
        FileSystem,
        &Root);
    if (EFI_ERROR(StatusCode))
    {
#ifdef DEBUG
        DebugStatusFormat(StatusCode, L"LoadFile", L"Failed to open volume");
#endif
        return StatusCode;
    }
#ifdef DEBUG
    DebugStatusFormat(StatusCode, L"LoadFile", L"Open volume success");
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
#ifdef DEBUG
        DebugStatusFormat(StatusCode, L"LoadFile", L"Failed to open file");
#endif
        return StatusCode;
    }
#ifdef DEBUG
    DebugStatusFormat(StatusCode, L"LoadFile", L"Open file success");
    Print(L"FileName: %s\n", FileName);
#endif
    
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * StrLen(FileName) + 2;
}

EFI_STATUS
EFIAPI
GetFileHandle(
    IN EFI_HANDLE ImageHandle,
    IN CHAR16 *FileName,
    OUT EFI_FILE_PROTOCOL **FileHandle)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN NoHandles = 0;
    EFI_HANDLE *HandleBuffer;
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &NoHandles,
        &HandleBuffer);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    Status = gBS->OpenProtocol(
        HandleBuffer[0],
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&FileSystem,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    EFI_FILE_PROTOCOL *Root;
    Status = FileSystem->OpenVolume(
        FileSystem,
        &Root);

    Status = Root->Open(
        Root,
        FileHandle,
        FileName, // L"\\Logo.bmp"
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    return Status;
}

EFI_STATUS
EFIAPI
ReadFile(
    IN EFI_FILE_PROTOCOL *File,
    OUT EFI_PHYSICAL_ADDRESS *FileBase)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_FILE_INFO *FileInfo;

    UINTN InfoSize = sizeof(EFI_FILE_INFO) + 128;
    // 开辟内存空间为FileInfo结构体大小加上CHAR16格式文件名长度
    // 这里原作者偷懒没计算长度，直接加了128
    Status = gBS->AllocatePool(
        EfiLoaderData,
        InfoSize,
        (VOID **)&FileInfo);

    Status = File->GetInfo(
        File,
        &gEfiFileInfoGuid,
        &InfoSize,
        FileInfo);

    UINTN FilePageSize = (FileInfo->FileSize >> 12) + 1;
    // 这里加1是因为，每页4K,文件不一定是4K的整数倍
    // 可以再斟酌是否需要加一

    EFI_PHYSICAL_ADDRESS FileBufferAddress;
    Status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        FilePageSize,
        &FileBufferAddress);

    UINTN ReadSize = FileInfo->FileSize;
    Status = File->Read(
        File,
        &ReadSize,
        (VOID *)FileBufferAddress);

    gBS->FreePool(FileInfo);
    *FileBase = FileBufferAddress;
    return Status;
}