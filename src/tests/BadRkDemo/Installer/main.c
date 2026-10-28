/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2017
*
*  TITLE:       MAIN.C
*
*  VERSION:     1.10
*
*  DATE:        14 May 2017
*
*  Installer for BadRkDemo BSOD generator.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/

#if !defined UNICODE
#error ANSI build is not supported
#endif

//
// Ignored warnings
//
#pragma warning(disable: 4005) // macro redefinition
#pragma warning(disable: 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable: 6102) // Using %s from failed function call at line %u
#pragma warning(disable: 6320) // Exception-filter expression is the constant EXCEPTION_EXECUTE_HANDLER
#if (_MSC_VER >= 1900)
#pragma warning(disable: 4091) // 'typedef ': ignored on left of '' when no variable is declared
#pragma warning(disable: 4311) // 'type cast': pointer truncation from %s to %s
#pragma warning(disable: 4312) // 'type cast': conversion from %s to %s of greater size
#endif

#include <Windows.h>
#include "minirtl\minirtl.h"
#include "minirtl\cmdline.h"
#include "ntos.h"
#include <ntstatus.h>

#define PGDEMOREGDRV L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\PGDemo"

VOID ConsolePrint(
    _In_ LPCWSTR Text)
{
    CHAR Utf8Buffer[512];
    INT Utf8Length;
    DWORD Written;
    HANDLE Output = GetStdHandle(STD_OUTPUT_HANDLE);

    if ((Output != NULL) && (Output != INVALID_HANDLE_VALUE)) {
        if (WriteConsoleW(Output, Text, (DWORD)_strlen(Text), &Written, NULL)) {
            WriteConsoleW(Output, L"\r\n", 2, &Written, NULL);
        }
        else {
            Utf8Length = WideCharToMultiByte(CP_UTF8, 0, Text, -1,
                Utf8Buffer, sizeof(Utf8Buffer) - 3, NULL, NULL);
            if (Utf8Length > 0) {
                Utf8Buffer[Utf8Length - 1] = '\r';
                Utf8Buffer[Utf8Length] = '\n';
                WriteFile(Output, Utf8Buffer, Utf8Length + 1, &Written, NULL);
            }
        }
    }
}

VOID ConsolePrintStatus(
    _In_ LPCWSTR Prefix,
    _In_ ULONG Status)
{
    WCHAR Buffer[160];

    RtlSecureZeroMemory(Buffer, sizeof(Buffer));
    _strcpy(Buffer, Prefix);
    _strcat(Buffer, L"0x");
    ultohex(Status, _strend(Buffer));
    ConsolePrint(Buffer);
}

/*
* NativeAdjustPrivilege
*
* Purpose:
*
* Enable single privilege.
*
*/
NTSTATUS NativeAdjustPrivilege(
    _In_ ULONG Privilege
)
{
    NTSTATUS Status;
    HANDLE TokenHandle;

    LUID Luid;
    TOKEN_PRIVILEGES TokenPrivileges;

    Luid.LowPart = Privilege;
    Luid.HighPart = 0;

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = Luid;
    TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    Status = NtOpenProcessToken(
        NtCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &TokenHandle);

    if (NT_SUCCESS(Status)) {
        Status = NtAdjustPrivilegesToken(
            TokenHandle,
            FALSE,
            &TokenPrivileges,
            sizeof(TOKEN_PRIVILEGES),
            (PTOKEN_PRIVILEGES)NULL,
            NULL);

        NtClose(TokenHandle);
    }

    if (Status == STATUS_NOT_ALL_ASSIGNED)
        Status = STATUS_PRIVILEGE_NOT_HELD;

    return Status;
}

