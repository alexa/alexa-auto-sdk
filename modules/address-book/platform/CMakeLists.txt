# AACE AddressBook Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/AddressBook/AddressBook.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/AddressBook/AddressBookEngineInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAddressBookPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AddressBook.cpp
)

target_include_directories(AACEAddressBookPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEAddressBookPlatform
    AACECorePlatform
)

install(
    TARGETS AACEAddressBookPlatform
    DESTINATION lib
    EXPORT AACEAddressBook
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
