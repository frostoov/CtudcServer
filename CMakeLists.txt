cmake_minimum_required(VERSION 3.0)
project(CtudcServer)

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
include_directories(${PROJECT_SOURCE_DIR})

set(
	SOURCES
	configparser/appconfigparser.cpp
	configparser/channelsconfigparser.cpp
	controller/tdccontroller.cpp
	controller/expocontroller.cpp
	controller/voltagecontroller.cpp
	net/packagereceiver.cpp
	appsettings.cpp
	voltage/amplifier.cpp
	voltage/serialbuf.cpp
	exposition/exposition.cpp
	exposition/eventwriter.cpp
	exposition/freq.cpp
	ftd/ftdmodule.cpp
	tdc/caenv2718.cpp
        tdc/tdc.cpp
	main.cpp
)

set(
	HEADERS
	configparser/configparser.hpp
	configparser/appconfigparser.hpp
	configparser/channelsconfigparser.hpp
	controller/tdccontroller.hpp
	controller/expocontroller.hpp
	controller/voltagecontroller.hpp
	net/packagereceiver.hpp
	voltage/amplifier.hpp
	voltage/serialbuf.hpp
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
)

add_executable(CtudcServer ${SOURCES})

set(Boost_USE_STATIC_LIBS ON)
find_package(
		Boost
		1.55.0
		COMPONENTS
		system
		filesystem
		REQUIRED
)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(
		${PROJECT_NAME}
		CAENVME
		ftd2xx
		trekcommon
		trekdata
		treknet
		pthread
		${Boost_LIBRARIES}
)
