QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#win32:contains(QMAKE_TARGET.arch, x86_64) {
#    message("x86_64 build")
#    INCLUDEPATH += D:\AvaSpecX64-DLL_9.11.0.0\qwt-6.1.4\src\
#} else {
#    message("win32 build")
#    INCLUDEPATH += D:\AvaSpecX64-DLL_9.11.0.0\qwt-6.1.4\src\
#}

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    avaspec.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

win32:contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L$$PWD -lavaspecx64
}
else {
        LIBS += -L$$PWD -lavaspecx64
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
