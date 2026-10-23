
#pragma once
#include "Specialization.hpp"
#include "StringBuilder.hpp"
#include "ntTypes.hpp"
using process_structure = SYSTEM_PROCESS_INFORMATION;
/*
   still in tests 
*/
class Scanner
{

    dll_vector<process_structure> _bufferProcesses{};
    const char* stateEnumParser(ULONG stateEnumValue);

public:
     Scanner() = default;
    ~Scanner() = default;

    Scanner& basic_info();
    Scanner& find_basic(bool (*predicate)(BasicProcessInfo info));
};

const char* Scanner::stateEnumParser(ULONG stateEnumValue)
{
    switch (stateEnumValue)
    {
     case 0:  return "Initialized";
     case 1:  return "Ready";
     case 2:  return "Running";
     case 3:  return "Standby";
     case 4:  return "Terminated";
     case 5:  return "Wait";
     case 6:  return "Transition";
     case 7:  return "DeferredReady";
     case 8:  return "GateWaitObsolete";
     case 9:  return "WaitingForProcessInSwap";
    default:  return "Unknown";
    };
}

Scanner& Scanner::basic_info()
{
    BasicProcessInfo info;

    for (auto proc = _bufferProcesses.first(); proc != nullptr; proc = ::dll_vector<process_structure>::next(proc))
    {
        info.procName = proc->ImageName.Buffer ? proc->ImageName.Buffer : L"System Idle Process";
        info.ppc      = proc->PrivatePageCount >> 10; // size in kb
        info.ws       = proc->WorkingSetSize   >> 10;
        info.pid      = static_cast<ULONG>(reinterpret_cast<ULONG_PTR>(proc->UniqueProcessId));
        info.thn      = proc->NumberOfThreads;
        info.hc       = proc->HandleCount;

        std::printf("=====================================================================================================================\n");
        std::printf("\n|PID :%-10lu |PN : %-38ws|THN : %-5lu |PPC : %-10zuKb |WS: %-10zuKb |HC : %-5lu\n", info.pid, info.procName, info.thn, info.ppc, info.ws, info.hc);
        SYSTEM_THREAD_INFORMATION* thread_array = reinterpret_cast<SYSTEM_THREAD_INFORMATION*>(proc->Threads);
        std::printf("=====================================================================================================================\n");
        std::printf("\n");

        for (int i{ 0 };i < info.thn;++i)
        {
            unsigned long long userTime    = thread_array[i].UserTime  .QuadPart / 10'000;
            unsigned long long kernelTime  = thread_array[i].KernelTime.QuadPart / 10'000;
            unsigned long long threadId    = (LONGLONG)thread_array[i] .ClientId.UniqueThread;
            const  char*       threadState = stateEnumParser(thread_array[i].ThreadState);

            std::printf("\t ---> |TNO : %-10llu |TID : %10llu |UTIME : %10llu |KTIME : %10llu |THSTATE: %10s \n",
                i + 1, threadId, userTime, kernelTime, threadState);

        }
    }
    return *this;
}

Scanner& Scanner::find_basic(bool(*predicate)(BasicProcessInfo info))
{
    std::printf("\n\n Start Custom Scanning ...\n\n");

    BasicProcessInfo info;
    for (auto proc = _bufferProcesses.first(); proc != nullptr;proc = ::dll_vector<process_structure>::next(proc))
    {

        info.pid = static_cast<ULONG>(reinterpret_cast<ULONG_PTR>(proc->UniqueProcessId));
        info.procName = proc->ImageName.Buffer ? proc->ImageName.Buffer : L"System Idle Process";
        info.ppc      = proc->PrivatePageCount  >> 10; // size in kb
        info.ws       = proc->WorkingSetSize    >> 10;     //size in kb
        info.thn      = proc->NumberOfThreads;
        info.hc       = proc->HandleCount;

        if (predicate(info))
        {
            std::printf("\n");
            std::printf("==========================================================================================================================\n");
            std::printf("|PID :%-10lu |PN : %-38ws|THN : %-5lu |PPC : %-10zuKb |WS: %-10zuKb |HC : %-5lu\n",
                info.pid, info.procName, info.thn, info.ppc, info.ws, info.hc);
            std::printf("==========================================================================================================================\n");
            std::printf("\n");
            SYSTEM_THREAD_INFORMATION* thread_array = reinterpret_cast<SYSTEM_THREAD_INFORMATION*>(proc->Threads);
            for (int i {} ; i < proc->NumberOfThreads ;++i)
            {
                unsigned long long userTime    = thread_array[i].UserTime.QuadPart   / 10'000;
                unsigned long long kernelTime  = thread_array[i].KernelTime.QuadPart / 10'000;
                unsigned long long threadId    = (LONGLONG)thread_array[i].ClientId.UniqueThread;
                const char*        threadState = stateEnumParser(thread_array[i].ThreadState);

                std::printf("\t ---> |TNO : %-10llu |TID : %10llu |UTIME : %10llu |KTIME : %10llu |THSTATE: %10s \n",
                    i + 1, threadId, userTime, kernelTime, threadState);

               
            }
        }
    }
    return *this;
}
