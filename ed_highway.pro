#-------------------------------------------------
#
# Project created by QtCreator 2019-08-09T15:42:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ed_highway
TEMPLATE = app

CONFIG(debug, debug|release): DEFINES += SRC_PATH='\\"$$PWD\\"'

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
        edsmapiv1.cpp \
        lzokay.cpp \
        main.cpp \
        mainwindow.cpp \
        qjsontablemodel.cpp \
        utils/restclient.cpp
HEADERS += \
        edsmapiv1.h \
        edsmv1_nearest.h \
        edsmv1_sysinfo.h \
        lzokay.hpp \
        point.h \
        qjsontablemodel.h \
        utils/cm_ctors.h \
        utils/ctpl_stl.h \
        utils/exec_exit.h \
        utils/guard_on.h \
        utils/json.hpp \
        mainwindow.h \
        utils/restclient.h \
        utils/runners.h \
        utils/shared_wrapper.h \
        utils/strfmt.h \
        utils/strutils.h \
        utils/type_checks.h \
        utils/variant_convert.h

FORMS += \
        mainwindow.ui
include($$PWD/widgets/widgets.pri)
include($$PWD/singleapp/singleapplication.pri)
include($$PWD/config_ui/config_ui.pri)


#ocr functionality, comment out to disable it
include($$PWD/ocr/ocr.pri)

#support for global hotkeys even when program is not active
include($$PWD/QHotkey/qhotkey.pri)

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
