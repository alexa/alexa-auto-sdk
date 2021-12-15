cmake_minimum_required(VERSION 3.12)
project(SampleApp CXX)

# include the generated conan cmake file
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

#include any additional cmake files defined in the project
file(GLOB cmake_files ${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake)
foreach(file ${cmake_files})
    include(${file})
endforeach(file)

# run the conan setup script
conan_basic_setup()

# set up the compiler settings for the project
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_SYSTEM_NAME MATCHES "(Darwin)")
    # AACE modules must be explicitly linked
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

include_directories(SampleApp include)
add_executable(SampleApp ${CXX_SOURCE_FILES})
target_link_libraries(SampleApp ${CONAN_LIBS})

if(NOT CMAKE_SYSTEM_NAME MATCHES "(Android|QNX)")
    target_link_libraries(SampleApp pthread)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set_target_properties(SampleApp
        PROPERTIES
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "../lib"
        XCODE_GENERATE_SCHEME True
        XCODE_SCHEME_ENVIRONMENT "DYLD_FALLBACK_LIBRARY_PATH=../lib"
    )
    if(PORTAUDIO)
        target_link_libraries(SampleApp
            "-framework CoreAudio"
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework CoreServices"
            "-framework Carbon"
        )
    endif()
endif()
