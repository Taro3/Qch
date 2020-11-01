QT += testlib sql
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle C++14

TEMPLATE = app

DBACCESSPATH = ../.. ../../..
INCLUDEPATH += $$DBACCESSPATH
DEPENDPATH += $$DBACCESSPATH

HEADERS += ../../dbutil.h \
            ../../../qchexception.h \
            ../../dbaccess.h \
            ../../listloader.h

SOURCES +=  tst_dbaccesstest.cpp \
            ../../dbutil.cpp \
            ../../../qchexception.cpp \
            ../../dbaccess.cpp \
            ../../listloader.cpp
