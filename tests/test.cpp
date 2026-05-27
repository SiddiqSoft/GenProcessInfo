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
#include <chrono>
#include <format>
#include <mutex>

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
		EXPECT_EQ(siddiqsoft::GenProcessInfo::GetProcessId(), procInfo.processId);
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
		EXPECT_EQ(procInfo.processId, siddiqsoft::GenProcessInfo::GetProcessId());
		EXPECT_GT(procInfo.processId, 0);

		std::cerr << "Process ID: " << procInfo.processId << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Process ID initialization should not throw";
	}
}

TEST(initialization, PlatformProperties)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		// Verify platform name is populated
		EXPECT_FALSE(procInfo.platformName.empty());
		EXPECT_GT(procInfo.platformName.length(), 0);

		// Verify architecture is populated
		EXPECT_FALSE(procInfo.platformArchitecture.empty());
		EXPECT_GT(procInfo.platformArchitecture.length(), 0);

		// Verify platform name is one of the expected values
		EXPECT_TRUE(procInfo.platformName == "Win32" || 
		            procInfo.platformName == "Win64" || 
		            procInfo.platformName == "Linux" || 
		            procInfo.platformName == "MacOS" || 
		            procInfo.platformName == "UNIX" || 
		            procInfo.platformName == "Unknown");

		// Verify architecture is one of the expected values
		EXPECT_TRUE(procInfo.platformArchitecture == "AMD64" || 
		            procInfo.platformArchitecture == "X86" || 
		            procInfo.platformArchitecture == "ARM64" || 
		            procInfo.platformArchitecture == "ARM" || 
		            procInfo.platformArchitecture == "Unknown");

		std::cerr << "Platform: " << procInfo.platformName << std::endl;
		std::cerr << "Architecture: " << procInfo.platformArchitecture << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Platform initialization should not throw";
	}
}

TEST(initialization, PlatformOSDetails)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;

		// Verify platformOSName is populated (may be empty on Windows)
		EXPECT_GE(procInfo.platformOSName.length(), 0);

		// Verify platformRelease is populated (may be empty on Windows)
		EXPECT_GE(procInfo.platformRelease.length(), 0);

		// Verify platformVersion is populated (may be empty on Windows)
		EXPECT_GE(procInfo.platformVersion.length(), 0);

		// On Unix/Linux systems, these should typically be populated
#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
		EXPECT_FALSE(procInfo.platformOSName.empty()) << "platformOSName should be populated on Unix/Linux";
		EXPECT_FALSE(procInfo.platformRelease.empty()) << "platformRelease should be populated on Unix/Linux";
		EXPECT_FALSE(procInfo.platformVersion.empty()) << "platformVersion should be populated on Unix/Linux";
