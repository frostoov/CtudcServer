cmake_minimum_required (VERSION 3.0)
project(CtudcServer)

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -fPIC")

set(
	SOURCES
	tdc/caenv2718.cpp
	tdc/emissem1.cpp
	tdc/pciqbus.cpp
	exposition/exposition.cpp
	exposition/eventwriter.cpp
	net/packagereceiver.cpp
	configparser/appconfigparser.cpp
	configparser/channelsconfigparser.cpp
	ftd/ftdmodule.cpp
	controller/ftdcontroller.cpp
	controller/tdccontroller.cpp
	controller/proccontroller.cpp
	controller/ardcontroller.cpp
	voltage/serialbuf.cpp
	voltage/voltage.cpp
	appsettings.cpp
	main.cpp
)

set(
	HEADERS
	tdc/caenv2718.hpp
	tdc/emissem1.hpp
	tdc/pciqbus.hpp
	tdc/tdc.hpp
	exposition/exposition.hpp
	exposition/channelconfig.hpp
	exposition/process.hpp
	exposition/eventwriter.hpp
	net/packagereceiver.hpp
	configparser/configparser.hpp
	configparser/appconfigparser.hpp
	configparser/channelsconfigparser.hpp
	ftd/defines.hpp
	ftd/ftdmodule.hpp
	controller/ftdcontroller.hpp
	controller/tdccontroller.hpp
	controller/proccontroller.hpp
	controller/ardcontroller.hpp
	voltage/serialbuf.hpp
	voltage/voltage.hpp
	appsettings.hpp
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
