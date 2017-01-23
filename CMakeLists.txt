cmake_minimum_required(VERSION 3.0)
project(CtudcServer CXX)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

set(Boost_USE_STATIC_LIBS ON)
find_package(
    Boost
    1.55.0
    COMPONENTS
    system
    filesystem
    REQUIRED)

find_package (LibUSB)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${LibUSB_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES
	"*.cpp")

add_executable(CtudcServer ${SOURCES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

target_link_libraries(
    ${PROJECT_NAME}
    CAENVME
    ftd2xx
    trekdata
    treknet
    pthread
    ${Boost_LIBRARIES}
    ${LibUSB_LIBRARIES})
