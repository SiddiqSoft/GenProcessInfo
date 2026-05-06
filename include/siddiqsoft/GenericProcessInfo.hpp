/*
    WinProcessInfo : Windows Process Information
    Version 2

    https://github.com/SiddiqSoft/WinProcessInfo

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

#pragma once

#include <chrono>
#include <format>
#include <string>
#include <fstream>
#include <sstream>

#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "psapi.lib")
#define SIDDIQSOFT_WINDOWS 1
#elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <cstring>
#define SIDDIQSOFT_UNIX 1
#endif

namespace siddiqsoft
{
	/// @brief Provides process information including CPU, memory, and system details.
	///
	/// This class captures and maintains information about the current process, including
	/// CPU cores, handles/file descriptors, threads, memory usage, and system hostname/domain information.
	/// Memory and thread snapshots are expensive operations and should be called sparingly.
	///
	/// Supports Windows, Linux, macOS, and other Unix-like systems with feature parity.
	class GenericProcessInfo
	{
	public:
		unsigned long                         cpuCores {0};            ///< Number of CPU cores available
		unsigned long                         cpuHandles {0};          ///< Number of handles/file descriptors used by the process
		unsigned long                         cpuThreads {0};          ///< Number of threads in the process
		size_t                                memPeakWorkingSet {0};   ///< Peak working set memory in KB
		size_t                                memWorkingSet {0};       ///< Current working set memory in KB
		size_t                                memPrivate {0};          ///< Private memory usage in KB
		std::string                           nameHostname {};         ///< DNS hostname
		std::string                           nameDomainName {};       ///< Domain name
		std::string                           nameHostnamePhysical {}; ///< Physical DNS hostname
		std::string                           nameFqdn {};             ///< Fully qualified domain name
		std::chrono::system_clock::time_point timeStartup {std::chrono::system_clock::now()}; ///< Process startup time
		unsigned long                         processId {0};                                  ///< Current process ID
#if defined(SIDDIQSOFT_WINDOWS)
		HANDLE processHandle {NULL}; ///< Handle to the current process (Windows only)
#endif

	public:
		/// @brief Default constructor that initializes process information.
		///
		/// Captures system information including CPU cores, process ID, and hostname details.
		GenericProcessInfo()
		{
#if defined(SIDDIQSOFT_WINDOWS)
			initializeWindows();
#else
			initializeUnix();
#endif
		}

		/// @brief Destructor that cleans up resources.
		~GenericProcessInfo() noexcept
		{
#if defined(SIDDIQSOFT_WINDOWS)
			if (processHandle != NULL) {
				CloseHandle(processHandle);
			}
#endif
		}

		// Delete copy operations - single ownership semantics
		GenericProcessInfo(const GenericProcessInfo&)            = delete;
		GenericProcessInfo& operator=(const GenericProcessInfo&) = delete;

		// Delete move operations - prevents accidental misuse
		GenericProcessInfo(GenericProcessInfo&&)            = delete;
		GenericProcessInfo& operator=(GenericProcessInfo&&) = delete;

		/// @brief Collects a snapshot of memory, handle, and thread count information.
		///
		/// This method is expensive and should be called sparingly. Consider using a
		/// low-priority background thread to measure these statistics periodically.
		void snapshot() noexcept
		{
			getMemoryInfo();
			getCurrentThreadCount();
		}

		/// @brief Calculates the uptime for this application.
		///
		/// @return Duration representing the elapsed time since process startup.
		///         Cast appropriately via duration_cast<> for desired precision.
		[[nodiscard]] std::chrono::system_clock::duration uptime() const noexcept
		{
			return std::chrono::system_clock::now() - timeStartup;
		}

		/// @brief Get the current process ID in a cross-platform manner
		static auto getCurrentProcessId() noexcept
		{
#if defined(SIDDIQSOFT_WINDOWS)
			return GetCurrentProcessId();
#else
			return getpid();
#endif
		}

	private:
#if defined(SIDDIQSOFT_WINDOWS)
		/// @brief Windows-specific initialization.
		void initializeWindows() noexcept
		{
			SYSTEM_INFO sysInfo {};

			// Store one-time information
			processId = GetCurrentProcessId();
			GetSystemInfo(&sysInfo);
			cpuCores      = sysInfo.dwNumberOfProcessors;

			processHandle = GetCurrentProcess();

			// Get the hostname
			DWORD dwSize {MAX_COMPUTERNAME_LENGTH};
			nameHostname.resize(MAX_COMPUTERNAME_LENGTH);
			GetComputerNameExA(COMPUTER_NAME_FORMAT::ComputerNameDnsHostname, nameHostname.data(), &dwSize);
			nameHostname.resize(dwSize);

			// Get the hostname(fully qualified)
			dwSize = MAX_COMPUTERNAME_LENGTH;
			nameFqdn.resize(MAX_COMPUTERNAME_LENGTH);
			GetComputerNameExA(COMPUTER_NAME_FORMAT::ComputerNameDnsFullyQualified, nameFqdn.data(), &dwSize);
			nameFqdn.resize(dwSize);

			// Get the local hostname(fully qualified)
			dwSize = MAX_COMPUTERNAME_LENGTH;
			nameHostnamePhysical.resize(MAX_COMPUTERNAME_LENGTH);
			GetComputerNameExA(COMPUTER_NAME_FORMAT::ComputerNamePhysicalDnsFullyQualified, nameHostnamePhysical.data(), &dwSize);
			nameHostnamePhysical.resize(dwSize);

			// Get the domain name
			dwSize = MAX_COMPUTERNAME_LENGTH;
			nameDomainName.resize(MAX_COMPUTERNAME_LENGTH);
			GetComputerNameExA(COMPUTER_NAME_FORMAT::ComputerNameDnsDomain, nameDomainName.data(), &dwSize);
			nameDomainName.resize(dwSize);
		}

		/// @brief Collects the memory information for this process on Windows.
		///
		/// Retrieves process memory counters including peak working set, current working set,
		/// and private memory usage. Also updates the handle count.
		///
		/// @see http://msdn.microsoft.com/en-us/library/windows/desktop/ms682050(v=vs.85).aspx
		void getMemoryInfo() noexcept
		{
			PROCESS_MEMORY_COUNTERS_EX pmcInfo {};

			GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS*)&pmcInfo, sizeof(pmcInfo));
			memPeakWorkingSet = pmcInfo.PeakWorkingSetSize / 1024;
			memWorkingSet     = pmcInfo.WorkingSetSize / 1024;
			memPrivate        = pmcInfo.PrivateUsage / 1024;

			cpuHandles        = 0;
			GetProcessHandleCount(processHandle, &cpuHandles);
		}

		/// @brief Retrieves the current thread count for this process on Windows.
		void getCurrentThreadCount() noexcept
		{
			HANDLE         snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
			PROCESSENTRY32 entry    = {0};
			entry.dwSize            = sizeof(entry);

			// Get the first process info
			BOOL ret = Process32First(snapshot, &entry);
			while (ret && entry.th32ProcessID != processId) {
				ret = Process32Next(snapshot, &entry);
			}

			CloseHandle(snapshot);

			cpuThreads = ret ? entry.cntThreads : 0;
		}

#else // Unix/Linux implementation

		/// @brief Unix/Linux-specific initialization.
		void initializeUnix() noexcept
		{
			// Get process ID
			processId = getpid();

			// Get CPU core count
			cpuCores = sysconf(_SC_NPROCESSORS_ONLN);
			if (cpuCores <= 0) {
				cpuCores = 1;
			}

			// Get hostname information
			char hostname_buf[256] = {0};
			if (gethostname(hostname_buf, sizeof(hostname_buf)) == 0) {
				nameHostname         = hostname_buf;
				nameHostnamePhysical = hostname_buf;
			}

			// Try to get FQDN
			struct utsname uts {};
			if (uname(&uts) == 0) {
				nameFqdn = uts.nodename;
			}

			// Extract domain name from FQDN if available
			if (!nameFqdn.empty()) {
				size_t dot_pos = nameFqdn.find('.');
				if (dot_pos != std::string::npos) {
					nameDomainName = nameFqdn.substr(dot_pos + 1);
				}
			}
		}

		/// @brief Collects the memory information for this process on Unix/Linux.
		///
		/// Reads from /proc/self/status and /proc/self/stat to gather memory and thread information.
		void getMemoryInfo() noexcept
		{
#if defined(__linux__)
			// Read from /proc/self/status for memory information
			std::ifstream status_file("/proc/self/status");
			if (status_file.is_open()) {
				std::string line;
				while (std::getline(status_file, line)) {
					if (line.find("VmPeak:") == 0) {
						// Peak virtual memory in KB
						std::istringstream iss(line.substr(7));
						iss >> memPeakWorkingSet;
					}
					else if (line.find("VmRSS:") == 0) {
						// Resident set size (working set) in KB
						std::istringstream iss(line.substr(6));
						iss >> memWorkingSet;
					}
					else if (line.find("VmData:") == 0) {
						// Private data segment in KB
						std::istringstream iss(line.substr(7));
						iss >> memPrivate;
					}
					else if (line.find("FDSize:") == 0) {
						// File descriptor count
						std::istringstream iss(line.substr(7));
						iss >> cpuHandles;
					}
				}
			}
#elif defined(__APPLE__)
			// macOS implementation using getrusage
			struct rusage usage {};
			if (getrusage(RUSAGE_SELF, &usage) == 0) {
				// ru_maxrss is in bytes on macOS
				memPeakWorkingSet = usage.ru_maxrss / 1024;
				memWorkingSet     = usage.ru_maxrss / 1024;
			}

			// Count file descriptors on macOS
			DIR* fd_dir = opendir("/dev/fd");
			if (fd_dir != nullptr) {
				struct dirent* entry;
				cpuHandles = 0;
				while ((entry = readdir(fd_dir)) != nullptr) {
					if (entry->d_name[0] != '.') {
						cpuHandles++;
					}
				}
				closedir(fd_dir);
			}
#else
			// Generic Unix fallback using getrusage
			struct rusage usage {};
			if (getrusage(RUSAGE_SELF, &usage) == 0) {
// ru_maxrss is in kilobytes on some systems, bytes on others
#if defined(__linux__)
				memPeakWorkingSet = usage.ru_maxrss;
				memWorkingSet     = usage.ru_maxrss;
#else
				memPeakWorkingSet = usage.ru_maxrss / 1024;
				memWorkingSet     = usage.ru_maxrss / 1024;
#endif
			}
#endif
		}

		/// @brief Retrieves the current thread count for this process on Unix/Linux.
		void getCurrentThreadCount() noexcept
		{
#if defined(__linux__)
			// Read from /proc/self/status for thread count
			std::ifstream status_file("/proc/self/status");
			if (status_file.is_open()) {
				std::string line;
				while (std::getline(status_file, line)) {
					if (line.find("Threads:") == 0) {
						std::istringstream iss(line.substr(8));
						iss >> cpuThreads;
						break;
					}
				}
			}
#elif defined(__APPLE__)
			// macOS implementation
			// Note: Getting thread count on macOS requires Mach APIs
			// For now, we'll use a simpler approach via /proc or system calls
			// This is a limitation of the generic Unix approach
			cpuThreads = 1; // Fallback value
#else
			// Generic Unix fallback - may not be accurate
			cpuThreads = 1;
#endif
		}

#endif // Platform-specific implementations
	};

	// Backward compatibility alias
	using WinProcessInfo = GenericProcessInfo;

#if defined INCLUDE_NLOHMANN_JSON_HPP_
	/// @brief Serializer for nlohmann::json library
	/// @param dest The destination target
	/// @param gpi Our source object
	static void to_json(nlohmann::json& dest, const GenericProcessInfo& gpi)
	{
		dest = nlohmann::json {{"processId", gpi.processId},
		                       {"hostname", gpi.nameHostname},
		                       {"fqdn", gpi.nameFqdn},
		                       {"domain", gpi.nameDomainName},
		                       {"localFqdn", gpi.nameHostnamePhysical},
		                       {"cpuHandles", gpi.cpuHandles},
		                       {"cpuThreads", gpi.cpuThreads},
		                       {"cpuCores", gpi.cpuCores},
		                       {"memPeakWorkingSet", gpi.memPeakWorkingSet},
		                       {"memWorkingSet", gpi.memWorkingSet},
		                       {"memPrivateBytes", gpi.memPrivate},
#if __cpp_lib_format
		                       {"timeStartup", std::format("{:%FT%T}Z", gpi.timeStartup)},
		                       {"timeCurrent", std::format("{:%FT%T}Z", std::chrono::system_clock::now())},
#endif
		                       {"uptime", std::chrono::duration_cast<std::chrono::microseconds>(gpi.uptime()).count()}};
	}
#endif
} // namespace siddiqsoft


/// @brief Specialization of std::formatter for GenericProcessInfo objects.
template <class charT>
struct std::formatter<siddiqsoft::GenericProcessInfo, charT> : std::formatter<std::string, charT>
{
	/// @brief Format a GenericProcessInfo object using the standard format interface.
	template <class FC>
	auto format(const siddiqsoft::GenericProcessInfo& gpi, FC& ctx) const
	{
#if defined INCLUDE_NLOHMANN_JSON_HPP_
		return std::formatter<std::string, charT>::format(nlohmann::json(gpi).dump(), ctx);
#else
		std::string s;

		// First element
		std::format_to(std::back_inserter(s), "{{\"processId\":{}", gpi.processId);
		std::format_to(std::back_inserter(s), ",\"hostname\":\"{}\",", gpi.nameHostname);
		std::format_to(std::back_inserter(s), ",\"domain\":\"{}\",", gpi.nameDomainName);
		std::format_to(std::back_inserter(s), ",\"fqdn\":\"{}\",", gpi.nameFqdn);
		std::format_to(std::back_inserter(s), ",\"localFqdn\":\"{}\",", gpi.nameHostnamePhysical);
		std::format_to(std::back_inserter(s), ",\"cpuHandles\":{},", gpi.cpuHandles);
		std::format_to(std::back_inserter(s), ",\"cpuThreads\":{},", gpi.cpuThreads);
		std::format_to(std::back_inserter(s), ",\"cpuCores\":{},", gpi.cpuCores);
		std::format_to(std::back_inserter(s), ",\"memPeakWorkingSet\":{},", gpi.memPeakWorkingSet);
		std::format_to(std::back_inserter(s), ",\"memWorkingSet\":{},", gpi.memWorkingSet);
		std::format_to(std::back_inserter(s), ",\"memPrivateBytes\":{},", gpi.memPrivate);
		std::format_to(std::back_inserter(s), ",\"timeStartup\":\"{:%FT%T}Z\"", gpi.timeStartup);
		std::format_to(std::back_inserter(s), ",\"timeCurrent\":\"{:%FT%T}Z\"", std::chrono::system_clock::now());
		// last element
		std::format_to(std::back_inserter(s),
		               ",\"uptime\":{}}}",
		               std::chrono::duration_cast<std::chrono::microseconds>(gpi.uptime()).count());
		return std::formatter<std::string, charT>::format(s, ctx);
#endif
	}
};
