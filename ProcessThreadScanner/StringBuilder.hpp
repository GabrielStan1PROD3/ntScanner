#pragma once
#pragma once
#include "windows.h"
#include <cstdio>
#include <cstdarg>



constexpr size_t initial_capacity = 64 << 4;
constexpr size_t initial_heap     = initial_capacity << 2; 

class string_builder
{
    size_t _stackCursor    { 0 }; //# 1
    size_t _heapCursor     { 0 }; //# 2
    size_t _heapCapacity   { 0 };
    size_t _stackCapacity{ initial_capacity };
    
    wchar_t  _stackBuffer [initial_capacity];
    wchar_t* _heapBuffer    { nullptr };
    bool isStackMovedToHeap { false };
    bool isHeapAllocated    { false };
    
    bool verify_stack(size_t stringSize);          //# 3
    bool reset_cursor() noexcept;                  //# 4                                                        
   
    bool move_cursor (size_t index) noexcept;  //# 5  
    bool use_heap(size_t stringSize);          //# 6
public:
    string_builder() = default;
   ~string_builder();
   const wchar_t* toString() noexcept;  //# 7
   const wchar_t* flush() noexcept;     //# 8
   void append(bool newLine, const wchar_t* format, ...); // #9

    string_builder(const string_builder& other) = delete;
    string_builder& operator=(const string_builder& other) = delete;
    string_builder(string_builder&& other) = delete;
    string_builder& operator= (string_builder&& other) noexcept = delete;



};





/*
    #1 Number of wchar_t elements used in the stack buffer,
       excluding the null terminator.

    #2 Number of wchar_t elements used in the heap buffer,
       excluding the null terminator.

    #3 Checks whether additional text fits in the stack buffer,
       leaving room for a null terminator.

    #4 Resets the stack cursor and writes a null terminator
       at the beginning of the stack buffer.

    #5 Writes a null terminator at an index before the current
       stack cursor. The cursor itself remains unchanged.

    #6 Ensures sufficient heap capacity for existing and additional
       text. Preserves existing data when migrating or growing.

    #7 Null-terminates and returns the stack-buffer contents.

    #8 Null-terminates and returns the active buffer, then resets
       both cursors while preserving the allocated memory.
       The returned text must be consumed before the builder
       is modified or destroyed.  
    
    #9 Formats and appends text using variadic arguments.
       Adds a newline when newLine is true.
       Switches to heap storage or grows it when needed.
       ...
*/



inline bool string_builder::verify_stack(size_t stringSize)
{
    return (_stackCursor + stringSize + 1) < _stackCapacity;
}




inline bool string_builder::reset_cursor() noexcept
{
    bool canDo = _stackCursor > 0;
    _stackBuffer[0] = L'\0';
    _stackCursor = 0;
    return canDo;
}


inline bool string_builder::move_cursor(std::size_t index) noexcept
{
    bool canDo = index < _stackCursor;

    if (canDo) _stackBuffer[index] = L'\0';

    return canDo;
}





inline bool string_builder::use_heap(size_t size)
{
    const size_t max_elements = SIZE_MAX / sizeof(wchar_t);
    const size_t used = isHeapAllocated ? _heapCursor : _stackCursor;
    if (used >= max_elements || size > max_elements - used - 1) return false;

    const  size_t requiredCapacity = used + size + 1;
    
    if (isHeapAllocated && _heapCapacity >= requiredCapacity) return true;
    
    size_t newCapacity = _heapCapacity;
    if (newCapacity == 0) newCapacity = initial_heap;

    if (newCapacity > max_elements) return false;
        
    while(newCapacity < requiredCapacity)
    {
        if (newCapacity > max_elements / 2)
        {
            newCapacity = requiredCapacity;
            break;
        }

        newCapacity *= 2;
    }
    wchar_t* newBuffer = static_cast<wchar_t*>(::VirtualAlloc(
        nullptr,
        newCapacity * sizeof(wchar_t),
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    ));
    if (!newBuffer)
        return false;

    const wchar_t* source = isHeapAllocated ? _heapBuffer : _stackBuffer;
    
    if (used != 0)
        ::RtlCopyMemory(newBuffer, source, used * sizeof(wchar_t));

    if (isHeapAllocated)
        ::VirtualFree(_heapBuffer, 0, MEM_RELEASE);

    _heapBuffer = newBuffer;
    _heapCapacity = newCapacity;
    _heapCursor = used;
    _stackCursor = 0;
    isHeapAllocated = true;
    isStackMovedToHeap = true;

    return true;
    }

inline string_builder::~string_builder()
{
    if (isHeapAllocated && _heapBuffer != nullptr) 
    {
        ::VirtualFree(_heapBuffer, 0, MEM_RELEASE);
        _heapBuffer = nullptr;
        _heapCursor     = 0;
        _heapCapacity = 0;
    }
}

inline const wchar_t* string_builder::toString() noexcept
{
    _stackBuffer[_stackCursor] = L'\0';
    std::printf("%zu", _stackCursor);
    return _stackBuffer;
}

inline const wchar_t* string_builder::flush() noexcept
{
    wchar_t*     data = isHeapAllocated ? _heapBuffer : _stackBuffer;
    const size_t used = isHeapAllocated ? _heapCursor : _stackCursor;
    data[used] = L'\0';
    _stackCursor = 0;
    _heapCursor  = 0;
   return data;
}


inline void string_builder::append( bool newLine,const wchar_t* format, ...)
{
    if (!format) return;

    va_list     args;
    va_start    (args, format);

    va_list      measure;
    va_copy     (measure, args);
    

    const int length = _vscwprintf(format, measure);

    va_end(measure);

    if (length < 0) 
    {
        va_end(args);
        return;
    }

    const size_t textSize  = static_cast<size_t>(length);
    const size_t addedSize = textSize + static_cast<size_t>(newLine);


    const bool onStack = !isStackMovedToHeap && verify_stack(addedSize);

    if (!onStack && !use_heap(addedSize))
    {
        va_end(args);
        return;
    }
    wchar_t* destination = onStack ? _stackBuffer + _stackCursor : _heapBuffer + _heapCursor;

    const int written = length > 0
        ? _vsnwprintf_s(
            destination,
            textSize + 1, 
            textSize,     
            format,
            args)
        : 0;
    va_end(args);

    if (written != length) return;

    size_t& used = onStack ? _stackCursor : _heapCursor;
    used += textSize;
    if (newLine)
    {
        destination[textSize] = L'\n';
        ++used;
    }
}
