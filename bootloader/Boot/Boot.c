#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "Elf.h"
#include "File.h"
#include "Setup.h"

#ifdef GEM_LOG
#include "Log.h"
#endif

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,       // 内存镜像句柄
    IN EFI_SYSTEM_TABLE *SystemTable // 系统表
)
{
    EFI_STATUS StatusCode = EFI_SUCCESS;

    // 重定位Kernel.elf，获得内核入口
    EFI_PHYSICAL_ADDRESS KernelEntryPoint;
    StatusCode = RelocateElf(L"\\Kernel.elf", &KernelEntryPoint);
#ifdef GEM_LOG
    if (EFI_ERROR(StatusCode))
    {
        LogStatusCode(StatusCode, L"UefiMain", L"Relocate kernel entry point failed");
        return StatusCode;
    }
    LogStatusCode(StatusCode, L"UefiMain", L"Relocate kernel entry point success");
#endif
    int (*KernelEntry)();
    KernelEntry = (int (*)())KernelEntryPoint;
    int Value = KernelEntry();
    Print(L"Return value from Kernel = %d\n", Value);

    return StatusCode;
}