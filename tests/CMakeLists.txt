if(${PROJECT_NAME}_BUILD_TESTS)
    set(TEST_TARGET ${PROJECT_NAME}_tests)

    # Create test executable
    add_executable(${TEST_TARGET})

    # Compile features
    target_compile_features(${TEST_TARGET} PRIVATE cxx_std_23)

    # Compiler-specific options
    target_compile_options(${TEST_TARGET}
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/std:c++latest>
        $<$<CXX_COMPILER_ID:Clang>:-fexperimental-library>)

    # Debug-specific options
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        # Add debug definitions
        target_compile_definitions(${TEST_TARGET} PRIVATE DEBUG=1 _DEBUG=1)
    endif()

    # Test sources
    target_sources(${TEST_TARGET}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

    # Google Test dependency
    CPMAddPackage("gh:google/googletest#v1.17.0")
    # Common dependencies
    CPMAddPackage("gh:nlohmann/json#v3.12.0")
    target_link_libraries(${PROJECT_NAME} INTERFACE nlohmann_json::nlohmann_json)

    # Link libraries
    target_link_libraries(${TEST_TARGET}
        PRIVATE
        GTest::gtest_main
        ${PROJECT_NAME}::${PROJECT_NAME}
        nlohmann_json::nlohmann_json
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:CURL::libcurl>
        $<$<CXX_COMPILER_ID:MSVC>:acw32h::acw32h>
    )

    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(${TEST_TARGET} XML_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/results")

    include(CTest)
    message(STATUS "${PROJECT_NAME}: Test configuration complete (${TEST_TARGET})")
endif()
