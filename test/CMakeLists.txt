cmake_minimum_required (VERSION 2.8)

include(CTest)
enable_testing()

include(../cmake/DownloadProject/DownloadProject.cmake)
#set(gtest_disable_pthreads on) #TODO(randl): Windows?
download_project(
    PROJ googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG master
    UPDATE_DISCONNECTED 1
)

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
endif()

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

include_directories(..)
include_directories($ENV{RSSDK_DIR}/include)

if(WIN32)
    link_directories($ENV{RSSDK_DIR}/lib/Win32)
endif()

add_executable(handpointing_test test.cpp ../HandPointing.cpp)
target_link_libraries(handpointing_test
    ${project_library_target_name} ${REQUIRED_LIBRARIES} gtest gmock)

add_test(all_tests handpointing_test)

add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS tiny_dnn_test)