#endif

		std::cerr << "OS Name: " << procInfo.platformOSName << std::endl;
		std::cerr << "Release: " << procInfo.platformRelease << std::endl;
		std::cerr << "Version: " << procInfo.platformVersion << std::endl;
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Platform OS details initialization should not throw";
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

		// Verify all expected top-level nested objects are present
		EXPECT_TRUE(info.contains("process"));
		EXPECT_TRUE(info.contains("host"));
		EXPECT_TRUE(info.contains("platform"));
		EXPECT_TRUE(info.contains("timeStartup"));
		EXPECT_TRUE(info.contains("timeCurrent"));
		EXPECT_TRUE(info.contains("uptime"));

		// Verify process is a nested object with expected fields
		EXPECT_TRUE(info["process"].is_object());
		EXPECT_TRUE(info["process"].contains("processId"));
		EXPECT_TRUE(info["process"].contains("cpuHandles"));
		EXPECT_TRUE(info["process"].contains("cpuThreads"));
		EXPECT_TRUE(info["process"].contains("cpuCores"));
		EXPECT_TRUE(info["process"].contains("memPeakWorkingSet"));
		EXPECT_TRUE(info["process"].contains("memWorkingSet"));
		EXPECT_TRUE(info["process"].contains("memPrivateBytes"));

		// Verify host is a nested object with expected fields
		EXPECT_TRUE(info["host"].is_object());
		EXPECT_TRUE(info["host"].contains("hostname"));
		EXPECT_TRUE(info["host"].contains("fqdn"));
		EXPECT_TRUE(info["host"].contains("domain"));
		EXPECT_TRUE(info["host"].contains("localFqdn"));

		// Verify platform is a nested object with expected fields
		EXPECT_TRUE(info["platform"].is_object());
		EXPECT_TRUE(info["platform"].contains("platform"));
		EXPECT_TRUE(info["platform"].contains("architecture"));
		EXPECT_TRUE(info["platform"].contains("name"));
		EXPECT_TRUE(info["platform"].contains("version"));
		EXPECT_TRUE(info["platform"].contains("release"));

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

		// Verify field values in nested objects
		EXPECT_EQ(info["process"].value("processId", 0), siddiqsoft::GenProcessInfo::GetProcessId());
		EXPECT_EQ(info["host"].value("hostname", ""), procInfo.nameHostname);
		EXPECT_EQ(info["host"].value("fqdn", ""), procInfo.nameFqdn);
		EXPECT_EQ(info["process"].value("cpuCores", 0), procInfo.cpuCores);
		EXPECT_EQ(info["process"].value("cpuThreads", 0), procInfo.cpuThreads);
		EXPECT_EQ(info["process"].value("memWorkingSet", 0), procInfo.memWorkingSet);

		std::cerr << "JSON field values verified" << std::endl;
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << "JSON field verification should not throw: " << e.what();
	}
}

TEST(json, JSONPlatformAndArchitecture)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;
		procInfo.snapshot();

		nlohmann::json info(procInfo);

		// Verify platform is a nested object
		EXPECT_TRUE(info["platform"].is_object());

		// Verify platform and architecture fields are present and match
		EXPECT_EQ(info["platform"].value("platform", ""), procInfo.platformName);
		EXPECT_EQ(info["platform"].value("architecture", ""), procInfo.platformArchitecture);

		// Verify they are not empty
		EXPECT_FALSE(info["platform"].value("platform", "").empty());
		EXPECT_FALSE(info["platform"].value("architecture", "").empty());

		// Verify they contain valid values
		std::string platform = info["platform"].value("platform", "");
		std::string architecture = info["platform"].value("architecture", "");

		EXPECT_TRUE(platform == "Win32" || platform == "Win64" || platform == "Linux" || 
		            platform == "MacOS" || platform == "UNIX" || platform == "Unknown");
		EXPECT_TRUE(architecture == "AMD64" || architecture == "X86" || architecture == "ARM64" || 
		            architecture == "ARM" || architecture == "Unknown");

		std::cerr << "Platform: " << platform << ", Architecture: " << architecture << std::endl;
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << "JSON platform/architecture verification should not throw: " << e.what();
	}
}

