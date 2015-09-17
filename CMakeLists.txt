cmake_minimum_required (VERSION 3.0)
project(CtudcServer)

add_definitions(${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -fPIC)

set(
    SOURCES
    observer/observer.cpp
    caen/tdcmodule.cpp
    managers/threadmanager.cpp
    managers/processmanager.cpp
    managers/readmanager.cpp
    managers/frequencymanager.cpp
    managers/ctudcreadmanager.cpp
    net/nettools.cpp
    net/packagereceiver.cpp
    configparser/appconfigparser.cpp
    configparser/channelsconfigparser.cpp
    ftd/ftdmodule.cpp
    controller/ftdcontroller.cpp
    controller/tdccontroller.cpp
    controller/processcontroller.cpp
    appsettings.cpp
    main.cpp
)

set(
    HEADERS
    observer/observer.hpp
    caen/types.hpp
    caen/tdcmodule.hpp
    managers/threadmanager.hpp
    managers/readmanager.hpp
    managers/ctudcreadmanager.hpp
    managers/frequencymanager.hpp
    managers/channelconfig.hpp
    managers/processmanager.hpp
    net/packagereceiver.hpp
    net/nettools.hpp
    configparser/configparser.hpp
    configparser/appconfigparser.hpp
    configparser/channelsconfigparser.hpp
    ftd/defines.hpp
    ftd/ftdmodule.hpp
    controller/ftdcontroller.hpp
    controller/tdccontroller.hpp
    controller/processcontroller.hpp
    appsettings.hpp
)

add_executable(CtudcServer ${SOURCES} ${HEADERS})

find_package(
    Boost
    1.55.0
    COMPONENTS
    system
    filesystem
    REQUIRED
)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(
    CtudcServer
    trekcommon
    trekdata
    treknet
    ftd2xx
    pthread
    ${Boost_LIBRARIES}
)

if(UNIX)
    target_link_libraries(
        CtudcServer
        CAENVME
    )
endif(UNIX)

if(WIN32)
    target_link_libraries(
        CtudcServer
        CAENVMElib
        ws2_32
        wsock32
    )
endif(WIN32)
