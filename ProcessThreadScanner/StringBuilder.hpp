#pragma once
#pragma once
#include "windows.h"
#include <cstdio>

/*
   compile time fixed size default is 1024 bytes
*/
template<std::size_t capacity = 64 << 4>
class string_builder
{
    char _buffer[capacity];
    std::size_t _cursor{ 0 };                                                                               //# 5
    std::size_t _capacity{ capacity };
    bool safe_append(const char* string, std::size_t stringSize, bool appendLine = false) noexcept;         //# 1
    bool reset_cursor() noexcept;                                                                           //# 2
    bool move_cursor(std::size_t index) noexcept;                                                           //# 3

public:
    string_builder() = default;
   ~string_builder() = default;
    string_builder(char* string, std::size_t stringSize);                                                       
 
    const char* toString() noexcept;                                                                         //# 4
    void append(const char* string, std::size_t size);
    void append_line(const char* string, std::size_t size);

};

/*
      
     # 5 _cursor represents the actual size of the _buffer 

     # 1 ensures that the string is apended corectly ,removes the null pointer and adds it to the buffer 
     # 2 puts  the nullterminator to the begging of the buffer 
     # 3 moves the cursor to an index size and puts there the null-terminator 
     # 4 converts the buffer into an string  (adds the null terminator to the end)
*/



template <std::size_t capacity>
inline bool string_builder<capacity>::safe_append(const char* string, std::size_t stringSize, bool appendLine) noexcept
{
    bool canAppend = _cursor + stringSize < capacity;
    if (canAppend)
    {
        for (std::size_t i{};i < stringSize;++i)
        {
            if (string[i] != '\0')
            {
                _buffer[_cursor] = string[i];
                ++_cursor;
            }
        }
        if (appendLine)
        {
            _buffer[_cursor] = '\n';
            ++_cursor;
        };


    }

    return canAppend;
}

template <std::size_t capacity>
inline bool string_builder<capacity>::reset_cursor() noexcept
{
    bool canDo = _cursor > 0;
    _buffer[0] = '\0';
    _cursor = 0;
    return canDo;
}

template <std::size_t capacity>
inline bool string_builder<capacity>::move_cursor(std::size_t index) noexcept
{
    bool canDo = index < _cursor;

    if (canDo) _buffer[index] = '\0';

    return canDo;
}

template <std::size_t capacity>
inline string_builder<capacity>::string_builder(char* string, std::size_t stringSize)
{
         safe_append(string, stringSize);
}  


template <std::size_t capacity>
inline const char* string_builder<capacity>::toString() noexcept
{
    _buffer[_cursor] = '\0';
    std::printf("%zu", _cursor);
    return _buffer;
}

template <std::size_t capacity>
inline void string_builder<capacity>::append(const char* string, std::size_t size)
{
    if (!safe_append(string, size)) { reset_cursor(); };
}

template <std::size_t capacity>
inline void string_builder<capacity>::append_line(const char* string, std::size_t size)
{
    if (!safe_append(string, size, true)) { reset_cursor(); };

}
