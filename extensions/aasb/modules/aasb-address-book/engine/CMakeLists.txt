# AASB Address Book Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/AddressBook/AASBAddressBookEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/AddressBook/AASBAddressBook.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_ADDRESSBOOK_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/AddAddressBookMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/AddAddressBookMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/AddressBook.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/AddressBookType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/ContactName.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/NavigationName.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/PhoneData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/PostalAddress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/RemoveAddressBookMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/AddressBook/AddressBook/RemoveAddressBookMessageReply.h
)

source_group("Address Book Message Headers" FILES ${AASB_ADDRESSBOOK_MESSAGES})

add_library(AASBAddressBookEngine SHARED
    ${HEADERS}
    ${AASB_ADDRESSBOOK_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAddressBookEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAddressBook.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBAddressBookEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)
 
target_link_libraries(AASBAddressBookEngine
    PUBLIC
        AACEAASBEngine
        AACEAddressBookPlatform
)

install(
    TARGETS AASBAddressBookEngine
    DESTINATION lib
    EXPORT AASBAddressBook
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
