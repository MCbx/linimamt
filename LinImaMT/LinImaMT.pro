#-------------------------------------------------
#
# Project created by QtCreator 2016-01-27T14:57:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LinImaMT
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    errordialog.cpp \
    imagefile.cpp \
    ifilelistwidget.cpp \
    idirtreewidget.cpp \
    attribute.cpp \
    newimage.cpp \
    bootsector.cpp \
    harddiskopen.cpp \
    fileviewer.cpp \
    optionsdialog.cpp \
    mbrsector.cpp

HEADERS  += mainwindow.h \
    errordialog.h \
    imagefile.h \
    ifilelistwidget.h \
    idirtreewidget.h \
    attribute.h \
    newimage.h \
    bootsector.h \
    harddiskopen.h \
    fileviewer.h \
    optionsdialog.h \
    mbrsector.h \
    hexspinbox.h

FORMS    += mainwindow.ui \
    errordialog.ui \
    attribute.ui \
    newimage.ui \
    bootsector.ui \
    harddiskopen.ui \
    fileviewer.ui \
    optionsdialog.ui \
    mbrsector.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    ../README.md \
    ../sshot.png
