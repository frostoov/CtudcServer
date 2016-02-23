cmake_minimum_required (VERSION 3.0)
project(CtudcServer)

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic")
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
	tdc/emissem1.cpp
	tdc/pciqbus.cpp
	tdc/caenv2718.cpp
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
	appsettings.hpp
	tdc/tdc.hpp
	tdc/pciqbus.hpp
	tdc/caenv2718.hpp
	tdc/emissem1.hpp
)

add_executable(CtudcServer ${SOURCES})

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
