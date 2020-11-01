#-------------------------------------------------
#
# Project created by QtCreator 2017-10-05T11:00:53
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Qch
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


SOURCES += \
    bbsmenu/bbsmenu.cpp \
    bbsmenu/bbsmenucacheloader.cpp \
    bbsmenu/bbsmenuloaderbase.cpp \
    bbsmenu/bbsmenusaver.cpp \
    bbsmenu/bbsmenuwebloader.cpp \
    bbsmenu/filepath.cpp \
    dbaccess/dbaccess.cpp \
    dbaccess/dbutil.cpp \
    dbaccess/listloader.cpp \
        main.cpp \
        mainwindow.cpp \
    hbbsutil.cpp \
    settings.cpp \
    datacache.cpp \
    qchexception.cpp \
    threadlist/threadlist.cpp \
    threadlist/threadlistloader.cpp \
    threadlist/threadlistmodelloader.cpp \
    writemessagedialog/writemessagedialog.cpp \
    writemessagedialog/writemessagedialogmodel.cpp \
    xmloutput.cpp \
    xmlinput.cpp

HEADERS += \
    bbsmenu/bbsmenu.h \
    bbsmenu/bbsmenucacheloader.h \
    bbsmenu/bbsmenuloaderbase.h \
    bbsmenu/bbsmenusaver.h \
    bbsmenu/bbsmenuwebloader.h \
    bbsmenu/filepath.h \
    common.h \
    dbaccess/dbaccess.h \
    dbaccess/dbutil.h \
    dbaccess/listloader.h \
        mainwindow.h \
    hbbsutil.h \
    settings.h \
    datacache.h \
    qchexception.h \
    threadlist/TLDefine.h \
    threadlist/threadlist.h \
    threadlist/threadlistloader.h \
    threadlist/threadlistmodelloader.h \
    writemessagedialog/writemessagedialog.h \
    writemessagedialog/writemessagedialogmodel.h \
    xmloutput.h \
    xmlinput.h

FORMS += \
        mainwindow.ui \
        writemessagedialog/writemessagedialog.ui

DISTFILES += \
    bbsmenu/Doc/BBSMenuUML.drawio \
    bbsmenu/Doc/BBSメニュー.xls \
    bbsmenu/Doc/README.txt \
    bbsmenu/README.txt \
    threadlist/README.txt

CONFIG += c++14

msvc:QMAKE_CXXFLAGS += /source-charset:utf-8 /execution-charset:utf-8
