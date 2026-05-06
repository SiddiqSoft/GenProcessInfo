# GenericProcessInfo API Documentation

## Overview

`GenericProcessInfo` is a header-only C++23 library that provides cross-platform process information including CPU, memory, and system details. The class captures and maintains information about the current process with minimal overhead.

**Namespace**: `siddiqsoft`

**Backward Compatibility**: Also available as `WinProcessInfo` for legacy code.

---

## Class: GenericProcessInfo

### Public Members

#### CPU Information

##### `unsigned long cpuCores`
- **Description**: Number of CPU cores available on the system
- **Type**: `unsigned long`
- **Initialized**: During construction
- **Platform Notes**: 
  - Windows: Retrieved via `GetSystemInfo()`
  - Unix/Linux: Retrieved via `sysconf(_SC_NPROCESSORS_ONLN)`

##### `unsigned long cpuHandles`
- **Description**: Number of handles (Windows) or file descriptors (Unix/Linux) currently used by the process
- **Type**: `unsigned long`
- **Initialized**: `0` (populated by `snapshot()`)
- **Platform Notes**:
  - Windows: Retrieved via `GetProcessHandleCount()`
  - Linux: Read from `/proc/self/status` (FDSize field)
  - macOS: Enumerated from `/dev/fd`

##### `unsigned long cpuThreads`
- **Description**: Number of threads in the current process
- **Type**: `unsigned long`
- **Initialized**: `0` (populated by `snapshot()`)
- **Performance**: Expensive operation - call sparingly
- **Platform Notes**:
  - Windows: Retrieved via `CreateToolhelp32Snapshot()`
  - Linux: Read from `/proc/self/status` (Threads field)
  - macOS: Fallback to `1` (requires Mach APIs for accurate count)

#### Memory Information

##### `size_t memPeakWorkingSet`
- **Description**: Peak working set memory used by the process in kilobytes
- **Type**: `size_t`
- **Unit**: Kilobytes (KB)
- **Initialized**: `0` (populated by `snapshot()`)
- **Platform Notes**:
  - Windows: Retrieved via `GetProcessMemoryInfo()` (PeakWorkingSetSize)
  - Linux: Read from `/proc/self/status` (VmPeak field)
  - macOS: Retrieved via `getrusage()` (ru_maxrss)

##### `size_t memWorkingSet`
- **Description**: Current working set memory used by the process in kilobytes
- **Type**: `size_t`
- **Unit**: Kilobytes (KB)
- **Initialized**: `0` (populated by `snapshot()`)
- **Platform Notes**:
  - Windows: Retrieved via `GetProcessMemoryInfo()` (WorkingSetSize)
  - Linux: Read from `/proc/self/status` (VmRSS field)
  - macOS: Retrieved via `getrusage()` (ru_maxrss)

##### `size_t memPrivate`
- **Description**: Private memory allocated for this process in kilobytes
- **Type**: `size_t`
- **Unit**: Kilobytes (KB)
- **Initialized**: `0` (populated by `snapshot()`)
- **Platform Notes**:
  - Windows: Retrieved via `GetProcessMemoryInfo()` (PrivateUsage)
  - Linux: Read from `/proc/self/status` (VmData field)
  - macOS: Retrieved via `getrusage()` (ru_maxrss)

#### Hostname and Domain Information

##### `std::string nameHostname`
- **Description**: DNS hostname of the local machine
- **Type**: `std::string`
- **Initialized**: During construction
- **Platform Notes**:
  - Windows: Retrieved via `GetComputerNameExA()` with `ComputerNameDnsHostname`
  - Unix/Linux: Retrieved via `gethostname()`

##### `std::string nameDomainName`
- **Description**: Domain name portion of the fully qualified domain name
- **Type**: `std::string`
- **Initialized**: During construction
- **Platform Notes**:
  - Windows: Retrieved via `GetComputerNameExA()` with `ComputerNameDnsDomain`
  - Unix/Linux: Extracted from FQDN if available

##### `std::string nameHostnamePhysical`
- **Description**: Physical DNS hostname of the local machine
- **Type**: `std::string`
- **Initialized**: During construction
- **Platform Notes**:
  - Windows: Retrieved via `GetComputerNameExA()` with `ComputerNamePhysicalDnsFullyQualified`
  - Unix/Linux: Retrieved via `gethostname()`

