# AASB

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(AASB LANGUAGES CXX)

add_subdirectory(modules/aasb)

# handlers
add_subdirectory(modules/aasb-core)
add_subdirectory(modules/aasb-alexa)
add_subdirectory(modules/aasb-apl)
add_subdirectory(modules/aasb-messaging)
add_subdirectory(modules/aasb-navigation)
add_subdirectory(modules/aasb-cbl)
add_subdirectory(modules/aasb-connectivity)
add_subdirectory(modules/aasb-phone-control)
add_subdirectory(modules/aasb-address-book)
add_subdirectory(modules/aasb-car-control)
add_subdirectory(modules/aasb-text-to-speech)