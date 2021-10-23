QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TARGET = client
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clientcore.cpp \
    filecontainer.cpp \
    fileexplorer.cpp \
    localfilecontainer.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    remotefilecontainer.cpp

HEADERS += \
    clientcore.h \
    filecontainer.h \
    fileexplorer.h \
    localfilecontainer.h \
    logindialog.h \
    mainwindow.h \
    remotefilecontainer.h

FORMS += \
    fileexplorer.ui \
    logindialog.ui \
    mainwindow.ui

TRANSLATIONS += \
    FTPClient_zh_SG.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src.qrc
