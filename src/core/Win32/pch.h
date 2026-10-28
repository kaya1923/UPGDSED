// pch.h
#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

#include "Win32/Types.h"
#include "Win32/Utils.h"
#include "Win32/Cpu.h"
#include "Win32/ApiHook.h"
#include "Win32/Process.h"
#include "Win32/Thread.h"
#include "Win32/File.h"
#include "Win32/Memory.h"
#include "Win32/Registry.h"
#include "Win32/Service.h"
#include "Win32/System.h"
#include "Win32/Window.h"

// Windows-specific includes
#include <winioctl.h>
#include <winternl.h>
#include <psapi.h>
#include <dbghelp.h>
#include <shlwapi.h>
#include <shellapi.h>

// Common structures and constants
#define MAX_PATH_LENGTH 1024

// Function prototypes
bool InitializeWin32Environment();
void CleanupWin32Environment();
