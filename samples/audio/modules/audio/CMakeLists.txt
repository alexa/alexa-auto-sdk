cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(AACEAudio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#add_definitions(-DAACE_DEBUG_LOG_ENABLED)

if(AAC_HOME)
	include(${AAC_HOME}/share/cmake/AACECore.cmake)
	include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
endif()

install(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
	DESTINATION include
	FILES_MATCHING PATTERN "*.h"
)

add_subdirectory(platforms)

install(
	EXPORT AACEAudio
	DESTINATION share/cmake
)