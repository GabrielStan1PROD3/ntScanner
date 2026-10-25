# Process Thread Scanner

A Windows console application written in C++20 that reads process and thread information through `NtQuerySystemInformation` and builds a formatted text report.

This is a personal learning project focused on Windows native APIs, raw memory management, resource ownership, move semantics, and variable-length structures. It is actively in development, and its interfaces and internal organization may change.

## Current functionality

- Enumerate processes and their threads from a system snapshot.
- Collect process IDs, names, thread counts, handle counts, private memory, and working sets.
- Collect thread IDs, thread states, and accumulated user-mode and kernel-mode CPU times.
- Filter processes using a function-pointer predicate through `Scanner::find_basic()`.
- Load `ntdll.dll` and resolve `NtQuerySystemInformation` at runtime.
- Manage the query buffer with `VirtualAlloc` / `VirtualFree`, retrying with a larger allocation when the query reports a size mismatch.
- Accumulate formatted process and thread rows in a reusable wide-character `string_builder`.
- Retrieve the accumulated report through `Scanner::dump()` so the caller can write it to its chosen destination.

The current executable writes the report to the console. `dump()` returns text; file output and other destinations are handled by the caller.

## Architecture

| Component | Responsibility |
| --- | --- |
| `LibraryLoader` | Load `ntdll.dll`, resolve the query function, and release its module reference |
| `ntRawBuffer` | Own the allocation used for the native query result |
| `dll_vector<SYSTEM_PROCESS_INFORMATION>` | Traverse variable-length process records in the query buffer |
| `Scanner` | Interpret process and thread records, apply predicates, and assemble report rows |
| `string_builder` | Format text and manage its inline and dynamically allocated storage |

The native snapshot and the formatted report use separate buffers. Process and thread records are read directly from the snapshot; they are not copied into individually allocated objects.

### Query and report flow

1. `LibraryLoader` resolves `NtQuerySystemInformation`.
2. `ntRawBuffer` requests the required size, allocates memory, and queries `SystemProcessInformation`. It starts with an additional 8 KiB of capacity and doubles the buffer on a size mismatch while the capacity limit permits.
3. `dll_vector<SYSTEM_PROCESS_INFORMATION>` follows `NextEntryOffset` to traverse the process records.
4. `Scanner::basic_info()` or `Scanner::find_basic()` reads process and thread data and appends formatted rows to its builder.
5. `Scanner::dump()` calls `string_builder::flush()` and returns the resulting text.
6. The caller consumes the text, for example by writing it to the console.

The snapshot is captured when a `Scanner` instance is constructed. Both scanning methods read that same snapshot. Construct a new instance to obtain a fresh one. Calling `dump()` resets the accumulated text, not the snapshot.

## String builder and memory model

`string_builder` provides `append(bool newLine, const wchar_t* format, ...)` for printf-style wide-string formatting.

- The object contains an inline buffer of **1,024 `wchar_t` elements**, or **2 KiB on Windows**. It lives wherever the builder object lives.
- Text starts in the inline buffer and moves to dynamically allocated storage when additional capacity is needed.
- The initial dynamic capacity is **4,096 `wchar_t` elements**, or **8 KiB on Windows**. A larger request can require a larger first allocation.
- Dynamic storage grows geometrically, with checked capacity arithmetic. A successful growth preserves the existing text.
- Formatting first measures the required text length, then writes into the selected buffer. `newLine == true` adds `L'\n'` after the formatted text.
- `flush()` ensures a final `L'\0'`, returns the active buffer, and resets both cursors to zero.
- Allocated capacity is retained after `flush()` for reuse. Once dynamic storage is active, subsequent appends continue using it.
- The destructor releases the dynamic allocation. Copy and move operations are disabled for the builder.

Capacities and cursors are measured in `wchar_t` elements. Allocation and copying convert those counts to bytes.

### Output lifetime

`flush()` and `Scanner::dump()` return a **borrowed `const wchar_t*`**. The builder remains the owner of the storage.

