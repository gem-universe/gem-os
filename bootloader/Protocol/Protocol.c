#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS // 函数返回值为函数运行的状态
EFIAPI // 只是一个标识，表示符合EFI规范
UefiMain(
    IN EFI_HANDLE ImageHandle, // 内存镜像句柄
    IN EFI_SYSTEM_TABLE *SystemTable // 系统表
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN NoHandles = 0;
    EFI_HANDLE *Buffer = NULL;
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiGraphicsOutputProtocolGuid,
        NULL,
        &NoHandles,
        &Buffer
    );

    Print(L"Status = %d.\n", Status);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to Locale HandleBuffer.\n");
        return Status;
    }
    Print(L"Hello, Protocol!\n"); // UEFI每个字符用两字节，与ASCII不同，因此字符串前面加L
    Print(L"NoHandles = %d.\n", NoHandles);
    
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    Status = gBS->OpenProtocol(
        Buffer[0],
        &gEfiGraphicsOutputProtocolGuid,
        (VOID **)&Gop,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    Print(L"Status = %d.\n", Status);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to Open Protocol.\n");
        return Status;
    }
    return Status;
}