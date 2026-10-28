/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2017 - 2018
*
*  TITLE:       PATTERNS.C
*
*  VERSION:     1.22
*
*  DATE:        23 Sept 2026
*
*  Pattern routines for the program.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/
#include "global.h"
#include "patterns.h"

//
// Patterns for ImgpValidateImageHash function
//
PATTERN_ENTRY g_ImgpValidateImageHashPatterns[] = {
    // Windows 10 RS2+ patterns would go here (simplified)
    { { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48 }, // Pattern
      { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0 }, // Mask
      16, // Length
      0   // Offset
    }
};

ULONG g_ImgpValidateImageHashPatternsCount = ARRAYSIZE(g_ImgpValidateImageHashPatterns);