# gtest

find_package(Threads REQUIRED)

# Enable ExternalProject CMake module
INCLUDE(ExternalProject)

ExternalProject_Add(
    gtest
    URL https://github.com/google/googletest/archive/release-1.8.1.zip
    SOURCE_DIR "${CMAKE_BINARY_DIR}/gtest-src"
    BINARY_DIR "${CMAKE_BINARY_DIR}/gtest-build"
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
ADD_LIBRARY(libgtest INTERFACE)
TARGET_INCLUDE_DIRECTORIES(libgtest
    INTERFACE
        ${source_dir}/googletest/include
)
TARGET_LINK_LIBRARIES(libgtest
    INTERFACE
        ${binary_dir}/googlemock/gtest/libgtest.a
        ${CMAKE_THREAD_LIBS_INIT}
)

# Create a libgmock target to be used as a dependency by test programs
ADD_LIBRARY(libgmock INTERFACE)
TARGET_INCLUDE_DIRECTORIES(libgmock
    INTERFACE
        ${source_dir}/googlemock/include
)
TARGET_LINK_LIBRARIES(libgmock
    INTERFACE
        ${binary_dir}/googlemock/libgmock.a
        ${CMAKE_THREAD_LIBS_INIT}
)

SET(GTEST_ROOT ${source_dir}/googletest)
message(STATUS ${GTEST_ROOT})