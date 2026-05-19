#   Generic Process Information
#   Copyright 2024 Abdulkareem Siddiq. All rights reserved.
#   See LICENSE file
#
#   When building from our CI server, the following options must be set:
#   - genericprocinfo_BUILD_TESTS=ON
#   - CI_BUILDID must be set to the gitversion
# 
cmake_minimum_required(VERSION 3.29)

# Version detection must precede project() declaration
# Figures out ${CURRENT_PROJECT_VERSION} and brings in CPM module
include(pack/CMakeCommonHelpers.cmake)

# Project metadata
project(GenProcessInfo
        VERSION         "${CURRENT_PROJECT_VERSION}"
        LANGUAGES       CXX
        HOMEPAGE_URL    "%%myurl%%"
        DESCRIPTION     "Provides for some basic process information")

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(${PROJECT_NAME}_BUILD_TESTS "Build tests for ${PROJECT_NAME}" OFF)
option(CI_BUILDID "Build ID for CI builds" "0.0.0")

# Library definition
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Include directories
target_include_directories(${PROJECT_NAME}
    INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

# Compile features and options
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

# Clang-specific options for experimental library features
target_compile_options(${PROJECT_NAME}
    INTERFACE
    $<$<CXX_COMPILER_ID:Clang>:-fexperimental-library>)


# Testing configuration
if(${PROJECT_NAME}_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests" AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    message(STATUS "${PROJECT_NAME}: Building tests (${PROJECT_NAME}_BUILD_TESTS=ON)")
    enable_testing()
    add_subdirectory(tests)
endif()
