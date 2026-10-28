// Window.c
#include "Window.h"

#pragma warning(disable: 4100) // Unreferenced parameter

// Function to create a window 
HWND CreateWindowEx(LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle,
                   int x, int y, int nWidth, int nHeight, HWND hWndParent,
                   HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    return CreateWindowExW(0, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight,
                          hWndParent, hMenu, hInstance, lpParam);
}

// Function to show a window
BOOL ShowWindowEx(HWND hWnd, int nCmdShow) {
    return ShowWindow(hWnd, nCmd);
}

// Function to update the window 
BOOL UpdateWindowEx(HWND hWnd) {
    return UpdateWindow(hWnd);
}

// Function to get a handle to a window by its class name
HWND GetWindowByClassName(LPCWSTR lpClassName, HWND hWndParent) {
    return FindWindowW(lpClassName, NULL);
}

// Function to get the window text
BOOL GetWindowTextEx(HWND hWnd, LPWSTR lpWindowText, int nMaxCount) {
    return GetWindowTextW(hWnd, lpWindowText, nMaxCount);
}

// Function to set the window text
BOOL SetWindowTextEx(HWND hWnd, LPCWSTR lpWindowText) {
    return SetWindowTextW(hWnd, lpWindowText);
}

// Function to destroy a window
BOOL DestroyWindowEx(HWND hWnd) {
    return DestroyWindow(hWnd);
}

// Function to get the window rectangle 
BOOL GetWindowRectEx(HWND hWnd, LPRECT lpRect) {
    return GetWindowRect(hWnd, lpRect);
}

// Function to set the window rectangle
BOOL SetWindowRectEx(HWND hWnd, LPCRECT lpRect) {
    return SetWindowPos(hWnd, NULL, lpRect->left, lpRect->top,
                       lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
                       SWP_NOZORDER | SWP_NOACTIVATE);
}
