/*
    TimeThis : Simple stopwatch implementation with optional callback on destructor
    Version 2

    https://github.com/SiddiqSoft/TimeThis

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

#include <functional>
#include <chrono>
#include <ostream>
#include <format>
#include <source_location>
#include <stdexcept>
#include <type_traits>

namespace siddiqsoft
{
	/// @brief A stopwatch class that measures elapsed time and optionally executes a callback on destruction.
	/// 
	/// This class provides a simple RAII-based timing mechanism. It captures the current time upon
	/// construction and can execute a user-provided callback with the elapsed duration upon destruction.
	/// 
	/// @note Copy and move operations are explicitly deleted to ensure single ownership semantics.
	class timethis
	{
	public:
		/// @brief Type alias for the callback function signature
		using callback_type = std::function<void(const std::chrono::system_clock::duration&)>;

		/// @brief Type alias for duration type
		using duration_type = std::chrono::system_clock::duration;

		/// @brief Type alias for time point type
		using time_point_type = std::chrono::system_clock::time_point;

		/// @brief Default constructor that captures the current time and source location.
		/// 
		/// @param sl Source location information (automatically captured from call site)
		explicit timethis(const std::source_location& sl = std::source_location::current()) noexcept
		    : m_callback()
		    , m_source_location(sl)
		    , m_start_time(std::chrono::system_clock::now())
		{
		}

		/// @brief Constructor that accepts a callback to be executed on destruction.
		/// 
		/// @param callback Function to be called with elapsed duration upon destruction
		/// @param sl Source location information (automatically captured from call site)
		explicit timethis(callback_type&& callback,
		                  const std::source_location& sl = std::source_location::current()) noexcept
		    : m_callback(std::move(callback))
		    , m_source_location(sl)
		    , m_start_time(std::chrono::system_clock::now())
		{
		}

		/// @brief Destructor that invokes the callback if one was provided.
		~timethis() noexcept
		{
			if (m_callback) {
				m_callback(elapsed());
			}
		}

		// Delete copy operations - single ownership semantics
		timethis(const timethis&) = delete;
		timethis& operator=(const timethis&) = delete;

		// Delete move operations - prevents accidental misuse
		timethis(timethis&&) = delete;
		timethis& operator=(timethis&&) = delete;

		/// @brief Calculates the duration since the creation of this object.
		/// 
		/// @return Duration representing the elapsed time
		[[nodiscard]] duration_type elapsed() const noexcept
		{
			return std::chrono::system_clock::now() - m_start_time;
		}

		/// @brief Converts the timing information to a formatted string.
		/// 
		/// @tparam charT Character type for the output string
		/// @return Formatted string containing timing information
		/// @throws std::invalid_argument if charT is wchar_t (not yet implemented)
		template <typename charT = char>
		[[nodiscard]] auto to_string() const
		{
			if constexpr (std::is_same_v<charT, char>) {
				return std::format("{} started on {:%FT%T}Z took {}us",
				                   m_source_location.function_name(),
				                   m_start_time,
				                   std::chrono::duration_cast<std::chrono::microseconds>(elapsed()).count());
			}
			else if constexpr (std::is_same_v<charT, wchar_t>) {
				throw std::invalid_argument("wchar_t formatting not yet implemented");
			}
		}

		/// @brief Stream output operator for convenient printing.
		/// 
		/// @param os Output stream
		/// @param src timethis object to output
		/// @return Reference to the output stream
		friend std::ostream& operator<<(std::ostream& os, const timethis& src)
		{
			os << src.m_source_location.function_name() << " took " << src.elapsed().count() << "ns";
			return os;
		}

	private:
		callback_type m_callback;                    ///< Optional callback to execute on destruction
		std::source_location m_source_location;      ///< Source location where this object was created
		time_point_type m_start_time;                ///< Time point when this object was created
	};

} // namespace siddiqsoft

/// @brief Specialization of std::formatter for timethis objects.
template <class charT>
struct std::formatter<siddiqsoft::timethis, charT> : std::formatter<charT>
{
	/// @brief Format a timethis object using the standard format interface.
	template <class FC>
	auto format(const siddiqsoft::timethis& sv, FC& ctx) const
	{
		return std::format_to(ctx.out(), "{}", sv.to_string<charT>());
	}
};
