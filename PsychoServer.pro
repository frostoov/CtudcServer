TEMPLATE = app
CONFIG = console c++11 thread

windows : {
	LIBS += -lCAENVMElib
	LIBS += -lboost_system-mgw49
	LIBS += -lw2_32
}
else{
	LIBS += -lCAENVME
	LIBS += -lboost_system
}

LIBS += -ltdcdata

SOURCES += main.cpp \
	caenTDC/tdcmodule.cpp \
	observer/observer.cpp \
	server.cpp \
	session.cpp \
	handler.cpp

HEADERS += \
	caenTDC/tdcmodule.hpp \
	caenTDC/types.hpp \
	observer/observer.hpp \
	server.hpp \
	session.hpp \
	handler.hpp
