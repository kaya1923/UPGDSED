// Registry.h
#pragma once
#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for registry-related operations
HKEY CreateRegistryKey(LPCWSTR lpKeyName, DWORD dwDesiredAccess);
HKEY OpenRegistryKey(LPCWSTR lpKeyName, DWORD dwDesiredAccess);
BOOL CloseRegistryKey(HKEY hKey);
BOOL DeleteRegistryKey(LPCWSTR lpKeyName);
BOOL SetRegistryValue(HKEY hKey, LPCWSTR lpValueName, DWORD dwValueType, LPCVOID lpData, DWORD dwDataSize);
BOOL GetRegistryValue(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpValueType, LPVOID lpData, LPDWORD lpDataSize);
BOOL DeleteRegistryValue(HKEY hKey, LPCWSTR lpValueName);
DWORD EnumerateRegistrySubkeys(HKEY hKey, LPWSTR* ppSubkeyNamesArray, DWORD dwMaxSubkeys);
DWORD EnumerateRegistryValues(HKEY hKey, LPWSTR* ppValueNamesArray, DWORD dwMaxValues);

#ifdef __cplusplus
}
#endif

#endif // __REGISTRY_H__