TEST(json, JSONOSDetails)
{
	try
	{
		siddiqsoft::GenProcessInfo procInfo;
		procInfo.snapshot();

		nlohmann::json info(procInfo);

		// Verify platform is a nested object
		EXPECT_TRUE(info["platform"].is_object());

		// Verify OS detail fields are present in the nested platform object
		EXPECT_TRUE(info["platform"].contains("name"));
		EXPECT_TRUE(info["platform"].contains("release"));
		EXPECT_TRUE(info["platform"].contains("version"));

		// Verify field values match the object members
		EXPECT_EQ(info["platform"].value("name", ""), procInfo.platformOSName);
		EXPECT_EQ(info["platform"].value("release", ""), procInfo.platformRelease);
		EXPECT_EQ(info["platform"].value("version", ""), procInfo.platformVersion);

		// On Unix/Linux systems, these should be populated
#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
		EXPECT_FALSE(procInfo.platformOSName.empty()) << "platformOSName should be populated on Unix/Linux";
		EXPECT_FALSE(procInfo.platformRelease.empty()) << "platformRelease should be populated on Unix/Linux";
		EXPECT_FALSE(procInfo.platformVersion.empty()) << "platformVersion should be populated on Unix/Linux";
#endif

		std::cerr << "OS Name: " << procInfo.platformOSName << std::endl;
		std::cerr << "Release: " << procInfo.platformRelease << std::endl;
		std::cerr << "Version: " << procInfo.platformVersion << std::endl;
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << "JSON OS details verification should not throw: " << e.what();
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
// Thread ID Tests
// ============================================================================

TEST(threadId, GetCurrentThreadId)
{
	try
	{
		auto threadId = siddiqsoft::GenProcessInfo::GetThreadId();

		// Verify thread ID is valid (non-zero/non-null)
#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
		EXPECT_NE(threadId, 0);
		std::cerr << "Current Thread ID (Windows): " << threadId << std::endl;
#else
		// On Unix/Linux, std::this_thread::id() is returned
		std::cerr << "Current Thread ID (Unix/Linux): " << threadId << std::endl;
		EXPECT_TRUE(true); // Just verify it doesn't throw
#endif
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "GetThreadId should not throw";
	}
}

TEST(threadId, ThreadIdConsistency)
{
	try
	{
		auto threadId1 = siddiqsoft::GenProcessInfo::GetThreadId();
		auto threadId2 = siddiqsoft::GenProcessInfo::GetThreadId();

		// Thread ID should be consistent within the same thread
#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
		EXPECT_EQ(threadId1, threadId2);
		std::cerr << "Thread ID consistency verified: " << threadId1 << std::endl;
#else
		// On Unix/Linux, compare std::this_thread::id() objects
		EXPECT_EQ(threadId1, threadId2);
		std::cerr << "Thread ID consistency verified" << std::endl;
#endif
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Thread ID consistency check should not throw";
	}
}

TEST(threadId, MultipleThreadIds)
{
	try
	{
		auto mainThreadId = siddiqsoft::GenProcessInfo::GetThreadId();
		std::vector<decltype(mainThreadId)> threadIds;
		std::mutex threadIdsMutex;

		// Create a few threads and collect their IDs
		std::vector<std::thread> threads;
		for (int i = 0; i < 3; ++i) {
			threads.emplace_back([&threadIds, &threadIdsMutex]() {
				// Get the thread ID for this spawned thread
				auto myThreadId = siddiqsoft::GenProcessInfo::GetThreadId();
				
				// Use mutex to safely add to the vector
				{
					std::lock_guard<std::mutex> lock(threadIdsMutex);
					threadIds.push_back(myThreadId);
					std::cerr << "Spawned thread ID: " << myThreadId << std::endl;
				}
			});
		}

		// Wait for all threads to complete
		for (auto& t : threads) {
			t.join();
		}
		
		// Verify we collected thread IDs from the spawned threads
		EXPECT_EQ(threadIds.size(), 3);

#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
		// On Windows, verify that at least some thread IDs are different from main thread
		bool hasDifferentThreadId = false;
		for (const auto& tid : threadIds) {
			if (tid != mainThreadId) {
				hasDifferentThreadId = true;
				break;
			}
		}
		EXPECT_TRUE(hasDifferentThreadId) << "Spawned threads should have different thread IDs";
		std::cerr << "Main Thread ID: " << mainThreadId << std::endl;
		for (size_t i = 0; i < threadIds.size(); ++i) {
			std::cerr << "Spawned Thread " << i << " ID: " << threadIds[i] << std::endl;
		}
#else
		// On Unix/Linux, just verify we got thread IDs
		std::cerr << "Collected " << threadIds.size() << " thread IDs from spawned threads" << std::endl;
#endif
	}
	catch (...)
	{
		EXPECT_TRUE(false) << "Multiple thread ID collection should not throw";
	}
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
		EXPECT_EQ(siddiqsoft::GenProcessInfo::GetProcessId(), procInfo.processId);
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
		EXPECT_EQ(siddiqsoft::GenProcessInfo::GetProcessId(), info["process"].value("processId", 0));
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
		EXPECT_EQ(siddiqsoft::GenProcessInfo::GetProcessId(), nlohmann::json(procInfo)["process"].value("processId", 0));
		std::cerr << std::format("{} - Contents:{}\n", __func__, procInfo);
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << e.what(); // if we throw then the test fails.
	}
}