##### `std::string nameFqdn`
- **Description**: Fully qualified domain name (FQDN) of the local machine
- **Type**: `std::string`
- **Initialized**: During construction
- **Platform Notes**:
  - Windows: Retrieved via `GetComputerNameExA()` with `ComputerNameDnsFullyQualified`
  - Unix/Linux: Retrieved via `uname()` (uts.nodename)

#### Time Information

##### `std::chrono::system_clock::time_point timeStartup`
- **Description**: Timestamp when this GenericProcessInfo instance was created
- **Type**: `std::chrono::system_clock::time_point`
- **Initialized**: During construction (current system time)
- **Note**: This is NOT the process startup time, but the time of object instantiation

#### Process Identification

##### `unsigned long processId`
- **Description**: Process ID of the current process
- **Type**: `unsigned long`
- **Initialized**: During construction
- **Platform Notes**:
  - Windows: Retrieved via `GetCurrentProcessId()`
  - Unix/Linux: Retrieved via `getpid()`

##### `HANDLE processHandle` (Windows only)
- **Description**: Handle to the current process (Windows platform only)
- **Type**: `HANDLE`
- **Initialized**: During construction (Windows only)
- **Availability**: Only defined when `SIDDIQSOFT_WINDOWS` is set
- **Note**: Automatically cleaned up in destructor

---

### Public Methods

#### Constructor

```cpp
GenericProcessInfo()
```

- **Description**: Default constructor that initializes process information
- **Behavior**: 
  - Captures system information including CPU cores, process ID, and hostname details
  - Calls platform-specific initialization (`initializeWindows()` or `initializeUnix()`)
  - Does NOT call `snapshot()` - memory and thread information are not populated
- **Exceptions**: None (noexcept)
- **Example**:
  ```cpp
  siddiqsoft::GenericProcessInfo procInfo;
  // procInfo now contains CPU cores, process ID, hostname, etc.
  // But memory and thread info are still 0
  ```

#### Destructor

```cpp
~GenericProcessInfo() noexcept
```

- **Description**: Destructor that cleans up resources
- **Behavior**:
  - On Windows: Closes the process handle if it was opened
  - On Unix/Linux: No special cleanup needed
- **Exceptions**: None (noexcept)

#### snapshot()

```cpp
void snapshot() noexcept
```

- **Description**: Collects a snapshot of memory, handle, and thread count information
- **Return**: `void`
- **Exceptions**: None (noexcept)
- **Performance**: **Expensive operation** - should be called sparingly
- **Recommendation**: Consider using a low-priority background thread to measure these statistics periodically
- **Behavior**:
  - Calls `getMemoryInfo()` to populate memory-related fields
  - Calls `getCurrentThreadCount()` to populate thread count
  - Updates: `memPeakWorkingSet`, `memWorkingSet`, `memPrivate`, `cpuHandles`, `cpuThreads`
- **Example**:
  ```cpp
  siddiqsoft::GenericProcessInfo procInfo;
  procInfo.snapshot();  // Now memory and thread info are populated
  std::cout << "Memory: " << procInfo.memWorkingSet << " KB\n";
  std::cout << "Threads: " << procInfo.cpuThreads << "\n";
  ```

#### uptime()

```cpp
[[nodiscard]] std::chrono::system_clock::duration uptime() const noexcept
```

- **Description**: Calculates the elapsed time since this GenericProcessInfo instance was created
- **Return**: `std::chrono::system_clock::duration` - Duration representing elapsed time
- **Exceptions**: None (noexcept)
- **Attributes**: `[[nodiscard]]` - Compiler will warn if return value is ignored
- **Note**: This is the uptime of the object instance, not the process itself
- **Usage**: Cast to desired precision using `std::chrono::duration_cast<>`
- **Example**:
  ```cpp
  siddiqsoft::GenericProcessInfo procInfo;
  // ... do some work ...
  auto elapsed = procInfo.uptime();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
  std::cout << "Elapsed: " << ms.count() << " ms\n";
  ```

#### getCurrentProcessId()

```cpp
static auto getCurrentProcessId() noexcept
```

- **Description**: Get the current process ID in a cross-platform manner
- **Return**: Process ID (platform-dependent type)
  - Windows: `DWORD` (from `GetCurrentProcessId()`)
  - Unix/Linux: `pid_t` (from `getpid()`)
- **Exceptions**: None (noexcept)
- **Static**: Yes - can be called without creating an instance
- **Example**:
  ```cpp
  auto pid = siddiqsoft::GenericProcessInfo::getCurrentProcessId();
  std::cout << "Process ID: " << pid << "\n";
  ```

