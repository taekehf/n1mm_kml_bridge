QT       += core network
QT       -= gui

TARGET    = n1mm_kml_bridge
TEMPLATE  = app
CONFIG   += console c++23
CONFIG   -= app_bundle

# Force the ratified -std=c++23 flag explicitly.
# qmake sometimes only passes -std=c++2b (the pre-ratification name),
# which does NOT unlock <print> on GCC 14 / MinGW builds.
QMAKE_CXXFLAGS += -std=c++23 -Wall -Wextra

SOURCES += \
    loadconfig.cpp \
    main.cpp \
    bridge.cpp \
    qrzclient.cpp \
    kmlwriter.cpp

HEADERS += \
    config.h \
    bridge.h \
    loadconfig.h \
    qrzclient.h \
    kmlwriter.h \
    contact.h \
    blockingqueue.h \
    maidenhead.h \
    compat_print.h
