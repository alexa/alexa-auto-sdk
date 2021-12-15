cmake_minimum_required(VERSION 3.12)
project(AutoSdkModule CXX)

# include the generated conan cmake file
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

# include the generated aac cmake file
include(${CMAKE_BINARY_DIR}/aacbuildinfo.cmake)

#include any additional cmake files defined by the project
file(GLOB cmake_files ${CMAKE_BINARY_DIR}/cmake/*.cmake)
foreach(file ${cmake_files})
    include(${file})
endforeach(file)

# run the conan setup script
conan_basic_setup(${CONAN_SETUP_OPTIONS})

# set up default compiler settings for every module
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(AutoSdkModule ${AAC_SOURCES})

target_include_directories(AutoSdkModule PUBLIC ${AAC_INCLUDES})

set(MODULE_FRAMEWORKS ${CONAN_FRAMEWORKS})
list(TRANSFORM MODULE_FRAMEWORKS PREPEND "-framework ")

target_link_libraries(AutoSdkModule
    PUBLIC
    ${CONAN_LIBS}
    ${AAC_SYSTEM_LIBS}
    ${MODULE_FRAMEWORKS}
    PRIVATE
    $<$<CONFIG:Release>:-Wl,-S>
)

# build unit tests
if (AAC_ENABLE_UNIT_TESTS)
    if (AAC_UNIT_TEST_FRAMEWORK_SOURCES)
        include_directories(AutoSdkModuleUnitTestFramework ${AAC_UNIT_TEST_FRAMEWORK_INCLUDES})
        add_library(AutoSdkModuleUnitTestFramework STATIC ${AAC_UNIT_TEST_FRAMEWORK_SOURCES})
        target_link_libraries(AutoSdkModuleUnitTestFramework
            ${CONAN_LIBS}
            AutoSdkModule
        )
        set(AAC_UNIT_TEST_LIBS AutoSdkModuleUnitTestFramework)
    endif()
    enable_testing()
    foreach(TEST_SRC ${AAC_UNIT_TESTS})
        get_filename_component( TEST_NAME ${TEST_SRC} NAME_WE )
        add_executable( ${TEST_NAME} ${TEST_SRC} )
        include_directories(${TEST_NAME} ${AAC_UNIT_TEST_FRAMEWORK_INCLUDES})
        target_link_libraries(${TEST_NAME} ${CONAN_LIBS} AutoSdkModule ${AAC_UNIT_TEST_LIBS})
        add_test(NAME ${TEST_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            COMMAND ${TEST_NAME})
    endforeach()
endif()
