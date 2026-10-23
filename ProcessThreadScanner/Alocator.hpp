#pragma once
#include  <windows.h>
#include  "ntTypes.hpp"
#include  <cstdio>
#include  <cstdlib>
#include  "Loader.hpp"

/*
    sizeMissMatch represets the status code of sizeMisMatch 
    success       represents the success status code 
    PAGE          size of a page of memory (64 << 6) (4096) 
*/

constexpr NTSTATUS sizeMissMatch = (NTSTATUS)0xC0000004L;
constexpr NTSTATUS success = (NTSTATUS)0x0L;
constexpr ULONG PAGE = 64 << 6;

class ntRawBuffer
{

    LibraryLoader _ntdllLoader{};
    ULONG _size{};
    ULONG _capacity{};

    unsigned char* _buffer;
    ULONG getSize(SYSTEM_INFORMATION_CLASS enumClass) const;                   // #4
    void force_default();                                                      // #5
public:
    ntRawBuffer(const ntRawBuffer& other) = delete;
    ntRawBuffer& operator =(const ntRawBuffer& other) = delete;                  
    ntRawBuffer(SYSTEM_INFORMATION_CLASS enumClass);                    
    ntRawBuffer(ntRawBuffer&& other) noexcept;                                   // # 2 
    ntRawBuffer& operator =(ntRawBuffer&& other)noexcept;                        // # 2
    ~ntRawBuffer();                                                              // # 1 
    unsigned char* data() { return _buffer; }                                    // # 3
};

/*

        1 #     Constructor , it mades first a call to querySystemInformation to find the size , 
                after that makes the actual call to populate the buffer , inbetween this 2 calls
                if the size is a mismatch then it will grow <<1 "(x2)" the capacity , if the capacity 
                supports that
        
        2 #     Move asignment operator && Move Constructor 
        
        3 #     returns the raw buffer to make operations on bytes
        
        4 #     Makes the  call to the ntdll function to ask the size without populate the buffer
        
        5 #     Forces all elements to default 

*/




inline ULONG ntRawBuffer::getSize(SYSTEM_INFORMATION_CLASS enumClass) const
{
    ULONG size{};
    if (nullptr != _ntdllLoader.querySystemInformation)
    {
        _ntdllLoader.querySystemInformation(enumClass, nullptr, 0, &size);
    }
    return size;
}

inline void ntRawBuffer::force_default() {
    _buffer = nullptr;
    _size = 0;
    _capacity = 0;
}

inline ntRawBuffer::ntRawBuffer(SYSTEM_INFORMATION_CLASS enumClass)
    : _size{ getSize(enumClass) }, _capacity{ _size + PAGE * 2 },
    _buffer{ static_cast<unsigned char*>(::VirtualAlloc(nullptr , _capacity , MEM_COMMIT | MEM_RESERVE , PAGE_READWRITE)) }
{
    if (_buffer != nullptr && _ntdllLoader.querySystemInformation != nullptr)
    {
        NTSTATUS status = _ntdllLoader.querySystemInformation(
            enumClass, _buffer, _capacity, &_size);

            while (status == sizeMissMatch && _capacity<ULONG_MAX /2)
            {

                ::VirtualFree(_buffer,0,MEM_RELEASE);
                
                _capacity <<= 1;
                _buffer = static_cast<unsigned char*>(::VirtualAlloc(nullptr, _capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
                
                if (!_buffer)
                {
                    force_default();
                    return; 
                }
                    status = _ntdllLoader.querySystemInformation(
                        enumClass, _buffer, _capacity, &_size);
                
            }
            if (status != success )
            {
                ::VirtualFree(_buffer, 0, MEM_RELEASE);
                force_default();
            }
        }
    }


inline ntRawBuffer::ntRawBuffer(ntRawBuffer&& other) noexcept :
    _size{ other._size }, _capacity{ other._capacity }, _buffer{ other._buffer }
{
    other.force_default();
}



inline ntRawBuffer& ntRawBuffer::operator=(ntRawBuffer&& other)noexcept
{
    if (this != &other)
    {
        if (_buffer) ::VirtualFree(_buffer, 0, MEM_RELEASE);
        _size = other._size;
        _capacity = other._capacity;
        _ntdllLoader = reinterpret_cast<LibraryLoader&&>(other._ntdllLoader);
        _buffer = other._buffer;
        other.force_default();
        
    }
    return *this;
}

inline ntRawBuffer::~ntRawBuffer()
{
    if (_buffer) ::VirtualFree(_buffer, 0, MEM_RELEASE);
}
    