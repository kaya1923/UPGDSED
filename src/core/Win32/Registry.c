// Registry.c
#include "Registry.h"

#pragma warning(disable: 4100) // Unreferenced parameter

// Function to create a registry key
HKEY CreateRegistryKey(LPCWSTR lpKeyName, DWORD dwDesiredAccess) {
    HKEY hKey;

    if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, lpKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, dwDesiredAccess, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return NULL;
    }

    return hKey;
}

// Function to open a registry key
HKEY OpenRegistryKey(LPCWSTR lpKeyName, DWORD dwDesiredAccess) {
    HKEY hKey;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, lpKeyName, 0, dwDesiredAccess, &hKey) != ERROR_SUCCESS) {
        return NULL;
    }

    return hKey;
}

// Function to close a registry key
BOOL CloseRegistryKey(HKEY hKey) {
    return (RegCloseKey(hKey) == ERROR_SUCCESS);
}

// Function to delete a registry key
BOOL DeleteRegistryKey(LPCWSTR lpKeyName) {
    return (RegDeleteKeyExW(HKEY_LOCAL_MACHINE, lpKeyName, 0, 0) == ERROR_SUCCESS);
}

// Function to set a registry value
BOOL SetRegistryValue(HKEY hKey, LPCWSTR lpValueName, DWORD dwValueType, LPCVOID lpData, DWORD dwDataSize) {
    return (RegSetValueExW(hKey, lpValueName, 0, dwValueType, (LPBYTE)lpData, dwDataSize) == ERROR_SUCCESS);
}

// Function to get a registry value
BOOL GetRegistryValue(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpValueType, LPVOID lpData, LPDWORD lpDataSize) {
    return (RegQueryValueExW(hKey, lpValueName, 0, lpValueType, (LPBYTE)lpData, lpDataSize) == ERROR_SUCCESS);
}

// Function to delete a registry value
BOOL DeleteRegistryValue(HKEY hKey, LPCWSTR lpValueName) {
    return (RegDeleteValueW(hKey, lpValueName) == ERROR_SUCCESS);
}

// Function to enumerate registry subkeys
DWORD EnumerateRegistrySubkeys(HKEY hKey, LPWSTR* ppSubkeyNamesArray, DWORD dwMaxSubkeys) {
    DWORD dwIndex = 0;
    DWORD dwSubkeyCount;
    DWORD dwMaxSubkeyNameLength;
    LSTATUS lsStatus;

    // Query the number and maximum length of the subkeys
    lsStatus = RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &dwSubkeyCount, &dwMaxSubkeyNameLength, NULL, NULL, NULL, NULL, NULL, NULL);
    if (lsStatus != ERROR_SUCCESS) {
        return 0;
    }

    // Allocate memory for the array of subkeys names
    ppSubkeyNamesArray = (LPWSTR*)malloc(sizeof(LPWSTR) * dwMaxSubkeys);
    if (ppSubkeyNamesArray == NULL)
        return 0;

    // Enumerate the subkeys
    for (DWORD i = 0; (i < dwSubkeyCount) && (dwIndex < dwMaxSubkeys); i++) {
        DWORD dwNameSize = dwMaxSubkeyNameLength + 1;  // Include terminating null character
        wchar_t* lpName = (wchar_t*)malloc(dwNameSize * sizeof(wchar_t));

        if (lpName == NULL)
            continue;

        lsStatus = RegEnumKeyExW(hKey, i, lpName, &dwNameSize, NULL, NULL, NULL, NULL);
        if (lsStatus == ERROR_SUCCESS) {
            ppSubkeyNamesArray[dwIndex] = _wcsdup(lpName);
            dwIndex++;
        }
        free(lpName);  // Free allocated buffer
    }

    return dwIndex;
}

// Function to enumerate registry values
DWORD EnumerateRegistryValues(HKEY hKey, LPWSTR* ppValueNamesArray, DWORD dwMaxValues) {
    DWORD dwIndex = 0;
    DWORD dwValueCount;
    DWORD dwMaxValueNameLength;
    LSTATUS lsStatus;

    // Query the number and maximum length of the values
    lsStatus = RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwValueCount, &dwMaxValueNameLength, NULL, NULL, NULL);
    if (lsStatus != ERROR_SUCCESS) {
        return 0;
    }

    // Allocate memory for the array of value names
    ppValueNamesArray = (LPWSTR*)malloc(sizeof(LPWSTR) * dwMaxValues);
    if (ppValueNamesArray == NULL)
        return 0;

    // Enumerate the values
    for (DWORD i = 0; (i < dwValueCount) && (dwIndex < dwMaxValues); i++) {
        DWORD dwNameSize = dwMaxValueNameLength + 1;  // Include terminating null character
        wchar_t* lpName = (wchar_t*)malloc(dwNameSize * sizeof(wchar_t));

        if (lpName == NULL)
            continue;

        lsStatus = RegEnumValueW(hKey, i, lpName, &dwNameSize, NULL, NULL, NULL, NULL);
        if (lsStatus == ERROR_SUCCESS) {
            ppValueNamesArray[dwIndex] = _wcsdup(lpName);
            dwIndex++;
        }
        free(lpName);  // Free allocated buffer
    }

    return dwIndex;
}
