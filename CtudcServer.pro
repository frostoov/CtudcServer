TEMPLATE = app
TARGET = CtudcServer
CONFIG = console thread

QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_CXXFLAGS += -std=c++14


win32-g++ {
	LIBS += -lCAENVMElib
	LIBS += -lboost_system-mgw49-1_58
	LIBS += -lboost_filesystem-mgw49-1_58
	LIBS += -lws2_32
	LIBS += -lwsock32
}
linux-clang {
	LIBS += -lCAENVME
	LIBS += -lboost_system
	LIBS += -lboost_filesystem
}


LIBS += -ltrekdata

HEADERS += \
	caen/tdcmodule.hpp \
	caen/types.hpp \
	configparser/channelsconfigparser.hpp \
	configparser/configparser.hpp \
	managers/ctudcreadmanager.hpp \
	managers/frequencymanager.hpp \
	managers/processmanager.hpp \
	managers/readmanager.hpp \
	managers/types.hpp \
	net/nettools.hpp \
	net/packagereceiver.hpp \
	net/server.hpp \
	net/session.hpp \
	observer/observer.hpp \
	managers/threadmanager.hpp \
	configparser/appconfigparser.hpp \
	appsettings.hpp \
	managers/facilitymanager.hpp

SOURCES += \
	caen/tdcmodule.cpp \
	configparser/channelsconfigparser.cpp \
	managers/ctudcreadmanager.cpp \
	managers/frequencymanager.cpp \
	managers/processmanager.cpp \
	managers/readmanager.cpp \
	net/nettools.cpp \
	net/packagereceiver.cpp \
	net/server.cpp \
	net/session.cpp \
	observer/observer.cpp \
	main.cpp \
	managers/threadmanager.cpp \
	configparser/appconfigparser.cpp \
	appsettings.cpp \
	managers/facilitymanager.cpp
