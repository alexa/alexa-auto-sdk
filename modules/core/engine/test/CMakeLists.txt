find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock)
find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main)

set(UNIT_TEST_SRCS
    EngineImplTest.cpp
    VehicleConfigurationImplTest.cpp
)

foreach(TEST_SRC ${UNIT_TEST_SRCS})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} AACECoreEngine
        GTest::GTest GTest::Main ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY})
    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME})
endforeach()
