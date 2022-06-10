#include <Library/UefiLib.h>

#include "Debug.h"

VOID DebugStatusFormat(
    EFI_STATUS StatusCode,
    const CHAR16 *FuncName,
    const CHAR16 *Info)
{
    Print(L"(Code %n) %s: %s\n", StatusCode, FuncName, Info);
}