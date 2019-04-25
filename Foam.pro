######################################################################
# Automatically generated by qmake (3.1) Mon Nov 19 12:20:41 2018
######################################################################

TEMPLATE = app
TARGET = Foam
INCLUDEPATH += . /usr/include/SDL2
CONFIG += c++17
CONFIG -= qt

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
SOURCES += main.cpp

LIBS += -lSDL2 -lSDL2main -lGLEW -lGL

HEADERS += \
    Repack.h \
    Properties.h \
    Button.h \
    Common.h \
    Context.h \
    Rectangle.h \
    MouseArea.h \
    Item.h \
    Component.h \
    Application.h \
    Style.h \
    DefaultStyle.h \
    TextBox.h

DISTFILES += \
    shader.vert \
    shader.frag
