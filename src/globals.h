#pragma once

#include <windows.h>
#include "patterns.h"

// Function declarations
void DisplayHelp(void);
BOOL ProcessCurrentDirectory(void);
BOOL ProcessFile(const char* szFileName, PCHAR pPatchPattern, DWORD dwPatternSize);
BOOL InitializeScanner(void);
void UninitializeScanner(void);
PVOID DetectImgpValidateImageHashPattern();
int GetWindowsVersion();

// Global variables
extern BOOLEAN g_bVerbose;
extern BOOLEAN g_bScanOnly;
extern BOOLEAN g_bQuiet;

// Windows version constants
#define WIN10_20H2 19041
#define WIN10_21H1 19043
#define WIN10_21H2 19044
#define WIN10_22H2 19045
#define WIN11_22H2 22621
#define WIN11_25H2 26200