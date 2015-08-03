TEMPLATE = app
TARGET = PsychoServer
CONFIG = console c++11 thread

QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3


win32-mingw {
	LIBS += -lCAENVMElib
	LIBS += -lboost_system-mgw49-1_58
	LIBS += -lboost_filesystem-mgw49-1_58
	LIBS += -lws2_32
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
	managers/ctudcreadmanager.hpp \
	managers/devicemanager.hpp \
	managers/modulehandler.hpp \
	managers/readmanager.hpp \
	net/server.hpp \
	net/session.hpp \
	observer/observer.hpp \
	managers/types.hpp \
	net/nettools.hpp \
	net/packagereciever.hpp \
    configparser/channelsconfigparser.hpp \
    configparser/configparser.hpp \
    threadblocker.hpp

SOURCES += \
	caen/tdcmodule.cpp \
	managers/ctudcreadmanager.cpp \
	managers/devicemanager.cpp \
	managers/modulehandler.cpp \
	managers/readmanager.cpp \
	net/server.cpp \
	net/session.cpp \
	observer/observer.cpp \
	main.cpp \
	net/nettools.cpp \
	net/packagereciever.cpp \
    configparser/channelsconfigparser.cpp
