cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(aal LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DUSE_UDPSRC -DUSE_UDPSINK")

if(ENABLE_GSTREAMER)
	find_package(PkgConfig)
	pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.8)
	pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0>=1.8)
	pkg_check_modules(GST_AUDIO REQUIRED gstreamer-audio-1.0>=1.8)

	add_definitions(-DCONFIG_GSTREAMER)
	add_definitions(-DUSE_GLOOP)
	add_definitions(-DUSE_FAKEMUTE)
	if (USE_PIPEWIRE)
		add_definitions(-DUSE_PIPEWIRE)
		message(STATUS "Pipewire enabled")
	endif()

	list(APPEND AAL_MODULE_SRC
		src/gstreamer/core.c
		src/gstreamer/player.c
		src/gstreamer/recorder.c
		src/gstreamer/mathstubs.c
	)

	# It is necessary to prepend include dirs of gstreamer libraries with CONAN_CMAKE_FIND_ROOT_PATH.
	# Otherwise the header files on the host rather than the ones in in toolchain will be included.
	if(DEFINED CONAN_CMAKE_FIND_ROOT_PATH)
		list(TRANSFORM GST_INCLUDE_DIRS PREPEND ${CONAN_CMAKE_FIND_ROOT_PATH})
		list(TRANSFORM GST_APP_INCLUDE_DIRS PREPEND ${CONAN_CMAKE_FIND_ROOT_PATH})
		list(TRANSFORM GST_AUDIO_INCLUDE_DIRS PREPEND ${CONAN_CMAKE_FIND_ROOT_PATH})
	endif()

	list(APPEND AAL_MODULE_INCLUDE_DIRS
		${GST_INCLUDE_DIRS}
		${GST_APP_INCLUDE_DIRS}
		${GST_AUDIO_INCLUDE_DIRS}
	)
	list(APPEND AAL_MODULE_LIBRARIES
		${GST_LDFLAGS}
		${GST_APP_LDFLAGS}
		${GST_AUDIO_LDFLAGS}
	)
endif()

if(ENABLE_OMXAL)
	add_definitions(-DCONFIG_OMXAL)
	list(APPEND AAL_MODULE_SRC
		src/omxal/core.c
		src/omxal/player.c
	)
	list(APPEND AAL_MODULE_LIBRARIES
		OpenMAXAL
		m
	)
endif()

if(ENABLE_QSA)
	add_definitions(-DCONFIG_QSA)
	list(APPEND AAL_MODULE_SRC
		src/qsa/core.c
		src/qsa/player.c
		src/qsa/recorder.c
		lib/c-ringbuf/ringbuf.c
	)
	list(APPEND AAL_MODULE_INCLUDE_DIRS
		${PROJECT_SOURCE_DIR}/lib/c-ringbuf
	)
	list(APPEND AAL_MODULE_LIBRARIES
		asound
	)
endif()

if (BUNDLE_GST_PLUGINS)
	add_definitions(-DBUNDLE_GST_PLUGINS)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Android")
	list(APPEND AAL_MODULE_LIBRARIES log)
endif()

if(AAC_ENABLE_TESTS)
add_subdirectory(src/test)
endif()

add_library(aal STATIC
	src/common.c
	${AAL_MODULE_SRC}
)

target_include_directories(aal
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${AAL_MODULE_INCLUDE_DIRS}
)

target_link_libraries(aal
	PRIVATE
		${AAL_MODULE_LIBRARIES}
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
