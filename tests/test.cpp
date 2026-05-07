/*
    GenProcessInfo : tests
    Process Information Tests

    Repo: https://github.com/SiddiqSoft/GenProcessInfo

    BSD 3-Clause License

    Copyright (c) 2021, Siddiq Software LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its
       contributors may be used to endorse or promote products derived from
       this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "gtest/gtest.h"
#include <format>

#include <thread>
#include <sstream>
#include "nlohmann/json.hpp"
#include "../include/siddiqsoft/GenProcessInfo.hpp"


// ============================================================================
// Initialization and Basic Properties Tests
// ============================================================================

TEST(initialization, DefaultConstructor)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		// Verify that basic properties are initialized
		EXPECT_EQ(siddiqsoft::GenProcessInfo::GetCurrentProcessId(), procInfo.processId);
		EXPECT_GT(procInfo.cpuCores, 0);
		EXPECT_FALSE(procInfo.nameHostname.empty());
		EXPECT_FALSE(procInfo.nameFqdn.empty());
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Constructor should not throw";
	}
}

TEST(initialization, HostnameProperties)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		// Verify hostname-related properties are populated
		EXPECT_FALSE(procInfo.nameHostname.empty());
		EXPECT_FALSE(procInfo.nameFqdn.empty());
		EXPECT_FALSE(procInfo.nameHostnamePhysical.empty());
		// Domain name may be empty on some systems
		EXPECT_GE(procInfo.nameHostname.length(), 1);
		EXPECT_GE(procInfo.nameFqdn.length(), 1);

		std::cerr << "Hostname: " << procInfo.nameHostname << std::endl;
		std::cerr << "FQDN: " << procInfo.nameFqdn << std::endl;
		std::cerr << "Physical Hostname: " << procInfo.nameHostnamePhysical << std::endl;
		std::cerr << "Domain: " << procInfo.nameDomainName << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Hostname initialization should not throw";
	}
}

TEST(initialization, CPUProperties)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		// Verify CPU-related properties
		EXPECT_GT(procInfo.cpuCores, 0);
		EXPECT_LE(procInfo.cpuCores, 1024); // Reasonable upper bound

		std::cerr << "CPU Cores: " << procInfo.cpuCores << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "CPU initialization should not throw";
	}
}

TEST(initialization, ProcessIDProperty)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		// Verify process ID matches current process
		EXPECT_EQ(procInfo.processId, siddiqsoft::GenProcessInfo::GetCurrentProcessId());
		EXPECT_GT(procInfo.processId, 0);

		std::cerr << "Process ID: " << procInfo.processId << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Process ID initialization should not throw";
	}
}


// ============================================================================
// Snapshot and Memory Information Tests
// ============================================================================

TEST(snapshot, SnapshotCollection)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		// Perform snapshot
		procInfo.snapshot();

		// After snapshot, memory and thread info should be populated
		EXPECT_GE(procInfo.memWorkingSet, 0);
		EXPECT_GE(procInfo.memPeakWorkingSet, 0);
		EXPECT_GE(procInfo.memPrivate, 0);
		EXPECT_GE(procInfo.cpuThreads, 1); // At least one thread

		std::cerr << "Working Set: " << procInfo.memWorkingSet << " KB" << std::endl;
		std::cerr << "Peak Working Set: " << procInfo.memPeakWorkingSet << " KB" << std::endl;
		std::cerr << "Private Memory: " << procInfo.memPrivate << " KB" << std::endl;
		std::cerr << "Thread Count: " << procInfo.cpuThreads << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Snapshot should not throw";
	}
}

TEST(snapshot, MultipleSnapshots)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		// Take multiple snapshots
		procInfo.snapshot();
		auto firstMemory = procInfo.memWorkingSet;
		auto firstThreads = procInfo.cpuThreads;

		procInfo.snapshot();
		auto secondMemory = procInfo.memWorkingSet;
		auto secondThreads = procInfo.cpuThreads;

		// Values should be reasonable (may vary slightly between snapshots)
		EXPECT_GE(firstMemory, 0);
		EXPECT_GE(secondMemory, 0);
		EXPECT_GE(firstThreads, 1);
		EXPECT_GE(secondThreads, 1);

		std::cerr << "First snapshot - Memory: " << firstMemory << " KB, Threads: " << firstThreads << std::endl;
		std::cerr << "Second snapshot - Memory: " << secondMemory << " KB, Threads: " << secondThreads << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Multiple snapshots should not throw";
	}
}

TEST(snapshot, HandleCount)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		procInfo.snapshot();

		// Handle/file descriptor count should be reasonable
		EXPECT_GE(procInfo.cpuHandles, 0);
		EXPECT_LE(procInfo.cpuHandles, 100000); // Reasonable upper bound

		std::cerr << "Handle Count: " << procInfo.cpuHandles << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Handle count snapshot should not throw";
	}
}


// ============================================================================
// Uptime Tests
// ============================================================================

TEST(uptime, UptimeCalculation)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		auto uptime = procInfo.uptime();

		// Uptime should be non-negative
		EXPECT_GE(uptime.count(), 0);

		// Convert to milliseconds for display
		auto uptimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(uptime);
		std::cerr << "Uptime: " << uptimeMs.count() << " ms" << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Uptime calculation should not throw";
	}
}

TEST(uptime, UptimeIncreases)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		auto uptime1 = procInfo.uptime();

		// Sleep for a short time
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		auto uptime2 = procInfo.uptime();

		// Second uptime should be greater than or equal to first
		EXPECT_GE(uptime2.count(), uptime1.count());

		std::cerr << "Uptime 1: " << uptime1.count() << " ns" << std::endl;
		std::cerr << "Uptime 2: " << uptime2.count() << " ns" << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Uptime comparison should not throw";
	}
}

TEST(uptime, UptimeDurationTypes)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		auto uptimeNs = procInfo.uptime();
		auto uptimeUs = std::chrono::duration_cast<std::chrono::microseconds>(uptimeNs);
		auto uptimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(uptimeNs);
		auto uptimeS = std::chrono::duration_cast<std::chrono::seconds>(uptimeNs);

		// All should be non-negative
		EXPECT_GE(uptimeNs.count(), 0);
		EXPECT_GE(uptimeUs.count(), 0);
		EXPECT_GE(uptimeMs.count(), 0);
		EXPECT_GE(uptimeS.count(), 0);

		std::cerr << "Uptime - ns: " << uptimeNs.count() << ", us: " << uptimeUs.count()
		          << ", ms: " << uptimeMs.count() << ", s: " << uptimeS.count() << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Uptime duration conversions should not throw";
	}
}


// ============================================================================
// JSON Serialization Tests
// ============================================================================

TEST(json, JSONSerialization)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;
		procInfo.snapshot();

		nlohmann::json info(procInfo);

		// Verify all expected fields are present
		EXPECT_TRUE(info.contains("processId"));
		EXPECT_TRUE(info.contains("hostname"));
		EXPECT_TRUE(info.contains("fqdn"));
		EXPECT_TRUE(info.contains("domain"));
		EXPECT_TRUE(info.contains("localFqdn"));
		EXPECT_TRUE(info.contains("cpuHandles"));
		EXPECT_TRUE(info.contains("cpuThreads"));
		EXPECT_TRUE(info.contains("cpuCores"));
		EXPECT_TRUE(info.contains("memPeakWorkingSet"));
		EXPECT_TRUE(info.contains("memWorkingSet"));
		EXPECT_TRUE(info.contains("memPrivateBytes"));
		EXPECT_TRUE(info.contains("timeStartup"));
		EXPECT_TRUE(info.contains("timeCurrent"));
		EXPECT_TRUE(info.contains("uptime"));

		std::cerr << "JSON serialization successful" << std::endl;
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << "JSON serialization should not throw: " << e.what();
	}
}

TEST(json, JSONFieldValues)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;
		procInfo.snapshot();

		nlohmann::json info(procInfo);

		// Verify field values
		EXPECT_EQ(info.value("processId", 0), siddiqsoft::GenProcessInfo::GetCurrentProcessId());
		EXPECT_EQ(info.value("hostname", ""), procInfo.nameHostname);
		EXPECT_EQ(info.value("fqdn", ""), procInfo.nameFqdn);
		EXPECT_EQ(info.value("cpuCores", 0), procInfo.cpuCores);
		EXPECT_EQ(info.value("cpuThreads", 0), procInfo.cpuThreads);
		EXPECT_EQ(info.value("memWorkingSet", 0), procInfo.memWorkingSet);

		std::cerr << "JSON field values verified" << std::endl;
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << "JSON field verification should not throw: " << e.what();
	}
}

TEST(json, JSONDump)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;
		procInfo.snapshot();

		nlohmann::json info(procInfo);
		std::string jsonStr = info.dump(4);

		// Verify JSON string is not empty and contains expected content
		EXPECT_FALSE(jsonStr.empty());
		EXPECT_GT(jsonStr.length(), 50);
		EXPECT_NE(jsonStr.find("processId"), std::string::npos);
		EXPECT_NE(jsonStr.find("hostname"), std::string::npos);

		std::cerr << "JSON dump:\n" << jsonStr << std::endl;
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << "JSON dump should not throw: " << e.what();
	}
}


// ============================================================================
// Formatting Tests
// ============================================================================

TEST(formatting, StdFormat)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;
		procInfo.snapshot();

		std::string formatted = std::format("{}", procInfo);

		// Verify formatted string is not empty and contains JSON
		EXPECT_FALSE(formatted.empty());
		EXPECT_GT(formatted.length(), 50);
		EXPECT_NE(formatted.find("processId"), std::string::npos);

		std::cerr << "Formatted output:\n" << formatted << std::endl;
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << "std::format should not throw: " << e.what();
	}
}

TEST(formatting, StreamOutput)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;
		procInfo.snapshot();

		std::ostringstream oss;
		oss << std::format("{}", procInfo);

		std::string output = oss.str();

		// Verify output is not empty
		EXPECT_FALSE(output.empty());
		EXPECT_GT(output.length(), 50);

		std::cerr << "Stream output:\n" << output << std::endl;
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << "Stream output should not throw: " << e.what();
	}
}


// ============================================================================
// Copy and Move Semantics Tests
// ============================================================================

TEST(semantics, NoCopyConstruction)
{
	// This test verifies that copy construction is deleted
	// If this compiles, the test fails
	static_assert(!std::is_copy_constructible_v<siddiqsoft::GenProcessInfo>,
	              "GenProcessInfo should not be copy constructible");
	EXPECT_TRUE(true);
}

TEST(semantics, NoCopyAssignment)
{
	// This test verifies that copy assignment is deleted
	static_assert(!std::is_copy_assignable_v<siddiqsoft::GenProcessInfo>,
	              "GenProcessInfo should not be copy assignable");
	EXPECT_TRUE(true);
}

TEST(semantics, NoMoveConstruction)
{
	// This test verifies that move construction is deleted
	static_assert(!std::is_move_constructible_v<siddiqsoft::GenProcessInfo>,
	              "GenProcessInfo should not be move constructible");
	EXPECT_TRUE(true);
}

TEST(semantics, NoMoveAssignment)
{
	// This test verifies that move assignment is deleted
	static_assert(!std::is_move_assignable_v<siddiqsoft::GenProcessInfo>,
	              "GenProcessInfo should not be move assignable");
	EXPECT_TRUE(true);
}


// ============================================================================
// Legacy Examples (Backward Compatibility)
// ============================================================================

TEST(examples, Example1)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		procInfo.snapshot();
		EXPECT_EQ(siddiqsoft::GenProcessInfo::GetCurrentProcessId(), procInfo.processId);
	}
	catch (...)
	{
		EXPECT_TRUE(false); // if we throw then the test fails.
	}
}

TEST(examples, Example2)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		// We must perform the snapshot to obtain memory and thread usage
		procInfo.snapshot();

		// tip: do not use brace-init/assignment as it will create an array instead of object!
		nlohmann::json info(procInfo);

		std::cerr << info.dump(4) << std::endl;
		EXPECT_EQ(siddiqsoft::GenProcessInfo::GetCurrentProcessId(), info.value("processId", 0));
		std::cerr << std::format("{} - Contents:{}\n", __func__, procInfo);
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << e.what(); // if we throw then the test fails.
	}
}

TEST(examples, Example3)
{
	try
	{
		siddiqsoft::WinProcessInfo procInfo;

		// We must perform the snapshot to obtain memory and thread usage
		procInfo.snapshot();

		std::cerr << nlohmann::json(procInfo).dump(4) << std::endl;
		EXPECT_EQ(siddiqsoft::GenProcessInfo::GetCurrentProcessId(), nlohmann::json(procInfo).value("processId", 0));
		std::cerr << std::format("{} - Contents:{}\n", __func__, procInfo);
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << e.what(); // if we throw then the test fails.
	}
}
