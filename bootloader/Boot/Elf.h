#ifndef _GEM_BOOTLOADER_ELF_
#define _GEM_BOOTLOADER_ELF_

#include <Uefi.h>

#define ELFMAG "\177ELF" /* Magic Number 0x7F E L F */
#define ELFCLASS32 1
#define ELFCLASS64 2

typedef struct
{
    UINT8 Magic[4];   // ELF魔术: 7F454C46
    UINT8 Class;      // 文件版本。32位为1，64位为2
    UINT8 Data;       // 文件字节序。小端为1，大端为2
    UINT8 Version;    // ELF版本
    UINT8 OsAbi;      // 二进制接口标识位。Unix System V为0x00
    UINT8 AbiVersion; // ABI扩展位，Linux 2.6已弃用
    UINT8 Pad[7];     // 保留位
} ELF_EIDENT;

/* ELF Header */
typedef struct
{
    ELF_EIDENT Ident;  // ELF标识符
    UINT16 Type;      // 目标文件类型：可重定位文件、可执行文件、动态链接类型...
    UINT16 Machine;   // 指令集架构
    UINT32 Version;   // ELF版本
    UINT64 Entry;     // 执行入口（Entry Point）地址
    UINT64 PhOff;     // 程序头表（Program Header Table）偏移地址
    UINT64 ShOff;     // 节头表（Section Header Table）偏题地址
    UINT32 Flags;     // 标识位，与目标指令集架构有关
    UINT16 EhSize;    // ELF头部(Header)大小。0x34或0x40
    UINT16 PhEntSize; // 程序头表入口（Entry）大小
    UINT16 PhNum;     // 程序头表入口数量
    UINT16 ShEntSize; // 节头表入口（Entry）大小
    UINT16 ShNum;     // 节头入口数量
    UINT16 ShStrNdx;  // 节入口索引，包含节头表里节的名字
} ELF64_EHDR;

/* Program Header */
typedef struct
{
    UINT32 Type;   // 段（Segment）类型：PHDR，LOAD，DYNAMIC...
    UINT32 Flags;  // 64位独有标识位
    UINT64 Offset; // 段在文件中的偏移
    UINT64 VAddr;  // 段的虚拟地址
    UINT64 PAddr;  // 段的物理地址
    UINT64 FileSz; // 本段在文件中的大小
    UINT64 MemSz;  // 本段在内存中的大小，可能与FileSize不同
    UINT64 Align;  // 对齐
} ELF64_PHDR;

EFI_STATUS
EFIAPI
CheckElf64(
    IN EFI_PHYSICAL_ADDRESS PAddr
);

EFI_STATUS
EFIAPI
RelocateElf(
    IN CHAR16 *FileName,
    OUT EFI_PHYSICAL_ADDRESS *RelocateAddr
);

#endif