#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "File.h"

EFI_STATUS // 函数返回值为函数运行的状态
EFIAPI // 只是一个标识，表示符合EFI规范
UefiMain(
    IN EFI_HANDLE ImageHandle, // 内存镜像句柄
    IN EFI_SYSTEM_TABLE *SystemTable // 系统表
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_FILE_PROTOCOL *Bin;
    Status = GetFileHandle(ImageHandle, L"\\Kernel.bin", &Bin);
    EFI_PHYSICAL_ADDRESS BinAddress;
    Status = ReadFile(Bin, &BinAddress);
    
    asm("jmp %0": : "m"(BinAddress));

    return Status;
}