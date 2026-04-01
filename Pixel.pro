QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/action.cpp \
    src/canvas.cpp \
    src/contextpannel.cpp \
    src/filterlayer.cpp \
    src/infopannel.cpp \
    src/instrumentpannel.cpp \
    src/layer.cpp \
    src/layerspannel.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/manipulator.cpp \
    src/object.cpp \
    src/palettepannel.cpp \
    src/project.cpp \
    src/projectmanager.cpp \
    src/tool.cpp \
    src/workspacecontroller.cpp

HEADERS += \
    include/action.h \
    include/canvas.h \
    include/contextpannel.h \
    include/filterlayer.h \
    include/infopannel.h \
    include/layer.h \
    include/instrumentpannel.h \
    include/layerspannel.h \
    include/mainwindow.h \
    include/manipulator.h \
    include/object.h \
    include/palettepannel.h \
    include/project.h \
    include/projectmanager.h \
    include/tool.h \
    include/workspacecontroller.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += mainwindow.ui \
    mainwindow.ui

RESOURCES += \
    icons.qrc
