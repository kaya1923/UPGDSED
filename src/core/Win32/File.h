// File.h
#pragma once
#ifndef __FILE_H__
#define __FILE_H__

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for file-related operations
HANDLE CreateFileWithAttributes(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                                LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                                DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
BOOL ReadFileData(HANDLE hFile, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, DWORD* lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
BOOL WriteFileData(HANDLE hFile, LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, DWORD* lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
BOOL GetFileInfo(LPCWSTR lpFileName, PFILE_INFO pFileInfo);
BOOL FileExists(LPCWSTR lpFileName);
BOOL DeleteFileEx(LPCWSTR lpFileName);
BOOL CopyFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);
BOOL MoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
DWORD GetDirectoryListing(LPCWSTR lpDirectoryPath, LPWSTR* ppFileNamesArray, DWORD dwMaxFiles);
BOOL CreateDirectoryEx(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
BOOL RemoveDirectoryEx(LPCWSTR lpPathName);
BOOL IsFileEncrypted(LPCWSTR lpFileName);

#ifdef __cplusplus
}
#endif

#endif // __FILE_H__