QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    src/action.cpp \
    src/canvas.cpp \
    src/concrete_tools.cpp \
    src/contextpannel.cpp \
    src/filter.cpp \
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
    src/raster_action.cpp \
    src/tool.cpp \
    src/workspacecontroller.cpp

HEADERS += \
    include/action.h \
    include/canvas.h \
    include/concrete_tools.h \
    include/contextpannel.h \
    include/filter.h \
    include/filterlayer.h \
    include/infopannel.h \
    include/instrumentpannel.h \
    include/layer.h \
    include/layerspannel.h \
    include/mainwindow.h \
    include/manipulator.h \
    include/object.h \
    include/palettepannel.h \
    include/project.h \
    include/projectmanager.h \
    include/raster_action.h \
    include/tool.h \
    include/workspacecontext.h \
    include/workspacecontroller.h

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += mainwindow.ui \
    mainwindow.ui

RESOURCES += \
    icons.qrc