---

### Deleted Methods

The following operations are explicitly deleted to enforce single ownership semantics:

```cpp
GenericProcessInfo(const GenericProcessInfo&)            = delete;
GenericProcessInfo& operator=(const GenericProcessInfo&) = delete;
GenericProcessInfo(GenericProcessInfo&&)                 = delete;
GenericProcessInfo& operator=(GenericProcessInfo&&)      = delete;
```

- **Copy Constructor**: Deleted - prevents copying
- **Copy Assignment**: Deleted - prevents copy assignment
- **Move Constructor**: Deleted - prevents moving
- **Move Assignment**: Deleted - prevents move assignment
- **Rationale**: Single ownership semantics; each instance manages its own resources

---

## Serialization

### JSON Serialization (nlohmann::json)

When `nlohmann/json.hpp` is included **before** `GenericProcessInfo.hpp`, automatic JSON serialization is available:

```cpp
#include "nlohmann/json.hpp"
#include "siddiqsoft/GenericProcessInfo.hpp"

siddiqsoft::GenericProcessInfo procInfo;
procInfo.snapshot();

nlohmann::json j = procInfo;  // Automatic conversion
std::cout << j.dump(2) << "\n";
```

**JSON Output Structure**:
```json
{
  "processId": 12345,
  "hostname": "mycomputer",
  "fqdn": "mycomputer.example.com",
  "domain": "example.com",
  "localFqdn": "mycomputer.example.com",
  "cpuHandles": 48,
  "cpuThreads": 4,
  "cpuCores": 8,
  "memPeakWorkingSet": 42960,
  "memWorkingSet": 42584,
  "memPrivateBytes": 84452,
  "timeStartup": "2024-01-15T10:30:45Z",
  "timeCurrent": "2024-01-15T10:30:50Z",
  "uptime": 5000000
}
```

**Field Mapping**:
| Member Field | JSON Field | Type | Unit |
|---|---|---|---|
| `processId` | `processId` | number | - |
| `nameHostname` | `hostname` | string | - |
| `nameFqdn` | `fqdn` | string | - |
| `nameDomainName` | `domain` | string | - |
| `nameHostnamePhysical` | `localFqdn` | string | - |
| `cpuHandles` | `cpuHandles` | number | - |
| `cpuThreads` | `cpuThreads` | number | - |
| `cpuCores` | `cpuCores` | number | - |
| `memPeakWorkingSet` | `memPeakWorkingSet` | number | KB |
| `memWorkingSet` | `memWorkingSet` | number | KB |
| `memPrivate` | `memPrivateBytes` | number | KB |
| `timeStartup` | `timeStartup` | string | ISO 8601 |
| (current time) | `timeCurrent` | string | ISO 8601 |
| (calculated) | `uptime` | number | microseconds |

### std::format Serialization

When using C++23 `std::format`, GenericProcessInfo can be formatted directly:

```cpp
#include <format>
#include "siddiqsoft/GenericProcessInfo.hpp"

siddiqsoft::GenericProcessInfo procInfo;
procInfo.snapshot();

std::cout << std::format("Process Info: {}\n", procInfo);
```

**Output**: JSON-formatted string (same structure as JSON serialization)

---

## Usage Examples

### Basic Usage

```cpp
#include "siddiqsoft/GenericProcessInfo.hpp"

int main() {
    siddiqsoft::GenericProcessInfo procInfo;
    
    // Access basic information (available immediately)
    std::cout << "Process ID: " << procInfo.processId << "\n";
    std::cout << "CPU Cores: " << procInfo.cpuCores << "\n";
    std::cout << "Hostname: " << procInfo.nameHostname << "\n";
    
    return 0;
}
```

### With Memory and Thread Snapshot

```cpp
#include "siddiqsoft/GenericProcessInfo.hpp"

int main() {
    siddiqsoft::GenericProcessInfo procInfo;
    
    // Perform expensive snapshot operation
    procInfo.snapshot();
    
    // Now memory and thread information are available
    std::cout << "Working Set: " << procInfo.memWorkingSet << " KB\n";
    std::cout << "Threads: " << procInfo.cpuThreads << "\n";
    std::cout << "Handles: " << procInfo.cpuHandles << "\n";
    
    return 0;
}
```

### Measuring Uptime

