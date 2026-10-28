// Window.h
#pragma once
#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for Window-related operations
HWND CreateWindowEx(LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle,
                   int x, int y, int nWidth, int nHeight, HWND hWndParent,
                   HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
BOOL ShowWindowEx(HWND hWnd, int nCmdShow);
BOOL UpdateWindowEx(HWND hWnd);
HWND GetWindowByClassName(LPCWSTR lpClassName, HWND hWndParent);
BOOL GetWindowTextEx(HWND hWnd, LPWSTR lpWindowText, int nMaxCount);
BOOL SetWindowTextEx(HWND hWnd, LPCWSTR lpWindowText);
BOOL DestroyWindowEx(HWND hWnd);
BOOL GetWindowRectEx(HWND hWnd, LPRECT lpRect);
BOOL SetWindowRectEx(HWND hWnd, LPCRECT lpRect);

#ifdef __cplusplus
}
#endif

#endif // __WINDOW_H__