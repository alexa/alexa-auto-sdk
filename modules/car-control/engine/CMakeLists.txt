# AACE CarControl Engine

# Find AVS
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)
find_library(AVS_ENDPOINTS_LIBRARY Endpoints)
find_library(AVS_MODE_CONTROLLER_LIBRARY ModeController)
find_library(AVS_POWER_CONTROLLER_LIBRARY PowerController)
find_library(AVS_RANGE_CONTROLLER_LIBRARY RangeController)
find_library(AVS_TOGGLE_CONTROLLER_LIBRARY ToggleController)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/AssetsDefault.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/AssetStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/CapabilityController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/CarControlConfigurationImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/CarControlEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/CarControlEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/CarControlServiceInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/Endpoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/ModeController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/PowerController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/PrimitiveController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/RangeController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/ToggleController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControl/ZoneDefinitions.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACECarControlEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AssetStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CapabilityController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControlConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControlEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControlEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Endpoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ModeController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PowerController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PrimitiveController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RangeController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ToggleController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ZoneDefinitions.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AACECarControlEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AACECarControlEngine
    PUBLIC
		AACECarControlPlatform
        AACECoreEngine
		AACEAlexaEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${AVS_ENDPOINTS_LIBRARY}
        ${AVS_MODE_CONTROLLER_LIBRARY}
        ${AVS_POWER_CONTROLLER_LIBRARY}
        ${AVS_RANGE_CONTROLLER_LIBRARY}
        ${AVS_TOGGLE_CONTROLLER_LIBRARY}
)

install(
    TARGETS AACECarControlEngine
    DESTINATION lib
    EXPORT AACECarControl
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
