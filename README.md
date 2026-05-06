ProcessInfo : Cross-Platform Process Information
-------------------------------------------

[![CodeQL](https://github.com/SiddiqSoft/WinProcessInfo/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/SiddiqSoft/WinProcessInfo/actions/workflows/codeql-analysis.yml)
[![Build Status](https://dev.azure.com/siddiqsoft/siddiqsoft/_apis/build/status/SiddiqSoft.WinProcessInfo?branchName=main)](https://dev.azure.com/siddiqsoft/siddiqsoft/_build/latest?definitionId=12&branchName=main)
![](https://img.shields.io/nuget/v/SiddiqSoft.WinProcessInfo)
![](https://img.shields.io/github/v/tag/SiddiqSoft/WinProcessInfo)
![](https://img.shields.io/azure-devops/tests/siddiqsoft/siddiqsoft/12)
![](https://img.shields.io/azure-devops/coverage/siddiqsoft/siddiqsoft/12)

# Objective

Provide for a simple "stats" for a daemon/service without the heft of the full process information library.

- Minimal footprint
- Serializer for nlohmann::json (you must include the library prior to this header file)
- Serializer for std::format (if supported by your compiler)


# Platform Support

This library supports the following platforms:
- **Windows** (Windows 7 and later)
- **Linux** (with /proc filesystem support)
- **macOS** (10.5 and later)
- **Unix-like systems** (with standard POSIX APIs)

# Requirements
- We use [`nlohmann::json`](https://github.com/nlohmann/json) only in our tests and the library is aware to provide a conversion operator if library is detected.
- We use `std::format` if present
- C++17 or later compiler

# Usage

## CMake with CPM (Recommended)

Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)
FetchContent_Declare(genericprocinfo
    GIT_REPOSITORY https://github.com/SiddiqSoft/WinProcessInfo.git
    GIT_TAG main
)
FetchContent_MakeAvailable(genericprocinfo)

# Link to your target
target_link_libraries(your_target PRIVATE genericprocinfo::genericprocinfo)
```

Or using CPM directly:

```cmake
include(CPM)
CPMAddPackage(
    NAME genericprocinfo
    GITHUB_REPOSITORY SiddiqSoft/WinProcessInfo
    GIT_TAG main
)

target_link_libraries(your_target PRIVATE genericprocinfo::genericprocinfo)
```

## NuGet (Windows/.NET)

- Use the nuget package [SiddiqSoft.WinProcessInfo](https://www.nuget.org/packages/SiddiqSoft.WinProcessInfo/)

## Manual Integration

- Copy the `include/siddiqsoft/GenericProcessInfo.hpp` header file to your project

## Interface

```
GenericProcessInfo (also available as WinProcessInfo for backward compatibility)
- uptime()
- snapshot()
- serializer for nlohmann::json
- serializer for std::format
```

## Example

```cpp
#include "gtest/gtest.h"
#include <format>
#include "nlohmann/json.hpp"
#include "siddiqsoft/GenericProcessInfo.hpp"

TEST(examples, Example2)
{
   try {
      siddiqsoft::GenericProcessInfo procInfo;

      // We must perform the snapshot to obtain memory and thread usage
      procInfo.snapshot();

      // tip: do not use brace-init/assignment as it will create an array instead of object!
      nlohmann::json info(procInfo);

      std::cerr << info.dump() << std::endl;
      EXPECT_EQ(siddiqsoft::GenericProcessInfo::getCurrentProcessId(), info.value("processId", 0));
   }
   catch (std::exception& e) {
      EXPECT_TRUE(false) << e.what(); // if we throw then the test fails.
   }
}

```


## Output
 Member Field | json Field | Comments
-------------:|:----------:|:-------------------
cpuCores | `cpuCount` | Number of cores/cpu
cpuHandles | `cpuHandles` | Handle count
cpuThreads | `cpuThreads` | Thread count. _This is expensive._
memPeakWorkingSet | `memPeakWorkingSet` | Peak virtual working memory in Kbytes.
memWorkingSet | `memWorkingSet` | Current working set in Kbytes.
memPrivate | `memPrivate` | Current physically allocated memory for this process in Kbytes.
nameHostname | `hostname` | The local hostname
nameDomainName | `domain` | The fully qualified domain portion
nameHostnamePhysical | `localFqdn` | The local physical hostname
nameFqdn | `fqdn` | The fully qualified dns name
processId | `processId` | The process id
timeStartup | `timeStartup` | The startup timestamp as ISO 8601 format. _This is the time of instance of this object._
&nbsp; | `timeCurrent` | The current timestamp as ISO 8601 format when the serialization took place.
&nbsp; | `uptime` | The number of _microseconds_ between the `timeStartup` and `timeCurrent`.<br/>Use the method `uptime()` and `duration_cast<>` to your desired ratio.

```json
{
    "cpuCores": 8,
    "cpuHandles": 48,
    "cpuThreads": 4,
    "domain": "",
    "fqdn": "istanbul",
    "hostname": "istanbul",
    "localFqdn": "istanbul",
    "memPeakWorkingSet": 42960, // KBytes
    "memPrivateBytes": 84452,   // Kbytes
    "memWorkingSet": 42584,     // Kbytes
    "processId": 10828,
    "timeCurrent": "2021-07-30T06:43:54.8649027Z",
    "timeStartup": "2021-07-30T06:43:54.8599558Z",
    "uptime": 5124              // Microseconds
}
```

<small align="right">

&copy; 2021 Siddiq Software LLC. All rights reserved.

</small>