/*
* NativeLoadDriver
*
* Purpose:
*
* Write required registry settings and load driver.
*
*/
NTSTATUS NativeLoadDriver(
    _In_ PWSTR DrvFullPath,
    _In_ PWSTR KeyName,
    _In_opt_ PWSTR DisplayName,
    _In_ BOOL ReloadDrv
)
{
    UNICODE_STRING ValueName, drvName;
    OBJECT_ATTRIBUTES attr;

    HANDLE hDrvKey;
    ULONG data, dataSize = 0;
    NTSTATUS ns = STATUS_UNSUCCESSFUL;
    hDrvKey = NULL;

    __try
    {
        if (!ARGUMENT_PRESENT(KeyName)) {
            ns = STATUS_OBJECT_NAME_NOT_FOUND;
            __leave;
        }

        RtlInitUnicodeString(&drvName, KeyName);
        InitializeObjectAttributes(&attr, &drvName, OBJ_CASE_INSENSITIVE, 0, NULL);
        ns = NtCreateKey(&hDrvKey, KEY_ALL_ACCESS, &attr, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);
        if (!NT_SUCCESS(ns)) {
            __leave;
        }

        if (ARGUMENT_PRESENT(DrvFullPath)) {
            RtlInitUnicodeString(&ValueName, L"ImagePath");
            dataSize = (ULONG)(1 + _strlen(DrvFullPath)) * sizeof(WCHAR);
            ns = NtSetValueKey(hDrvKey, &ValueName, 0, REG_EXPAND_SZ, (PVOID)DrvFullPath, dataSize);
            if (!NT_SUCCESS(ns)) {
                __leave;
            }
        }

        data = 1;
        RtlInitUnicodeString(&ValueName, L"Type");
        ns = NtSetValueKey(hDrvKey, &ValueName, 0, REG_DWORD, (PVOID)&data, sizeof(DWORD));
        if (!NT_SUCCESS(ns)) {
            __leave;
        }

        data = 3;
        RtlInitUnicodeString(&ValueName, L"Start");
        ns = NtSetValueKey(hDrvKey, &ValueName, 0, REG_DWORD, (PVOID)&data, sizeof(DWORD));
        if (!NT_SUCCESS(ns)) {
            __leave;
        }

        data = SERVICE_ERROR_NORMAL;
        RtlInitUnicodeString(&ValueName, L"ErrorControl");
        ns = NtSetValueKey(hDrvKey, &ValueName, 0, REG_DWORD, (PVOID)&data, sizeof(DWORD));
        if (!NT_SUCCESS(ns)) {
            __leave;
        }

        if (ARGUMENT_PRESENT(DisplayName)) {
            RtlInitUnicodeString(&ValueName, L"DisplayName");
            dataSize = (ULONG)(1 + _strlen(DisplayName)) * sizeof(WCHAR);
            ns = NtSetValueKey(hDrvKey, &ValueName, 0, REG_SZ, DisplayName, dataSize);
            if (!NT_SUCCESS(ns)) {
                __leave;
            }
        }
        NtClose(hDrvKey);
        hDrvKey = NULL;

        ns = NtLoadDriver(&drvName);
        if (ns == STATUS_IMAGE_ALREADY_LOADED) {
            if (ReloadDrv == TRUE) {
                NtUnloadDriver(&drvName); //unload previous driver version
                NtYieldExecution();
                ns = NtLoadDriver(&drvName);
            }
            else {
                ns = STATUS_SUCCESS;
            }
        }

    }
    __finally {
        if (hDrvKey != NULL) {
            NtClose(hDrvKey);
        }
    }
    return ns;
}

#define PGDEMO_SET_TEST_TYPE    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define TT_DRIVER_LIST          0
#define TT_CR4                  1
#define TT_DRIVER_OBJECT        2
#define TT_NOTIFY_CALLOUT       3

typedef struct _INOUT_PARAM {
    ULONG TestType;
} INOUT_PARAM, *PINOUTPARAM;


