TEMPLATE = app
CONFIG = console c++11 thread

QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

SOURCES +=\
    main.cpp \
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
windows {
    LIBS += -lCAENVMElib
    LIBS += -lboost_system-mgw49-1_58
    LIBS += -lws2_32
}
unix {
    LIBS += -lCAENVME
    LIBS += -lboost_system
}


LIBS += -ltdcdata
