cmake_minimum_required(VERSION 3.16)
project(obs-pavirtualoutput)

include(external/FindLibObs.cmake)
find_package(LibObs REQUIRED)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(pavirtualoutput_SOURCES
    src/pavirtualoutput.cpp src/PAVirtualOutputProps.cpp)

set(pavirtualoutput_HEADERS
        src/pavirtualoutput.h src/PAVirtualOutputProps.h)

include_directories(
        "${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api")

add_library(pavirtualoutput MODULE ${pavirtualoutput_SOURCES} ${pavirtualoutput_HEADERS})

target_link_libraries(pavirtualoutput
        libobs
        Qt5::Core
        Qt5::Widgets)

if(ARCH EQUAL 64)
    set(ARCH_NAME "x86_64")
else()
    set(ARCH_NAME "i686")
endif()

set_target_properties(pavirtualoutput PROPERTIES PREFIX "")