// Utils.c
#include "Utils.h"

#pragma warning(disable: 4100) // Unreferenced parameter

// Function to convert a wide character string to an ANSI string
BOOL WideCharToAnsi(LPCWSTR lpWideStr, LPSTR lpAnsiStr, DWORD dwAnsiStrSize) {
    int iResult = WideCharToMultiByte(CP_ACP, 0, lpWideStr, -1, lpAnsiStr, dwAnsiStrSize, NULL, NULL);
    return (iResult > 0);
}

// Function to convert an ANSI string to a wide character string
BOOL AnsiToWideChar(LPCSTR lpAnsiStr, LPWSTR lpWideStr, DWORD dwWideStrSize) {
    int iResult = MultiByteToWideChar(CP_ACP, 0, lpAnsiStr, -1, lpWideStr, dwWideStrSize);
    return (iResult > 0);
}

// Function to get current timestamp
DWORD GetCurrentTimestamp() {
    SYSTEMTIME st;
    GetSystemTime(&st);
    
    // Convert system time to a timestamp (in seconds since epoch)
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    
    LARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    // Convert to Unix epoch time (seconds)
    return (DWORD)(li.QuadPart / 10000000 - 11644473600LL);
}

// Function to format a timestamp into a readable string
BOOL FormatTimestamp(DWORD dwTimestamp, LPWSTR lpBuffer, DWORD dwBufferSize) {
    time_t t = (time_t)dwTimestamp;
    struct tm* pTm;

    pTm = localtime(&t);
    if (pTm == NULL)
        return FALSE;

    // Format into a readable string (e.g., "2023-10-10 14:30:45")
    return (swprintf_s(lpBuffer, dwBufferSize, L"%04d-%02d-%02d %02d:%02d:%02d",
                      pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
                      pTm->tm_hour, pTm->tm_min, pTm->tm_sec) != -1);
}

// Function to generate a random UUID/GUID
BOOL GenerateGuid(LPGUID lpGuid) {
    return (CoCreateGuid(lpGuid) == S_OK);
}

// Function to convert a GUID to string representation
BOOL GuidToString(const GUID* lpGuid, LPWSTR lpBuffer, DWORD dwBufferSize) {
    return (StringFromGUID2(*lpGuid, lpBuffer, dwBufferSize) > 0);
}
