TEMPLATE = app
CONFIG = console c++11 thread

QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

SOURCES +=\
	main.cpp \
	caen/tdcmodule.cpp \
	observer/observer.cpp \
	server.cpp \
	session.cpp \
	process.cpp \
	devicemanager.cpp

HEADERS += \
	caen/tdcmodule.hpp \
	caen/types.hpp \
	observer/observer.hpp \
	server.hpp \
	session.hpp \
	process.hpp \
	devicemanager.hpp
message($$QMAKESPEC)
win32-mingw {
	LIBS += -lCAENVMElib
	LIBS += -lboost_system-mgw49-1_58
	LIBS += -lws2_32
}
linux-clang {
	LIBS += -lCAENVME
	LIBS += -lboost_system
}


LIBS += -ltdcdata
