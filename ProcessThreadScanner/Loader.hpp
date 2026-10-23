#pragma once
#include <windows.h>
#include <cstdio>


/*
     it's a function pointer that has the signature of the NtQuerySystemInformation 
     __stdcall (WINAPI)  for windows convention 
*/
extern "C" using ntQuerySystemInformation_delegate = NTSTATUS(__stdcall*)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);



class LibraryLoader             //# 1
{
private:
    HINSTANCE _dllInstance; 

public:
    ntQuerySystemInformation_delegate querySystemInformation;
    LibraryLoader();
    LibraryLoader(const LibraryLoader& other) = delete;
    LibraryLoader& operator =(const LibraryLoader& other) = delete;
    LibraryLoader(LibraryLoader&& other) noexcept;

    LibraryLoader& operator =(LibraryLoader&& other) noexcept;
    ~LibraryLoader();
};




/*
    #1 Wrapper that loads dynamically the ntdll at runtime , and gets the address of the function NtQuerySystemInformation
    Supports move operations 
*/
LibraryLoader::LibraryLoader() :_dllInstance{ ::LoadLibrary(TEXT("ntdll.dll")) }
{
    if (nullptr != _dllInstance)
    {
        querySystemInformation = (ntQuerySystemInformation_delegate)GetProcAddress(_dllInstance, "NtQuerySystemInformation");
        if (nullptr != querySystemInformation) std::printf("You are good to go success \n");
    }
}



inline LibraryLoader::LibraryLoader(LibraryLoader&& other) noexcept :
    _dllInstance{ other._dllInstance }, querySystemInformation{ other.querySystemInformation }
{
    other._dllInstance = nullptr;
    other.querySystemInformation = nullptr;
}



inline LibraryLoader& LibraryLoader::operator=(LibraryLoader&& other) noexcept
{
    if (this != &other)
    {
        if (_dllInstance) ::FreeLibrary(_dllInstance);
        _dllInstance = other._dllInstance;
        querySystemInformation = other.querySystemInformation;
        other._dllInstance = nullptr;
        other.querySystemInformation = nullptr;
    }
    return *this;
}

LibraryLoader::~LibraryLoader()
{
    if (_dllInstance) ::FreeLibrary(_dllInstance);

}
