#-------------------------------------------------
#
# Project created by QtCreator 2013-06-22T10:15:34
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = PeerChat
TEMPLATE = app


SOURCES += main.cpp\
           mainwindow.cpp \
           peer.cpp \
           message.cpp

HEADERS  += mainwindow.h \
            peer.h \
            message.h
