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
LIBS += -lftd2xx

HEADERS += \
    caen/tdcmodule.hpp \
    caen/types.hpp \
    configparser/appconfigparser.hpp \
    configparser/channelsconfigparser.hpp \
    configparser/configparser.hpp \
    managers/channelconfig.hpp \
    managers/ctudcreadmanager.hpp \
    managers/frequencymanager.hpp \
    managers/processmanager.hpp \
    managers/readmanager.hpp \
    managers/threadmanager.hpp \
    net/nettools.hpp \
    net/packagereceiver.hpp \
    net/server.hpp \
    net/session.hpp \
    observer/observer.hpp \
    applog.hpp \
    appsettings.hpp \
    makestring.hpp \
    timeprint.hpp \
    ftd/defines.hpp \
    ftd/ftdmodule.hpp \
    controller/controller.hpp \
    controller/ctudccontroller.hpp \
    controller/tdccontroller.hpp \
    controller/ftdcontroller.hpp \
    controller/processcontroller.hpp

SOURCES += \
    caen/tdcmodule.cpp \
    configparser/appconfigparser.cpp \
    configparser/channelsconfigparser.cpp \
    managers/ctudcreadmanager.cpp \
    managers/frequencymanager.cpp \
    managers/processmanager.cpp \
    managers/readmanager.cpp \
    managers/threadmanager.cpp \
    net/nettools.cpp \
    net/packagereceiver.cpp \
    net/server.cpp \
    net/session.cpp \
    observer/observer.cpp \
    appsettings.cpp \
    main.cpp \
    ftd/ftdmodule.cpp \
    controller/ctudccontroller.cpp \
    controller/tdccontroller.cpp \
    controller/ftdcontroller.cpp \
    controller/processcontroller.cpp
