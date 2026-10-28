// Utils.h
#pragma once
#ifndef __UTILS_H__
#define __UTILS_H__

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for utility functions
BOOL WideCharToAnsi(LPCWSTR lpWideStr, LPSTR lpAnsiStr, DWORD dwAnsiStrSize);
BOOL AnsidCharToWideChar(LPCSTR lpAnsiStr, LPWSTR lpWideStr, DWORD dwWideStrSize);
DWORD GetCurrentTimestamp();
BOOL FormatTimestamp(DWORD dwTimestamp, LPWSTR lpBuffer, DWORD dwBufferSize);
BOOL GenerateGuid(LPGUID lpGuid);
BOOL GuidToString(const GUID* lpGuid, LPWSTR lpBuffer, DWORD dwBufferSize);

#ifdef __cplusplus
}
#endif

#endif // __UTILS_H__