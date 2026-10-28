// Cpu.h
#pragma once
#ifndef __CPU_H__
#define __CPU_H__

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for CPU-related operations
BOOL GetCpuInfo(PCPU_INFO pCpuInfo);
BOOL IsProcessorFeaturePresentEx(DWORD dwFeature);
BOOL GetProcessorFrequency(PDWORD pdwFrequency);
BOOL GetCpuUsage(PFLOAT pfUsage);

#ifdef __cplusplus
}
#endif

#endif // __CPU_H__