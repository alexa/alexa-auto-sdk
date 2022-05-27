cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(aal LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(ENABLE_GSTREAMER)
	find_package(PkgConfig)
	pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.8)
	pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0>=1.8)
	pkg_check_modules(GST_AUDIO REQUIRED gstreamer-audio-1.0>=1.8)

	if(BUNDLE_GST_PLUGINS)
		pkg_check_modules(GST_PLUGINS_BASE REQUIRED gst-plugins-base>=1.8)
		pkg_check_modules(GST_PLUGINS_GOOD REQUIRED gst-plugins-good>=1.8)
		pkg_check_modules(GST_PLUGINS_BAD REQUIRED gst-plugins-bad>=1.8)
	endif()

	add_definitions(-DCONFIG_GSTREAMER)
	add_definitions(-DUSE_GLOOP)
	add_definitions(-DUSE_FAKEMUTE)

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

	# Workaround for https://gitlab.kitware.com/cmake/cmake/-/issues/15746
	string(REPLACE "-framework;" "-framework " GST_LDFLAGS "${GST_LDFLAGS}")
	string(REPLACE "-framework;" "-framework " GST_APP_LDFLAGS "${GST_APP_LDFLAGS}")
	string(REPLACE "-framework;" "-framework " GST_AUDIO_LDFLAGS "${GST_AUDIO_LDFLAGS}")

	list(APPEND AAL_MODULE_LIBRARIES
		${GST_LDFLAGS}
		${GST_APP_LDFLAGS}
		${GST_AUDIO_LDFLAGS}
	)

	if(BUNDLE_GST_PLUGINS)
		string(REPLACE "-framework;" "-framework " GST_PLUGINS_BASE_LDFLAGS "${GST_PLUGINS_BASE_LDFLAGS}")
		string(REPLACE "-framework;" "-framework " GST_PLUGINS_GOOD_LDFLAGS "${GST_PLUGINS_GOOD_LDFLAGS}")
		string(REPLACE "-framework;" "-framework " GST_PLUGINS_BAD_LDFLAGS "${GST_PLUGINS_BAD_LDFLAGS}")

		list(APPEND AAL_MODULE_LIBRARIES
			${GST_PLUGINS_BASE_LDFLAGS}
			${GST_PLUGINS_GOOD_LDFLAGS}
			${GST_PLUGINS_BAD_LDFLAGS}
		)
		if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			list(APPEND AAL_MODULE_LIBRARIES
				"-framework AudioUnit"
				"-framework CoreAudio"
				"-framework AudioToolbox"
			)
		endif()
	endif()
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

if(BUNDLE_GST_PLUGINS)
	add_definitions(-DBUNDLE_GST_PLUGINS)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Android")
	list(APPEND AAL_MODULE_LIBRARIES log)
endif()

if(AAC_ENABLE_TESTS)
add_subdirectory(src/test)
endif()

add_library(aal SHARED
	src/common.c
	${AAL_MODULE_SRC}
)

set_target_properties(aal PROPERTIES SOVERSION 1)
set_target_properties(aal PROPERTIES OUTPUT_NAME "SystemAudio")

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
		$<$<CONFIG:Release>:-Wl,-s> # Strip symbols
		$<$<PLATFORM_ID:Linux>:-Wl,-z,relro,-z,now> # Full RELRO
		$<$<PLATFORM_ID:Android>:-Wl,-z,relro,-z,now> # Full RELRO
		$<$<PLATFORM_ID:QNX>:-Wl,-z,relro,-z,now> # Full RELRO
		$<$<PLATFORM_ID:Linux>:-Wl,-z,noexecstack>
		$<$<PLATFORM_ID:Android>:-Wl,-z,noexecstack>
		$<$<PLATFORM_ID:QNX>:-Wl,-z,noexecstack>
)

target_compile_options(aal
	PRIVATE
		# stack protection
		-fstack-protector-strong

		# string format
		-Wformat -Wformat-security -Werror=format-security

		# _FORTIFY_SOURCE requires -O1 or higher, so enable it only for release build
		$<$<CONFIG:Release>:-D_FORTIFY_SOURCE=2>
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
