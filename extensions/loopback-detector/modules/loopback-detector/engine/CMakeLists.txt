find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)

add_library(AACELoopbackDetectorEngine SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopbackDetectorEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LoopbackDetector.cpp
)

target_include_directories(AACELoopbackDetectorEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        # Somehow some AVS Captions headers are now in avs_shared/ folder rather than include/. If you
        # include any AVS headers that also include Captions headers, you need to add avs_shared/ to
        # the include paths.
        ${AVS_INCLUDE_DIRS}/../avs_shared
)

target_link_libraries(AACELoopbackDetectorEngine
    PUBLIC
        AACECoreEngine
        AACEAlexaEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
)

install(
    TARGETS AACELoopbackDetectorEngine
    DESTINATION lib
    EXPORT AACELoopbackDetector
)

