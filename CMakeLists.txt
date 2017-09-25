cmake_minimum_required (VERSION 2.8)

project (HandPointing)

set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")
set(CMAKE_DEBUG_POSTFIX "_d")

include_directories($ENV{RSSDK_DIR}/include)

if(WIN32)
    link_directories($ENV{RSSDK_DIR}/lib/Win32)
endif()

add_executable(HandPointing main.cpp)
target_link_libraries(HandPointing libpxcmd)

MESSAGE(STATUS "PlatformName:             " $ENV{PlatformName} )
MESSAGE(STATUS "CMAKE_DEBUG_POSTFIX:      " ${CMAKE_DEBUG_POSTFIX} )

