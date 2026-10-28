// Types.h
#pragma once
#ifndef __TYPES_H__
#define __TYPES_H__

#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

// Common structures and types used across the Win32 API wrapper
typedef struct _WIN32_API_HOOK {
    HMODULE hModule;
    LPCSTR lpProcName;
    FARPROC lpOriginal;
    FARPROC lpHook;
    bool bHooked;
} WIN32_API_HOOK, *PWIN32_API_HOOK;

// Process information structure
typedef struct _PROCESS_INFO {
    DWORD dwProcessId;
    HANDLE hProcess;
    LPWSTR lpProcessName;
    DWORD dwParentProcessId;
    BOOL bIs64Bit;
    BOOL bHasDebugPrivilege;
} PROCESS_INFO, *PPROCESS_INFO;

// Thread information structure
typedef struct _THREAD_INFO {
    DWORD dwThreadId;
    HANDLE hThread;
    DWORD dwProcessId;
    LPVOID lpStartAddress;
    LPVOID lpThreadLocalBase;
    LPVOID lpWin32StartAddress;
} THREAD_INFO, *PTHREAD_INFO;

// Memory region structure
typedef struct _MEMORY_REGION {
    LPVOID lpBaseAddress;
    SIZE_T dwRegionSize;
    DWORD dwState;
    DWORD dwProtect;
    DWORD dwType;
} MEMORY_REGION, *PMEMORY_REGION;

// File information structure
typedef struct _FILE_INFO {
    HANDLE hFile;
    LPWSTR lpFileName;
    DWORD dwFileSizeLow;
    DWORD dwFileSizeHigh;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
} FILE_INFO, *PFILE_INFO;

// Registry key structure
typedef struct _REGISTRY_KEY {
    HKEY hKey;
    LPWSTR lpKeyName;
    DWORD dwKeyType;
} REGISTRY_KEY, *PREGISTRY_KEY;

// Window information structure
typedef struct _WINDOW_INFO {
    HWND hWnd;
    DWORD dwProcessId;
    DWORD dwThreadId;
    LPWSTR lpWindowName;
    RECT rcWindow;
    BOOL bIsVisible;
} WINDOW_INFO, *PWINDOW_INFO;

// System information structure
typedef struct _SYSTEM_INFO {
    DWORD dwProcessorCount;
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *PSYSTEM_INFO;

// Error information structure
typedef struct _ERROR_INFO {
    DWORD dwErrorCode;
    LPWSTR lpErrorMessage;
    BOOL bHasError;
} ERROR_INFO, *PERROR_INFO;

// API hook callback function type
typedef BOOL (WINAPI *HOOK_CALLBACK)(LPVOID lpData);

// Constants for various Win32 API functions

// Protection constant names mapping to protect value
#define WIN32_PAGE_NOACCESS         0x01
#define WIN32_PAGE_READONLY         0x02
#define WIN32_PAGE_READWRITE        0x04
#define WIN32_PAGE_WRITECOPY        0x08
#define WIN32_PAGE_EXECUTE          0x10
#define WIN32_PAGE_EXECUTE_READ     0x20
#define WIN32_PAGE_EXECUTE_READWRITE 0x40
#define WIN32_PAGE_EXECUTE_WRITECOPY 0x80
#define WIN32_PAGE_GUARD           0x100
#define WIN32_PAGE_NOCACHE         0x200
#define WIN32_PAGE_WRITECOMBINE    0x400

// State constant names mapping to memory state values
#define WIN32_MEM_COMMIT            0x1000
#define WIN32_MEM_RESERVE           0x2000
#define WIN32_MEM_FREE              0x10000

// Type constant names mapping to memory type values
#define WIN32_MEM_PRIVATE           0x20000
#define WIN32_MEM_MAPPED            0x40000
#define WIN32_MEM_IMAGE             0x1000000

#endif // __TYPES_H__