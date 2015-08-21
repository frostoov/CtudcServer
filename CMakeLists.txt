cmake_minimum_required (VERSION 3.0)
project(CtudcServer)

add_definitions(-std=c++14 -O3)

set(
	SOURCE
	observer/observer.cpp
	caen/tdcmodule.cpp
	managers/threadmanager.cpp
	managers/processmanager.cpp
	managers/readmanager.cpp
	managers/frequencymanager.cpp
	managers/ctudcreadmanager.cpp
	net/nettools.cpp
	net/packagereceiver.cpp
	net/server.cpp
	net/session.cpp
	configparser/appconfigparser.cpp
	configparser/channelsconfigparser.cpp
	ftd/ftdmodule.cpp
	controller/ftdcontroller.cpp
	controller/tdccontroller.cpp
	controller/ctudccontroller.cpp
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
	net/session.hpp
	net/server.hpp
	net/nettools.hpp
	configparser/configparser.hpp
	configparser/appconfigparser.hpp
	configparser/channelsconfigparser.hpp
	makestring.hpp
	applog.hpp
	ftd/defines.hpp
	ftd/ftdmodule.hpp
	controller/controller.hpp
	controller/ftdcontroller.hpp
	controller/tdccontroller.hpp
	controller/ctudccontroller.hpp
	controller/processcontroller.hpp
	appsettings.hpp
	timeprint.hpp
)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(CtudcServer ${SOURCE})
target_link_libraries(
	CtudcServer
	CAENVME
	boost_system
	boost_filesystem
	trekdata
	ftd2xx
    pthread
)

if(WIN32)
	target_link_libraries(
		CtudcServer
		ws2_32
		wsock32
	)
endif(WIN32)