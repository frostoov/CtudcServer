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

set(SOURCES
    configparser/appconfigparser.cpp
    configparser/channelsconfigparser.cpp
    controller/tdccontroller.cpp
    controller/expocontroller.cpp
    controller/voltagecontroller.cpp
    controller/emisscontr.cpp
    appsettings.cpp
    voltage/amplifier.cpp
    exposition/channelconfig.cpp
    exposition/exposition.cpp
    exposition/eventwriter.cpp
    exposition/freq.cpp
    ftd/ftdmodule.cpp
    tdc/caenv2718.cpp
    tdc/emisstdc.cpp
    tdc/tdc.cpp
    emiss/controlerem1.cpp
    emiss/controlerem8.cpp
    emiss/pciqbus.cpp
    main.cpp)

set(HEADERS
    configparser/configparser.hpp
    configparser/appconfigparser.hpp
    configparser/channelsconfigparser.hpp
    controller/tdccontroller.hpp
    controller/expocontroller.hpp
    controller/voltagecontroller.hpp
    controller/emisscontr.cpp
    voltage/amplifier.hpp
    exposition/channelconfig.hpp
    exposition/process.hpp
    exposition/eventwriter.hpp
    exposition/exposition.hpp
    exposition/freq.hpp
    ftd/ftdmodule.hpp
    ftd/defines.hpp
    appsettings.hpp
    tdc/tdc.hpp
    tdc/caenv2718.hpp
    tdc/emisstdc.cpp
    emiss/controlerem1.hpp
    emiss/controlerem8.hpp
    emiss/pciqbus.hpp)

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
