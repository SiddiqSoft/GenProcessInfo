ProcessInfo : Cross-Platform Process Information
-------------------------------------------

[![CodeQL](https://github.com/SiddiqSoft/WinProcessInfo/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/SiddiqSoft/WinProcessInfo/actions/workflows/codeql-analysis.yml)
[![Build Status](https://dev.azure.com/siddiqsoft/siddiqsoft/_apis/build/status/SiddiqSoft.WinProcessInfo?branchName=main)](https://dev.azure.com/siddiqsoft/siddiqsoft/_build/latest?definitionId=12&branchName=main)
![](https://img.shields.io/nuget/v/SiddiqSoft.WinProcessInfo)
![](https://img.shields.io/github/v/tag/SiddiqSoft/WinProcessInfo)
![](https://img.shields.io/azure-devops/tests/siddiqsoft/siddiqsoft/12)
![](https://img.shields.io/azure-devops/coverage/siddiqsoft/siddiqsoft/12)

 ## Overview

A header-only C++23 library providing cross-platform process information with minimal footprint. Captures CPU, memory, and system details for the current process.

**Features:**
- Minimal footprint - header-only library
- Cross-platform support (Windows, Linux, macOS, Unix)
- JSON serialization via `nlohmann::json`
- `std::format` support for easy output
- Single ownership semantics

## Quick Start

### Installation

<details>
<summary><b>CMake with CPM (Recommended)</b></summary>

```cmake
CPMAddPackage("gh:SiddiqSoft/GenProcessInfo#main") // replace main with specific version
target_link_libraries(${PROJECT_NAME} INTERFACE GenProcessInfo::GenProcessInfo)
```
</details>

<details>
<summary><b>NuGet (Windows)</b></summary>

```
Install-Package SiddiqSoft.WinProcessInfo
```
</details>

<details>
<summary><b>Manual</b></summary>

Copy `include/siddiqsoft/GenProcessInfo.hpp` to your project.
</details>

### Basic Usage

```cpp
#include "siddiqsoft/GenProcessInfo.hpp"

int main() {
    siddiqsoft::GenProcessInfo procInfo;
    
    // Basic info available immediately
    std::cout << "Process ID: " << procInfo.processId << "\n";
    std::cout << "CPU Cores: " << procInfo.cpuCores << "\n";
    
    // Expensive operation - call sparingly
    procInfo.snapshot();
    std::cout << "Memory: " << procInfo.memWorkingSet << " KB\n";
    std::cout << "Threads: " << procInfo.cpuThreads << "\n";
    
    return 0;
}
```

<details>
<summary><b>JSON Output Example</b></summary>

```cpp
#include <format>
#include "nlohmann/json.hpp"
#include "siddiqsoft/GenProcessInfo.hpp"

int main() {
    siddiqsoft::GenProcessInfo procInfo;
    procInfo.snapshot();
    
    nlohmann::json j = procInfo;
    std::cout << j.dump(2) << "\n";
    
    return 0;
}
```
</details>

## Requirements

- **C++23 or later** (MSVC 2022+, GCC 13+, Clang 17+)
- **Windows 7+**, **Linux** (with /proc), **macOS 10.5+**, or **POSIX-compliant systems**
- Optional: `nlohmann::json` for JSON serialization

## Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| Windows | ✓ Full | Windows 7 and later |
| Linux | ✓ Full | Requires /proc filesystem |
| macOS | ⚠ Limited | Thread count limited (requires Mach APIs) |
| Unix | ⚠ Limited | Generic POSIX fallback |

## API Reference

See [API.md](API.md) for comprehensive documentation including:
- All public members and methods
- Detailed parameter descriptions
- Platform-specific behavior
- Performance considerations
- Complete usage examples
- Thread safety notes

## Key Methods

| Method | Description | Cost |
|--------|-------------|------|
| `snapshot()` | Capture memory, handles, and thread count | Expensive |
| `uptime()` | Get elapsed time since object creation | Cheap |
| `getCurrentProcessId()` | Get current process ID (static) | Cheap |

## Key Members

| Member | Type | Description |
|--------|------|-------------|
| `processId` | `unsigned long` | Current process ID |
| `cpuCores` | `unsigned long` | Available CPU cores |
| `cpuThreads` | `unsigned long` | Process thread count |
| `cpuHandles` | `unsigned long` | Open handles/file descriptors |
| `memWorkingSet` | `size_t` | Current memory in KB |
| `memPeakWorkingSet` | `size_t` | Peak memory in KB |
| `memPrivate` | `size_t` | Private memory in KB |
| `nameHostname` | `std::string` | DNS hostname |
| `nameFqdn` | `std::string` | Fully qualified domain name |

See [API.md](API.md) for complete member documentation.

<details>
<summary><b>Performance Tips</b></summary>

1. **Reuse instances** - Create once, call `snapshot()` periodically
2. **Background thread** - Call `snapshot()` in a low-priority background thread
3. **Cache results** - Store values if frequent access is needed
4. **Avoid copies** - Copy/move operations are deleted (single ownership)
</details>

<details>
<summary><b>Backward Compatibility</b></summary>

The library provides `WinProcessInfo` as an alias for `GenProcessInfo`:
```cpp
siddiqsoft::WinProcessInfo procInfo;  // Legacy name still works
```
</details>

<details>
<summary><b>License</b></summary>

BSD 3-Clause License - See [LICENSE](LICENSE) file
</details>

---

<small align="right">

&copy; 2021 Siddiq Software LLC. All rights reserved.

</small>