```cpp
#include <chrono>
#include "siddiqsoft/GenericProcessInfo.hpp"

int main() {
    siddiqsoft::GenericProcessInfo procInfo;
    
    // ... do some work ...
    
    auto elapsed = procInfo.uptime();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed);
    
    std::cout << "Elapsed: " << seconds.count() << " seconds\n";
    
    return 0;
}
```

### JSON Serialization

```cpp
#include <format>
#include "nlohmann/json.hpp"
#include "siddiqsoft/GenericProcessInfo.hpp"

int main() {
    siddiqsoft::GenericProcessInfo procInfo;
    procInfo.snapshot();
    
    nlohmann::json j = procInfo;
    std::cout << j.dump(2) << "\n";
    
    return 0;
}
```

### Using std::format

```cpp
#include <format>
#include "siddiqsoft/GenericProcessInfo.hpp"

int main() {
    siddiqsoft::GenericProcessInfo procInfo;
    procInfo.snapshot();
    
    std::cout << std::format("Process Info: {}\n", procInfo);
    
    return 0;
}
```

### Periodic Monitoring

```cpp
#include <thread>
#include <chrono>
#include "siddiqsoft/GenericProcessInfo.hpp"

void monitorProcess() {
    siddiqsoft::GenericProcessInfo procInfo;
    
    while (true) {
        procInfo.snapshot();  // Expensive operation
        
        std::cout << "Memory: " << procInfo.memWorkingSet << " KB, "
                  << "Threads: " << procInfo.cpuThreads << "\n";
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

int main() {
    std::thread monitor(monitorProcess);
    monitor.detach();
    
    // ... main application logic ...
    
    return 0;
}
```

---

## Platform-Specific Behavior

### Windows
- Uses Windows API for all operations
- Requires linking with `psapi.lib` (automatically handled via `#pragma comment`)
- Provides accurate thread count via process snapshot
- Supports all hostname variants (DNS, physical, FQDN, domain)

### Linux
- Reads from `/proc/self/status` for memory and thread information
- Requires `/proc` filesystem support
- Provides accurate thread count
- Hostname information via standard POSIX APIs

### macOS
- Uses `getrusage()` for memory information
- File descriptor enumeration via `/dev/fd`
- Thread count limited to fallback value (requires Mach APIs for accuracy)
- Hostname information via standard POSIX APIs

### Unix-like Systems
- Generic fallback using `getrusage()` and POSIX APIs
- May have limited accuracy for some metrics
- Hostname resolution via `gethostname()` and `uname()`

---

## Performance Considerations

### Expensive Operations
- **`snapshot()`**: Expensive - involves system calls and file I/O
  - Recommendation: Call sparingly, consider background thread for periodic updates
- **`cpuThreads`**: Expensive to retrieve
  - Windows: Requires process snapshot enumeration
  - Linux: Requires reading `/proc/self/status`

### Cheap Operations
- **Constructor**: Lightweight - basic system information retrieval
- **`uptime()`**: Very cheap - simple time calculation
- **`getCurrentProcessId()`**: Very cheap - single system call
- **Memory access**: All member variables are public and directly accessible

### Optimization Tips
1. Create a single `GenericProcessInfo` instance and reuse it
2. Call `snapshot()` in a low-priority background thread
3. Cache results if frequent access is needed
4. Use `uptime()` instead of creating new instances for timing

---

## Thread Safety

- **Not thread-safe**: The class is not designed for concurrent access
- **Recommendation**: Create separate instances per thread or use external synchronization
- **Single ownership**: Deleted copy/move operations enforce single ownership semantics

---

## Backward Compatibility

The library provides a type alias for backward compatibility:

```cpp
using WinProcessInfo = GenericProcessInfo;
```

Legacy code can use either name:
```cpp
siddiqsoft::WinProcessInfo procInfo;  // Old name
siddiqsoft::GenericProcessInfo procInfo;  // New name
```

---

## Compiler Support

- **C++23 or later** required
- **MSVC**: Visual Studio 2022 or later
- **GCC**: 13 or later
- **Clang**: 17 or later (with `-fexperimental-library` flag)

---

## Dependencies

- **Required**: C++23 standard library
- **Optional**: `nlohmann/json` for JSON serialization
- **Optional**: `std::format` (C++20+) for formatting

---

## License

BSD 3-Clause License - See LICENSE file for details

---

## See Also

- [README.md](README.md) - Project overview and usage guide
- [Changelog.md](Changelog.md) - Version history and changes
