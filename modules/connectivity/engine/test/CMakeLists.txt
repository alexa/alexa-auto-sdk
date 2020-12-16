find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock)
find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main)

find_path(GMXCT_INCLUDE_DIR gmxct/gmxct.mm
    CMAKE_FIND_ROOT_PATH_BOTH
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(TEST_SOURCES
    src/AlexaConnectivityEngineImplTest.cpp
)

if(XCODE AND EXISTS ${GMXCT_INCLUDE_DIR}/gmxct/gmxct.mm)
    enable_testing()
    find_package(XCTest REQUIRED)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")

    # Dummy Source
    # 1. The test bundle requires gmxct.mm and a static library for linking.
    # 2. ${TEST_SOURCES} do not work here because they need to be with gmxct.mm.
    # Adding the dummy symbol enables linking and avoids duplicating ${TEST_SOURCES}.
    if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
            "void _dummy() {}"
        )
    endif()

    # Static Lib
    add_library(ConnectivityLib STATIC
        ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
    )
    target_include_directories(ConnectivityLib
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${AVS_INCLUDE_DIRS}
    )
    target_link_libraries(ConnectivityLib AACEConnectivityEngine AACEAlexaTestsLib
        GTest::GTest ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY} ${AVS_AVS_COMMON_LIBRARY}
    )

    # XCTest Bundle
    xctest_add_bundle(ConnectivityTests ConnectivityLib ${GMXCT_INCLUDE_DIR}/gmxct/gmxct.mm ${TEST_SOURCES})
    xctest_add_test(XCTest.ConnectivityTests ConnectivityTests)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    foreach(TEST_SOURCE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        add_executable(${TEST_NAME}
            ${TEST_SOURCE}
        )
        target_include_directories(${TEST_NAME}
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
            PRIVATE
                ${AVS_INCLUDE_DIRS}
        )
        target_link_libraries(${TEST_NAME} AACEConnectivityEngine AACEAlexaTestsLib
            GTest::GTest GTest::Main ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY} ${AVS_AVS_COMMON_LIBRARY}
        )
        add_test(NAME ${TEST_NAME}
            COMMAND ${CMAKE_COMMAND}
                -E env GTEST_OUTPUT=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}.xml
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}
        )
    endforeach()
endif()
