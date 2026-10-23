# Process Thread Scanner

A Windows console application written in C++20 that reads and displays process and thread information through `NtQuerySystemInformation`.

This is a personal learning project focused on Windows native APIs, raw memory management, resource ownership, move semantics, and working with variable-length structures. It is actively in development, and its interfaces and internal organization may change.

## Current functionality

- Enumerate processes and their threads from a system snapshot.
- Display process IDs, names, thread counts, handle counts, private memory, and working sets.
- Display thread IDs, thread states, and accumulated user-mode and kernel-mode CPU times.
- Filter processes using a function-pointer predicate through `Scanner::find_basic()`.
- Load `ntdll.dll` and resolve `NtQuerySystemInformation` at runtime.
- Manage the query buffer with `VirtualAlloc` / `VirtualFree`, retrying with a larger allocation when the query reports a size mismatch.

## How it works

1. `LibraryLoader` loads `ntdll.dll` and resolves the query function.
2. `ntRawBuffer` requests the required buffer size, allocates memory, and queries `SystemProcessInformation`. If the buffer is too small, it doubles its capacity and retries while the capacity limit permits.
3. `dll_vector<SYSTEM_PROCESS_INFORMATION>` traverses the variable-length process records using `NextEntryOffset`.
4. `Scanner` reads each process and its thread array, applies an optional predicate, and prints the results.

The snapshot is captured when a `Scanner` instance is constructed. Both scanning methods read that same snapshot. Construct a new instance to obtain a fresh one.

## Output

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

Memory values are divided by 1024 and represent KiB, although the current console labels use `Kb`.

## Build and run

Requirements:

- Windows.
- Visual Studio with the **Desktop development with C++** workload.
- MSVC **v145** and the Windows SDK **10.0**, as configured in the included project.

The project is configured for C++20 and includes Debug and Release configurations for Win32 and x64.

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

The current entry point prints all processes and their threads:

```cpp
#include "ProcessScanner.hpp"

int main()
{
    Scanner scanner{};
    scanner.basic_info();
    return 0;
}
```

To display only processes with at least ten threads, replace the `basic_info()` call with:

```cpp
scanner.find_basic([](BasicProcessInfo info) -> bool {
    return info.thn >= 10;
});
```

The predicate has the signature `bool (*)(BasicProcessInfo)`. It can be a regular function or a lambda without captures. Threads are displayed for each process accepted by the predicate.

Filtering is currently configured in source code; the executable does not yet expose command-line options.

## Project structure

Sources are located in `ProcessThreadScanner/`:

| File | Responsibility |
| --- | --- |
| `ProcessThreadScanner.cpp` | Console entry point |
| `ProcessScanner.hpp` | Process filtering, thread-state names, and console output |
| `Specialization.hpp` | Traversal of process records through `NextEntryOffset` |
| `Alocator.hpp` | Ownership, allocation, resizing, and release of the raw query buffer |
| `Loader.hpp` | Runtime loading of `ntdll.dll` and function resolution |
| `ntTypes.hpp` | Native structure declarations and `BasicProcessInfo` |
| `StringBuilder.hpp` | Experimental string builder with a capacity set at compile time |

## Development status

Process and thread output currently uses `printf` directly. The string builder is still being developed and is not integrated into the scanner's output.

Ideas for the next iterations include separating record parsing from scanner orchestration, developing formatted output and buffer flushing, and introducing a dedicated synchronization wrapper when multithreaded use is added.

The implementation is Windows-specific and uses locally declared native structures. Compatibility across Windows versions and architectures is still an area for validation.
