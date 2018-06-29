find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock)
find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main)

set(UNIT_TEST_SRCS
    NavigationEngineImplTest.cpp
    NavigationCapabilityAgentTest.cpp
)

foreach(TEST_SRC ${UNIT_TEST_SRCS})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_include_directories(${TEST_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${AVS_INCLUDE_DIRS}
    )
    target_link_libraries(${TEST_NAME} AACENavigationEngine
        GTest::GTest GTest::Main ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY} ${AVS_AVS_COMMON_LIBRARY}
    )
    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME})
endforeach()