Consume or copy the returned text before another append, another flush/dump, or destruction of the owning object. The next write may overwrite the text or replace its allocation. The caller must not free the returned pointer.

## Build and run

Requirements:

- Windows.
- Visual Studio with the **Desktop development with C++** workload.
- MSVC **v145** and the Windows SDK **10.0**, as configured in the included project.

The project is configured for C++20 and includes Debug and Release configurations for Win32 and x64. Use x64 as the starting configuration; compatibility across architectures remains an area for validation.

### Visual Studio

1. Open `NativeNtdllFunctions.slnx`.
2. Select **Debug | x64** and set `ProcessThreadScanner` as the startup project if needed.
3. Build and run the console application.

### Command line

From a Visual Studio Developer Command Prompt, in the directory containing the solution:

```bat
msbuild NativeNtdllFunctions.slnx /p:Configuration=Debug /p:Platform=x64
```

## Usage

Collect a report and consume it immediately:

```cpp
#include "ProcessScanner.hpp"
#include <cstdio>

int main()
{
    Scanner scanner{};
    scanner.basic_info();

    const wchar_t* report = scanner.dump();
    std::printf("%ls", report);
    return 0;
}
```

To include only processes with at least ten threads, replace the `basic_info()` call with:

```cpp
scanner.find_basic([](BasicProcessInfo info) -> bool {
    return info.thn >= 10;
});
```

The predicate has the signature `bool (*)(BasicProcessInfo)`. It can be a regular function or a lambda without captures. Thread rows are included for each accepted process.

Both scanning methods return `Scanner&`, so collection and retrieval can also be chained:

```cpp
std::printf("%ls", scanner.basic_info().dump());
```

Each scanning call appends to the existing report. Use `dump()` between reports to reset the text for reuse. Filtering is configured in source code; the executable does not yet expose command-line options.

### Using the builder directly

```cpp
#include "StringBuilder.hpp"
#include <cstdio>

int main()
{
    string_builder text{};
    text.append(false, L"PID: %lu", 42UL);
    text.append(true, L" | Threads: %u", 6U);
    std::printf("%ls", text.flush());

    text.append(true, L"Next report");
    std::printf("%ls", text.flush());
    return 0;
}
```

The second report reuses the same builder. If dynamic storage has already been allocated, its capacity is retained between reports.

## Report fields

Process rows contain:

| Field | Description |
| --- | --- |
| `PID` | Process ID |
| `PN` | Process name |
| `THN` | Number of threads |
| `PPC` | Private memory |
| `WS` | Working set |
| `HC` | Number of handles |

Each process is followed by its thread rows:

| Field | Description |
| --- | --- |
| `TNO` | Thread number within the displayed process |
| `TID` | Thread ID |
| `UTIME` | Accumulated user-mode CPU time, in milliseconds |
| `KTIME` | Accumulated kernel-mode CPU time, in milliseconds |
| `THSTATE` | Thread state as text |

Memory values are divided by 1024 and represent KiB, although the current report labels use `Kb`.

## Project structure

Sources are located in `ProcessThreadScanner/`:

| File | Responsibility |
| --- | --- |
| `ProcessThreadScanner.cpp` | Console entry point and report consumption |
| `ProcessScanner.hpp` | Process filtering, thread-state names, report assembly, and `dump()` |
| `Specialization.hpp` | Traversal of process records through `NextEntryOffset` |
| `Alocator.hpp` | Ownership, allocation, resizing, and release of the raw query buffer |
| `Loader.hpp` | Runtime loading of `ntdll.dll` and function resolution |
| `ntTypes.hpp` | Native structure declarations and `BasicProcessInfo` |
| `StringBuilder.hpp` | Wide-character formatting, inline storage, dynamic growth, and flushing |

## Development status

The string builder is integrated into process and thread report assembly. The current focus is refining buffer state, error reporting, and the contracts between components. In particular, `append()` currently returns `void`, so callers do not yet receive a success/failure result.