/*
* CallDriver
*
* Purpose:
*
* Send request to the driver.
*
*/
BOOL CallDriver(
    ULONG TestType,
    BOOL ReportErrors)
{
    BOOL            Result = FALSE;
    HANDLE          h;
    INOUT_PARAM     tmp;
    DWORD           bytesIO;

    h = CreateFile(TEXT("\\\\.\\PGDemo"), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (h != INVALID_HANDLE_VALUE) {

        tmp.TestType = TestType;

        Result = DeviceIoControl(h, PGDEMO_SET_TEST_TYPE,
            &tmp, sizeof(tmp), &tmp,
            sizeof(tmp), &bytesIO, NULL);

        if (Result) {
            ConsolePrintStatus(L"[PGDemo] Test request accepted, type = ", TestType);
        }
        else if (ReportErrors) {
            ConsolePrintStatus(L"[PGDemo] DeviceIoControl failed, Win32 error = ", GetLastError());
        }

        CloseHandle(h);
    }
    else if (ReportErrors) {
        ConsolePrintStatus(L"[PGDemo] Cannot open \\\\.\\PGDemo, Win32 error = ", GetLastError());
    }

    return Result;
}

typedef struct _CPUInfo {
    int eax;
    int ebx;
    int ecx;
    int edx;
} CPUInfo, *PCPUInfo;

/*
* IsSmepSupported
*
* Purpose:
*
* Return TRUE if SMEP supported by current CPU, FALSE otherwise.
*
*/
BOOL IsSmepSupported(
    VOID)
{
    CPUInfo cpuInfo = { 0, 0, 0, 0 };

    __cpuid((int*)&cpuInfo, 7);

    if (cpuInfo.ebx & (1 << 7)) {
        return TRUE;
    }
    return FALSE;
}

/*
* main
*
* Purpose:
*
* Program entry point.
*
*/
int wmain(void)
{
    NTSTATUS Status;
    ULONG l, TestType = 0;

    HANDLE Link = NULL;

    UNICODE_STRING str, drvname;
    OBJECT_ATTRIBUTES Obja;

    WCHAR szBuffer[MAX_PATH + 1];

    l = 0;
    RtlSecureZeroMemory(szBuffer, sizeof(szBuffer));
    GetCommandLineParam(GetCommandLine(), 1, (LPWSTR)&szBuffer, MAX_PATH, &l);
    if (l > 0)
        TestType = strtoul(szBuffer);

    ConsolePrint(L"[PGDemo] PatchGuard test controller");
    if (TestType > TT_NOTIFY_CALLOUT) {
        ConsolePrint(L"[PGDemo] Invalid test. Use 0, 1, 2 or 3.");
        return ERROR_INVALID_PARAMETER;
    }

    // If the driver is already running, send the next test directly. The
    // driver intentionally has no unload routine, so reloading it is invalid.
    if (CallDriver(TestType, FALSE))
        return ERROR_SUCCESS;

    if (TestType == TT_CR4) {
        if (!IsSmepSupported()) {
            MessageBox(GetDesktopWindow(), TEXT("[PGDemo] SMEP is not supported by this CPU"), NULL, MB_ICONERROR);
            return ERROR_NOT_SUPPORTED;
        }
    }

    Status = NativeAdjustPrivilege(SE_LOAD_DRIVER_PRIVILEGE);
    if (!NT_SUCCESS(Status)) {
        ConsolePrintStatus(L"[PGDemo] NativeAdjustPrivilege failed, NTSTATUS = ", Status);
        return RtlNtStatusToDosError(Status);
    }

    _strcpy(szBuffer, L"\\??\\");
    _strcat(szBuffer, NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath.Buffer);
    _strcat(szBuffer, L"pgdemo.sys");

    RtlInitUnicodeString(&str, L"\\*");
    RtlInitUnicodeString(&drvname, szBuffer);
    InitializeObjectAttributes(&Obja, &str, OBJ_CASE_INSENSITIVE, 0, NULL);

    Status = NtCreateSymbolicLinkObject(&Link, SYMBOLIC_LINK_ALL_ACCESS, &Obja, &drvname);
    if (!NT_SUCCESS(Status)) {
        ConsolePrintStatus(L"[PGDemo] Cannot prepare driver image path, NTSTATUS = ", Status);
    }
    else {

        Status = NativeLoadDriver(L"\\*", PGDEMOREGDRV, NULL, FALSE);
        ConsolePrintStatus(L"[PGDemo] NativeLoadDriver NTSTATUS = ", Status);

        if (Link)
            NtClose(Link);

        if (NT_SUCCESS(Status)) {
            if (!CallDriver(TestType, TRUE))
                ConsolePrint(L"[PGDemo] Driver loaded, but test request failed.");
        }
    }

    return NT_SUCCESS(Status) ? ERROR_SUCCESS : RtlNtStatusToDosError(Status);
}
