# Alexa Auto SDK C++ Sample App

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(APPLICATION_NAME "SampleApp" CACHE STRING "Scriptable application name")
if(APPLICATION_NAME)
    add_subdirectory(${APPLICATION_NAME})
endif()
