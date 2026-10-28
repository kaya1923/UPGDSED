// Thread.h
#pragma once
#ifndef __THREAD_H__
#define __THREAD_H__

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for thread-related operations
HANDLE CreateThreadEx(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize,
                     LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
                     DWORD dwCreationFlags, LPDWORD lpThreadId);
BOOL GetThreadInfoById(DWORD dwThreadId, PTHREAD_INFO pThreadInfo);
BOOL SuspendThreadEx(HANDLE hThread);
BOOL ResumeThreadEx(HANDLE hThread);
BOOL TerminateThreadEx(HANDLE hThread, DWORD dwExitCode);
DWORD GetRunningThreads(PTHREAD_INFO* ppThreadInfoArray);

#ifdef __cplusplus
}
#endif

#endif // __THREAD_H__