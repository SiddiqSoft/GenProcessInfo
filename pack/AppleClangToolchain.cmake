# Apple Clang Toolchain for LLVM 22+
# 
# This toolchain file configures CMake to use LLVM's Clang compiler on macOS
# with proper header search paths for libc++ to avoid conflicts with Xcode SDK headers.
#
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=pack/AppleClangToolchain.cmake ...

# Compiler configuration
set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")

# Disable Xcode SDK to avoid header conflicts with libc++
set(CMAKE_OSX_SYSROOT "")

# Use libc++ standard library
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

# Use system root for includes to prevent Xcode SDK conflicts
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isysroot /")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot /")
