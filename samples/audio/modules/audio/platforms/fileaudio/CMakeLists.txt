add_library(AACEFileAudio SHARED
	src/AudioManager.cpp
	src/EmptyAudioCapture.cpp
	src/MediaPlayer.cpp
	src/Speaker.cpp
)

target_include_directories(AACEFileAudio
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(AACEFileAudio
	AACECoreEngine
	AACEAlexaPlatform
)

install(
	TARGETS AACEFileAudio
	DESTINATION lib
	EXPORT AACEAudio
)