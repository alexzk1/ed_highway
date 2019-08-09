#-------------------------------------------------
#
# Project created by QtCreator 2019-08-09T15:42:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ed_highway
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        restclient.cpp \
        spanshapi.cpp \
        spanshroutewidget.cpp \
        spanshsyssuggest.cpp

HEADERS += \
        ctpl_stl.h \
        json.hpp \
        mainwindow.h \
        restclient.h \
        runners.h \
        spansh_route.h \
        spansh_sysname.h \
        spanshapi.h \
        spanshroutewidget.h \
        spanshsyssuggest.h \
        strfmt.h \
        strutils.h

FORMS += \
        mainwindow.ui \
        spanshroutewidget.ui

include($$PWD/singleapp/singleapplication.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QMAKE_CXXFLAGS +=  -pipe -std=c++14 -Wall -frtti -fexceptions -Werror=return-type -Werror=overloaded-virtual
QMAKE_CXXFLAGS +=  -Wctor-dtor-privacy -Werror=delete-non-virtual-dtor -fstrict-aliasing
QMAKE_CXXFLAGS +=  -Werror=strict-aliasing -Wstrict-aliasing=2

LIBS += -lcurl
LIBS += -lz -dead_strip
DEFINES += QAPPLICATION_CLASS=QApplication


DISTFILES +=

RESOURCES += \
    resources.qrc
