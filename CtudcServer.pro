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


LIBS += -ltdcdata

HEADERS += \
	caen/tdcmodule.hpp \
	caen/types.hpp \
	configparser/channelsconfigparser.hpp \
	configparser/configparser.hpp \
	managers/ctudcreadmanager.hpp \
	managers/devicemanager.hpp \
	managers/frequencymanager.hpp \
	managers/processmanager.hpp \
	managers/readmanager.hpp \
	managers/types.hpp \
	net/nettools.hpp \
	net/server.hpp \
	net/session.hpp \
	observer/observer.hpp \
	managers/threadmanager.hpp \
    net/packagereceiver.hpp

SOURCES += \
	caen/tdcmodule.cpp \
	configparser/channelsconfigparser.cpp \
	managers/ctudcreadmanager.cpp \
	managers/devicemanager.cpp \
	managers/frequencymanager.cpp \
	managers/processmanager.cpp \
	managers/readmanager.cpp \
	net/nettools.cpp \
	net/server.cpp \
	net/session.cpp \
	observer/observer.cpp \
	main.cpp \
	managers/threadmanager.cpp \
    net/packagereceiver.cpp

