### Required dependencies:
### sudo apt-get install libglfw3-dev libglfw3
### sudo apt-get install libglew-dev
### sudo apt-get install libglm-dev
### sudo apt-get install libsoil-dev
##SOIL
#First download the SOIL.h header file from its website
#http://www.lonesock.net/soil.html
#Place the header file in your project directory and put the line
#in your project file. After that you have to install the soil library to use -lSOIL. To install the library use the command
# sudo apt-get install libsoil-dev



QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        config.cpp \

LIBS += -lGL -lglfw -lGLEW -lGLU  -lX11 -lpthread -lXrandr -lXi -ldl -lSOIL
LIBS += `pkg-config opencv --libs`


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    config.h \

DISTFILES += \
    res/particle.frag \
    res/particle.vs \
    res/sprite.vs \
    res/sprite.frag
