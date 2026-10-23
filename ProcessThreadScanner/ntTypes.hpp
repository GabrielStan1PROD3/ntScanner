#pragma once
#pragma once
#include <windows.h>

using NTSTATUS = LONG;
using KPRIORITY = LONG;




struct BasicProcessInfo
{
    const wchar_t* procName;
    SIZE_T    ppc;
    SIZE_T     ws;
    ULONG     pid;
    ULONG     thn;
    ULONG      hc;
    
};


enum SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation = 0,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemModuleInformation = 11,
    SystemHandleInformation = 16
};

struct UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
};

struct CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
};

struct SYSTEM_THREAD_INFORMATION
{
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG         WaitTime;
    PVOID         StartAddress;
    CLIENT_ID     ClientId;
    KPRIORITY     Priority;
    LONG          BasePriority;
    ULONG         ContextSwitches;
    ULONG         ThreadState;
    ULONG         WaitReason;
};

struct SYSTEM_PROCESS_INFORMATION
{
    ULONG                     NextEntryOffset;
    ULONG                     NumberOfThreads;
    LARGE_INTEGER             WorkingSetPrivateSize;
    ULONG                     HardFaultCount;
    ULONG                     NumberOfThreadsHighWatermark;
    ULONGLONG                 CycleTime;
    LARGE_INTEGER             CreateTime;
    LARGE_INTEGER             UserTime;
    LARGE_INTEGER             KernelTime;
    UNICODE_STRING            ImageName;
    KPRIORITY                 BasePriority;
    HANDLE                    UniqueProcessId;
    HANDLE                    InheritedFromUniqueProcessId;
    ULONG                     HandleCount;
    ULONG                     SessionId;
    ULONG_PTR                 UniqueProcessKey;
    SIZE_T                    PeakVirtualSize;
    SIZE_T                    VirtualSize;
    ULONG                     PageFaultCount;
    SIZE_T                    PeakWorkingSetSize;
    SIZE_T                    WorkingSetSize;
    SIZE_T                    QuotaPeakPagedPoolUsage;
    SIZE_T                    QuotaPagedPoolUsage;
    SIZE_T                    QuotaPeakNonPagedPoolUsage;
    SIZE_T                    QuotaNonPagedPoolUsage;
    SIZE_T                    PagefileUsage;
    SIZE_T                    PeakPagefileUsage;
    SIZE_T                    PrivatePageCount;
    LARGE_INTEGER             ReadOperationCount;
    LARGE_INTEGER             WriteOperationCount;
    LARGE_INTEGER             OtherOperationCount;
    LARGE_INTEGER             ReadTransferCount;
    LARGE_INTEGER             WriteTransferCount;
    LARGE_INTEGER             OtherTransferCount;
    SYSTEM_THREAD_INFORMATION Threads[1];
};

