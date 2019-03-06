###########################################################################
# Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###########################################################################

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