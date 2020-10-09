# AASB Navigation Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Navigation/AASBNavigationEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Navigation/AASBNavigation.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_NAVIGATION_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/AlternateRouteType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/AnnounceManeuverMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/AnnounceRoadRegulationMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/CancelNavigationMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/ControlDisplay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/ControlDisplayMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/ErrorCode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/ErrorType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/EventName.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/GetNavigationStateMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/GetNavigationStateMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/NavigateToPreviousWaypointMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/NavigationErrorMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/NavigationEventMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/RoadRegulation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/ShowAlternativeRoutesMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/ShowAlternativeRoutesSucceededMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/ShowPreviousWaypointsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Navigation/Navigation/StartNavigationMessage.h
)

source_group("Navigation Message Headers" FILES ${AASB_NAVIGATION_MESSAGES})

add_library(AASBNavigationEngine SHARED
    ${HEADERS}
    ${AASB_NAVIGATION_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBNavigationEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBNavigation.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBNavigationEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBNavigationEngine
    PUBLIC
        AACEAASBEngine
        AACENavigationPlatform
)

install(
    TARGETS AASBNavigationEngine
    DESTINATION lib
    EXPORT AASBNavigation
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
