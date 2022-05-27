# QNX cmake setting required to build with CMake
set(CMAKE_SYSTEM_NAME QNX)

set(CMAKE_C_COMPILER qcc)
set(CMAKE_C_COMPILER_TARGET gcc_ntoaarch64le)
set(CMAKE_CXX_COMPILER q++)
set(CMAKE_CXX_COMPILER_TARGET gcc_ntoaarch64le)

set(CMAKE_AR ntoaarch64-ar CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB ntoaarch64-ranlib CACHE FILEPATH "Ranlib")
set(_CMAKE_TOOLCHAIN_PREFIX ntoaarch64-)

set(CMAKE_SYSROOT $ENV{QNX_TARGET})
