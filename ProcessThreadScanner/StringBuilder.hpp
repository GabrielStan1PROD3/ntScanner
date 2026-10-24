#pragma once
#pragma once
#include "windows.h"
#include <cstdio>

/*
   compile time fixed size default is 1024 bytes
*/

constexpr size_t initial_capacity = 64 << 4;
constexpr size_t initial_heap     = initial_capacity << 2;
class string_builder
{
    size_t _cursor       { 0 }; //# 5
    size_t _heapSize     { 0 };
    size_t _heapCapacity { 0 };
    size_t _stackCapacity{ initial_capacity };
    
    wchar_t _buffer [initial_capacity];
    wchar_t* _heapBuffer { nullptr };
   
    bool isHeapAllocated { false };

    bool safe_append (const wchar_t* string, size_t stringSize, bool appendLine = false) noexcept;      //# 1
    bool reset_cursor() noexcept;                                                                           //# 2
   
    bool move_cursor (size_t index) noexcept;  //# 3
    bool move_heap   (size_t size,const wchar_t* string, bool newLine = false);    
public:
    string_builder() = default;
   ~string_builder();                                                     
    
    const wchar_t* toString() noexcept;  //# 4
    const wchar_t* flush   () noexcept;

    void append      (const wchar_t* string, size_t size);
    void append_line (const wchar_t* string, size_t size);

};

/*
      
     # 5 _cursor represents the actual size of the _buffer 

     # 1 ensures that the string is apended corectly ,removes the null pointer and adds it to the buffer 
     # 2 puts  the nullterminator to the begging of the buffer 
     # 3 moves the cursor to an index size and puts there the null-terminator 
     # 4 converts the buffer into an string  (adds the null terminator to the end)
*/



inline bool string_builder::safe_append(const wchar_t* string,
        size_t stringSize, bool appendLine) noexcept
{
   
    bool canAppend = (_cursor + stringSize + appendLine) < _stackCapacity;                        //appendLine implicit conv 1 -true 0 - false
 
    if (canAppend)
    {
        for (size_t i{};i < stringSize;++i)
        {
            if (string[i] != L'\0')
            {
                _buffer[_cursor] = string[i];
              ++_cursor;
            }
        }
        if (appendLine)
        {
            _buffer[_cursor] = L'\n';
          ++_cursor;
        };


    }

    return canAppend;
}


inline bool string_builder::reset_cursor() noexcept
{
    bool canDo = _cursor > 0;
    _buffer[0] = L'\0';
    _cursor = 0;
    return canDo;
}


inline bool string_builder::move_cursor(std::size_t index) noexcept
{
    bool canDo = index < _cursor;

    if (canDo) _buffer[index] = L'\0';

    return canDo;
}




inline bool string_builder::move_heap(size_t size, const wchar_t* string, bool newLine)
{
    size_t requiredCapacity =_heapSize +_cursor +size + newLine + 1;


    if (_heapCapacity < requiredCapacity)

    {
        if (!isHeapAllocated)
        {
            _heapCapacity = initial_heap;
            _heapBuffer = (wchar_t*)::VirtualAlloc(nullptr, sizeof(wchar_t)* _heapCapacity,
                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            
            if (!_heapBuffer) return false;
            
            ::RtlCopyMemory(_heapBuffer, _buffer, _cursor * sizeof(wchar_t));
            
            _heapSize += _cursor;
            isHeapAllocated = true;
        }
            while (_heapCapacity < requiredCapacity && _heapCapacity < SIZE_MAX)
            {
                if (_heapCapacity >= SIZE_MAX) return false;
                _heapCapacity *= 2;
            }
        

       
            wchar_t* tmp = (wchar_t*) ::VirtualAlloc(nullptr, sizeof(wchar_t) * _heapCapacity,
                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
           
            if (!tmp) return false;

            ::RtlCopyMemory(tmp, _heapBuffer, _heapSize *sizeof(wchar_t));
            ::VirtualFree(_heapBuffer, 0, MEM_RELEASE);
           
            _heapBuffer = tmp;
            ::RtlCopyMemory(_heapBuffer + _heapSize, _buffer, _cursor * sizeof(wchar_t));
           
            _heapSize += _cursor; 
    }

    for (size_t i{ 0 }; i < size; ++i)
    {
        if (string[i] != L'\0')
        {
            _heapBuffer[_heapSize++] = string[i];
        }
    }
    _heapBuffer[_heapSize] = L'\n';
    return true;
}

inline string_builder::~string_builder()
{
    if (isHeapAllocated && _heapBuffer != nullptr) 
    {
        ::VirtualFree(_heapBuffer, 0, MEM_RELEASE);
        _heapBuffer = nullptr;
        _heapSize     = 0;
        _heapCapacity = 0;
    }
}

inline const wchar_t* string_builder::toString() noexcept
{
    _buffer[_cursor] = L'\0';
    std::printf("%zu", _cursor);
    return _buffer;
}

inline void string_builder::append(const wchar_t* string, size_t size)
{
    if (!safe_append(string, size)) 
    {

        if (move_heap(size, string))
        {
            reset_cursor();
        }
    };
}
inline void string_builder::append_line(const wchar_t* string, size_t size)
{
    if (!safe_append(string, size, true))
    {
        if (move_heap(size, string, true))
        {
            reset_cursor();
        }
    };
}
