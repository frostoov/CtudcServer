TEMPLATE = app
CONFIG = console c++11 thread

LIBS += -lCAENVME -ltdcdata -lboost_system

SOURCES += main.cpp \
	caenTDC/tdcmodule.cpp \
	observer/observer.cpp

HEADERS += \
	caenTDC/tdcmodule.hpp \
	caenTDC/types.hpp \
	observer/observer.hpp
