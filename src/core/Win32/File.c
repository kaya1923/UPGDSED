// File.c
#include "File.h"
#include <shlwapi.h>

#pragma warning(disable: 4100) // Unreferenced parameter

// Function to create a file with specific attributes
HANDLE CreateFileWithAttributes(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                                LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                                DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                       dwFlagsAndAttributes, hTemplateFile);
}

// Function to read data from a file
BOOL ReadFileData(HANDLE hFile, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, DWORD* lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
    return ReadFile(hFile, lpBuffer, dwNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

// Function to write data to a file
BOOL WriteFileData(HANDLE hFile, LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, DWORD* lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
    return WriteFile(hFile, lpBuffer, dwNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

// Function to get file information
BOOL GetFileInfo(LPCWSTR lpFileName, PFILE_INFO pFileInfo) {
    WIN32_FIND_DATAW fd;
    HANDLE hFind;

    // Find the first file in the directory to get its metadata
    hFind = FindFirstFileExW(lpFileName, FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);
    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    pFileInfo->hFile = INVALID_HANDLE_VALUE;
    pFileInfo->lpFileName = _wcsdup(fd.cFileName);
    pFileInfo->dwFileSizeLow = fd.nFileSizeLow;
    pFileInfo->dwFileSizeHigh = fd.nFileSizeHigh;
    pFileInfo->ftCreationTime = fd.ftCreationTime;
    pFileInfo->ftLastAccessTime = fd.ftLastAccessTime;
    pFileInfo->ftLastWriteTime = fd.ftLastWriteTime;

    FindClose(hFind);
    return TRUE;
}

// Function to check if a file exists
BOOL FileExists(LPCWSTR lpFileName) {
    DWORD dwAttrib = GetFileAttributesW(lpFileName);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// Function to delete a file
BOOL DeleteFileEx(LPCWSTR lpFileName) {
    return DeleteFileW(lpFileName);
}

// Function to copy a file
BOOL CopyFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists) {
    return CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
}

// Function to move (rename) a file
BOOL MoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName) {
    return MoveFileExW(lpExistingFileName, lpNewFileName, MOVEFILE_REPLACE_EXISTING);
}

// Function to get directory listing
DWORD GetDirectoryListing(LPCWSTR lpDirectoryPath, LPWSTR* ppFileNamesArray, DWORD dwMaxFiles) {
    WIN32_FIND_DATAW fd;
    HANDLE hFind;
    DWORD dwIndex = 0;

    // Add wildcard to the path to find all files 
    LPCWSTR lpSearchPath = L"*";
    if (lpDirectoryPath != NULL && wcslen(lpDirectoryPath) > 0)
        wcscat_s((LPWSTR)lpDirectoryPath, MAX_PATH_LENGTH, lpSearchPath);

    hFind = FindFirstFileExW(lpDirectoryPath, FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        // Add file name to the array
        ppFileNamesArray[dwIndex] = _wcsdup(fd.cFileName);
        dwIndex++;
    } while (FindNextFileW(hFind, &fd) && dwIndex < dwMaxFiles);

    FindClose(hFind);
    return dwIndex;
}

// Function to create a directory
BOOL CreateDirectoryEx(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    return CreateDirectoryW(lpPathName, lpSecurityAttributes);
}

// Function to remove a directory
BOOL RemoveDirectoryEx(LPCWSTR lpPathName) {
    return RemoveDirectoryW(lpPathName);
}

// Function to check if a file is encrypted
BOOL IsFileEncrypted(LPCWSTR lpFileName) {
    DWORD dwAttrib = GetFileAttributesW(lpFileName);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_ENCRYPTED));
}
