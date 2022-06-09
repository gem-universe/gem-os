#include <Uefi.h>
#include <Library/UefiLib.h>

EFI_STATUS // 函数返回值为函数运行的状态
EFIAPI // 只是一个标识，表示符合EFI规范
UefiMain(
    EFI_HANDLE ImageHandle, // 内存镜像句柄
    EFI_SYSTEM_TABLE *SystemTable // 系统表
)
{
    Print(L"Hello, World!\n"); // UEFI每个字符用两字节，与ASCII不同，因此字符串前面加L
    return 0;
}