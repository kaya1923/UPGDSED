// Thread.c
#include "Thread.h"

#pragma warning(disable: 4100) // Unreferenced parameter

// Function to create a thread with given parameters
HANDLE CreateThreadEx(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize,
                     LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
                     DWORD dwCreationFlags, LPDWORD lpThreadId) {
    return CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

// Function to get thread information by ID
BOOL GetThreadInfoById(DWORD dwThreadId, PTHREAD_INFO pThreadInfo) {
    HANDLE hThread;

    // Open the thread with full access rights
    hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dwThreadId);
    if (hThread == NULL)
        return FALSE;

    // Get thread information using GetThreadContext API
    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_FULL;
    
    BOOL bResult = GetThreadContext(hThread, &ctx);

    pThreadInfo->dwThreadId = dwThreadId;
    pThreadInfo->hThread = hThread;
    pThreadInfo->dwProcessId = GetProcessIdOfThread(hThread);
    pThreadInfo->lpStartAddress = ctx.Eip;  // This may not be entirely correct depending on platform
    pThreadInfo->lpThreadLocalBase = (LPVOID)ctx.Ebx;
    pThreadInfo->lpWin32StartAddress = ctx.Eip;

    CloseHandle(hThread);
    
    return bResult;
}

// Function to suspend a thread
BOOL SuspendThreadEx(HANDLE hThread) {
    return (SuspendThread(hThread) != (DWORD)-1);
}

// Function to resume a thread
BOOL ResumeThreadEx(HANDLE hThread) {
    return (ResumeThread(hThread) != (DWORD)-1);
}

// Function to terminate a thread
BOOL TerminateThreadEx(HANDLE hThread, DWORD dwExitCode) {
    return TerminateThread(hThread, dwExitCode);
}

// Function to get all running threads in the system
DWORD GetRunningThreads(PTHREAD_INFO* ppThreadInfoArray) {
    HANDLE hSnapshot;
    THREADENTRY32 te32;
    DWORD dwIndex = 0;

    // Take a snapshot of all threads in the system
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    // Set the size of the structure before using it
    te32.dwSize = sizeof(THREADENTRY32);

    // Retrieve information about the first thread and exit if unsuccessful
    if (!Thread32First(hSnapshot, &te32)) {
        CloseHandle(hSnapshot);
        return 0;
    }

    // Allocate memory for the array of threads
    *ppThreadInfoArray = (PTHREAD_INFO)malloc(sizeof(THREAD_INFO) * 1024);
    if (*ppThreadInfoArray == NULL) {
        CloseHandle(hSnapshot);
        return 0;
    }

    // Walk the snapshot of threads
    do {
        (*ppThreadInfoArray)[dwIndex].dwThreadId = te32.th32ThreadID;
        (*ppThreadInfoArray)[dwIndex].dwProcessId = te32.th32OwnerProcessID;
        (*ppThreadInfoArray)[dwIndex].hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
        (*ppThreadInfoArray)[dwIndex].lpStartAddress = (LPVOID)te32.lpBasePrio;
        
        dwIndex++;
    } while (Thread32Next(hSnapshot, &te32) && dwIndex < 1024);

    CloseHandle(hSnapshot);
    return dwIndex;
}
