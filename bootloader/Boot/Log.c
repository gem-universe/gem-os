#include <Library/UefiLib.h>

#include "Log.h"

VOID LogStatusCode(
    EFI_STATUS StatusCode,
    const CHAR16 *FuncName,
    const CHAR16 *Info)
{
    Print(L"(Code %d) %s: %s\n", StatusCode, FuncName, Info);
}