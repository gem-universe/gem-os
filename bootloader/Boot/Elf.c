#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "Elf.h"
#include "File.h"
#include "Setup.h"

#ifdef GEM_LOG
#include "Log.h"
#endif

EFI_STATUS
EFIAPI
CheckElf(
    IN EFI_PHYSICAL_ADDRESS ElfBufferAddr,
    IN UINTN ElfClass,
    IN UINTN ElfData)
{
    EFI_STATUS StatusCode = EFI_SUCCESS;

    // 检查魔数是否为0x7F454C46
    ELF64_EHDR *ElfHeader = (ELF64_EHDR *)ElfBufferAddr;
    UINT8 *Magic = ElfHeader->Ident.Magic;
    if (*Magic != ELF_MAG0 || *(Magic + 1) != ELF_MAG1 ||
        *(Magic + 2) != ELF_MAG2 || *(Magic + 3) != ELF_MAG3)
    {
        StatusCode = GEM_ELF_CHECK_FAILED;
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"CheckElf", L"Magic number is not 0x7F454C46");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"CheckElf", L"Check magic number success");
#endif

    // 检查32/64位
    if (ElfHeader->Ident.Class != ElfClass)
    {
        StatusCode = GEM_ELF_CHECK_FAILED;
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"CheckElf", L"Check class failed");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"CheckElf", L"Check class success");
#endif

    // 检查大小端
    if (ElfHeader->Ident.Data != ElfData)
    {
        StatusCode = GEM_ELF_CHECK_FAILED;
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"CheckElf", L"Check endianness failed");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"CheckElf", L"Check endianness success");
#endif
    return StatusCode;
}

EFI_STATUS
EFIAPI
RelocateElf(
    IN CHAR16 *FileName,
    OUT EFI_PHYSICAL_ADDRESS *RelocateEntryPointAddr)
{
    EFI_STATUS StatusCode = EFI_SUCCESS;

    // 加载文件到Buffer中
    EFI_PHYSICAL_ADDRESS FileBufferAddr;
    UINTN FileBufferPages;
    StatusCode = LoadFile(FileName, &FileBufferAddr, &FileBufferPages);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"RelocateElf", L"Load file failed");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"RelocateElf", L"Load file success");
#endif

    // 检查是否符合ELF文件格式
    StatusCode = CheckElf(FileBufferAddr, ELFCLASS64, ELFDATALE);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"RelocateElf", L"Check elf64 failed");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"RelocateElf", L"Check elf64 success");
#endif

    // 找出所有PT_LOAD段
    ELF64_EHDR *ElfHeader = (ELF64_EHDR *)FileBufferAddr;
    ELF64_PHDR *PHeader = (ELF64_PHDR *)(FileBufferAddr + ElfHeader->PhOff);
    EFI_PHYSICAL_ADDRESS LowAddr = 0xFFFFFFFFFFFFFFFFULL;
    EFI_PHYSICAL_ADDRESS HighAddr = 0;
    for (UINTN i = 0; i < ElfHeader->PhNum; i++)
    {
        if (PHeader[i].Type == PT_LOAD)
        {
            LowAddr = PHeader[i].PAddr < LowAddr ? PHeader[i].PAddr : LowAddr;
            HighAddr = (PHeader[i].PAddr + PHeader[i].MemSz) > HighAddr ? (PHeader[i].PAddr + PHeader[i].MemSz) : HighAddr;
        }
    }
    // 地址异常判断
    if (LowAddr >= HighAddr)
    {
        StatusCode = GEM_ELF_RELOCATE_FAILED;
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"RelocateElf", L"Segments PT_LOAD not found");
#endif
        return StatusCode;
    }

    UINTN ElfPages = (HighAddr - LowAddr + 4095) / 4096;
    EFI_PHYSICAL_ADDRESS RelocateAddr;
    // TODO 将内核加载到内存是应该随便分配地址，还是加载到指定地址？
    // 注意这里第二个参数是EfiLoaderCode，表明是Loader代码，有什么特殊作用？
    StatusCode = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderCode,
        ElfPages,
        &RelocateAddr);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"RelocateElf", L"Allocate pages for loader code failed");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"RelocateElf", L"Allocate pages for loader code success");
#endif

    // LowAddr为ELF文件中地址最小的PT_LOAD段的“物理地址”。
    // 通过计算RelocateOffset，每个段的“物理地址”加上RelocateOffset就是要复制到内存中的地址。
    UINTN RelocateOffset = RelocateAddr - LowAddr;
    for (UINTN i = 0; i < ElfHeader->PhNum; i++)
    {
        if (PHeader[i].Type == PT_LOAD)
        {
            gBS->CopyMem(
                (VOID *)(PHeader[i].PAddr + RelocateOffset),
                (VOID *)(FileBufferAddr + PHeader[i].Offset),
                PHeader[i].FileSz);
#ifdef GEM_LOG
            Print(L"Copy memory for PT_LOAD segment %d\n", i);
#endif
            // PT_LOAD在内存里的空间可能大于在文件中的空间，将多余内存空间填充0防止出现奇怪代码指令
            if (PHeader[i].MemSz > PHeader[i].FileSz)
            {
                gBS->SetMem(
                    (VOID *)(PHeader[i].PAddr + RelocateOffset + PHeader[i].FileSz),
                    PHeader[i].MemSz - PHeader[i].FileSz,
                    0);
#ifdef GEM_LOG
                Print(L"Clear extra meory for PT_LOAD segment %d\n", i);
#endif
            }
        }
    }

    // 返回重定位后的程序入口
    *RelocateEntryPointAddr = ElfHeader->Entry + RelocateOffset;
#ifdef GEM_LOG
    Print(L"Entry address: %016llx\n", *RelocateEntryPointAddr);
#endif

    // 释放ELF Buffer的空间
    // 踩坑，一定要最后FreePages，因为上面几行获得ElfHeader还指向ELF Buffer
    StatusCode = gBS->FreePages(FileBufferAddr, FileBufferPages);
    if (EFI_ERROR(StatusCode))
    {
#ifdef GEM_LOG
        LogStatusCode(StatusCode, L"RelocateElf", L"Free pages of ELF file buffer failed");
#endif
        return StatusCode;
    }
#ifdef GEM_LOG
    LogStatusCode(StatusCode, L"RelocateElf", L"Free pages of ELF file buffer success");
#endif
    return StatusCode;
}