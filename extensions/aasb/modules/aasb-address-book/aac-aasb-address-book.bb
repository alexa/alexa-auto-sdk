SUMMARY = "Alexa Automotive Core - AASB Address Book"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://engine/include/AASB/Engine/AddressBook/AASBAddressBookEngineService.h;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

DEPENDS = "aac-module-address-book aac-module-aasb nlohmann"

inherit pkgconfig aac-module devlibsonly
