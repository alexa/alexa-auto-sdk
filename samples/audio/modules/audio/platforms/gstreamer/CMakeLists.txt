add_definitions(-DUSE_GLOOP)
#add_definitions(-DUSE_DUCKING)

# GStreamer
find_package(PkgConfig)
pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.8)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0>=1.8)

set(PLATFORM_EXTRA_SOURCES)
set(PLATFORM_EXTRA_INCLUDE_DIRS)
set(PLATFORM_EXTRA_LDFLAGS)

if(AGL_FRAMEWORK)
	# AGL Application Framework
	pkg_check_modules(AFB REQUIRED afb-daemon)
	list(APPEND PLATFORM_EXTRA_SOURCES
		src/agl/AGLAudioManager.cpp
	)
	list(APPEND PLATFORM_EXTRA_INCLUDE_DIRS
		${AFB_INCLUDE_DIRS}
	)
	list(APPEND PLATFORM_EXTRA_LDFLAGS
		${AFB_LDFLAGS}
	)
	add_definitions(-DUSE_AGL_FRAMEWORK)
endif()

add_library(AACEGStreamerAudio SHARED
	src/common/Context.cpp
	src/common/GstPlayer.cpp
	src/common/GstRecorder.cpp
	src/common/GstUtils.cpp
	src/AudioManager.cpp
	src/GstAudioCapture.cpp
	src/GstMediaPlayer.cpp
	${PLATFORM_EXTRA_SOURCES}
)

target_include_directories(AACEGStreamerAudio
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${GST_INCLUDE_DIRS}
		${GST_APP_INCLUDE_DIRS}
		${PLATFORM_EXTRA_INCLUDE_DIRS}
)

target_link_libraries(AACEGStreamerAudio
	AACECoreEngine
	AACEAlexaPlatform
	${GST_LDFLAGS}
	${GST_APP_LDFLAGS}
	${PLATFORM_EXTRA_LDFLAGS}
)

install(
	TARGETS AACEGStreamerAudio
	DESTINATION lib
	EXPORT AACEAudio
)