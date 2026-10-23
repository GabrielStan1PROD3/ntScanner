
#pragma once
#include "Alocator.hpp"

using process_structure = SYSTEM_PROCESS_INFORMATION;

template<typename T>
class dll_vector;


template<>
class dll_vector<SYSTEM_PROCESS_INFORMATION>
{

private:


    ntRawBuffer    _rawBuffer{ SystemProcessInformation };


public:
    process_structure* first() noexcept { return reinterpret_cast<process_structure*>(_rawBuffer.data()); }
    static process_structure* next(process_structure* current) noexcept;

};
inline process_structure* dll_vector<SYSTEM_PROCESS_INFORMATION>::next(process_structure* current) noexcept
{
    if (nullptr == current || current->NextEntryOffset == 0) return nullptr;

    return reinterpret_cast<process_structure*>(
        reinterpret_cast<unsigned char*>(current) + current->NextEntryOffset
        );
}
