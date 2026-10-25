
#pragma once
#include "Specialization.hpp"
#include "StringBuilder.hpp"
#include "ntTypes.hpp"
using process_structure = SYSTEM_PROCESS_INFORMATION;
/*
   still in tests 
   it will be modified in one text_parser, 
*/

class Scanner
{

    dll_vector<process_structure> _bufferProcesses{};
    const wchar_t* stateEnumParser(ULONG stateEnumValue);
    string_builder builder{};

public:
     Scanner() = default;
    ~Scanner() = default;
    const wchar_t* dump() { return builder.flush(); };
    Scanner& basic_info();
    Scanner& find_basic(bool (*predicate)(BasicProcessInfo info));
};

const wchar_t* Scanner::stateEnumParser(ULONG stateEnumValue)
{
    switch (stateEnumValue)
    {
     case 0:  return L"Initialized";
     case 1:  return L"Ready";
     case 2:  return L"Running";
     case 3:  return L"Standby";
     case 4:  return L"Terminated";
     case 5:  return L"Wait";
     case 6:  return L"Transition";
     case 7:  return L"DeferredReady";
     case 8:  return L"GateWaitObsolete";
     case 9:  return L"WaitingForProcessInSwap";
    default:  return L"Unknown";
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

        builder.append(true,L"=====================================================================================================================");
        builder.append(true,L"\n|PID :%-10lu |PN : %-38ws|THN : %-5lu |PPC : %-10zuKb |WS: %-10zuKb |HC : %-5lu\n", info.pid, info.procName, info.thn, info.ppc, info.ws, info.hc);
        SYSTEM_THREAD_INFORMATION* thread_array = reinterpret_cast<SYSTEM_THREAD_INFORMATION*>(proc->Threads);
        builder.append(true,L"=====================================================================================================================");
        builder.append(true,L" ");

        for (unsigned i{ 0 };i < info.thn;++i)
        {
            unsigned long long userTime    = thread_array[i].UserTime  .QuadPart / 10'000;
            unsigned long long kernelTime  = thread_array[i].KernelTime.QuadPart / 10'000;
            unsigned long long threadId    = (LONGLONG)thread_array[i] .ClientId.UniqueThread;
            const  wchar_t*       threadState = stateEnumParser(thread_array[i].ThreadState);

            builder.append(true,L"\t ---> |TNO : %-10llu |TID : %10llu |UTIME : %10llums |KTIME : %10llums |THSTATE: %10s ",
                i + 1, threadId, userTime, kernelTime, threadState);

        }
    }
    return *this;
}

Scanner& Scanner::find_basic(bool(*predicate)(BasicProcessInfo info))
{
    

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
            builder.append(true,L"\n");
            builder.append(true,L"==========================================================================================================================");
            builder.append(true,L"|PID :%-10lu |PN : %-38ws|THN : %-5lu |PPC : %-10zuKb |WS: %-10zuKb |HC : %-5lu",
                info.pid, info.procName, info.thn, info.ppc, info.ws, info.hc);
            builder.append(true,L"==========================================================================================================================");
            std::printf("\n");
            SYSTEM_THREAD_INFORMATION* thread_array = reinterpret_cast<SYSTEM_THREAD_INFORMATION*>(proc->Threads);
            for (unsigned i {} ; i < proc->NumberOfThreads ;++i)
            {
                unsigned long long userTime    = thread_array[i].UserTime.QuadPart   / 10'000;
                unsigned long long kernelTime  = thread_array[i].KernelTime.QuadPart / 10'000;
                unsigned long long threadId    = (LONGLONG)thread_array[i].ClientId.UniqueThread;
                const wchar_t*        threadState = stateEnumParser(thread_array[i].ThreadState);

                builder.append(true,L"\t ---> |TNO : %-10llu |TID : %10llu |UTIME : %10llu |KTIME : %10llu |THSTATE: %10s \n",
                    i + 1, threadId, userTime, kernelTime, threadState);

               
            }
        }
    }
    return *this;
}
