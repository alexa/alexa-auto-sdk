cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(aal LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DUSE_GLOOP)
add_definitions(-DUSE_FAKEMUTE)

# GStreamer
find_package(PkgConfig)
pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.8)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0>=1.8)

add_library(aal STATIC
	src/core.c
	src/player.c
	src/recorder.c
)

target_include_directories(aal
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${GST_INCLUDE_DIRS}
		${GST_APP_INCLUDE_DIRS}
)

target_link_libraries(aal
	${GST_LDFLAGS}
	${GST_APP_LDFLAGS}
)

install(
	TARGETS aal
	DESTINATION lib
)

install(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
	DESTINATION include
	FILES_MATCHING PATTERN "*.h"
)
