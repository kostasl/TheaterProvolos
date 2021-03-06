### Required dependencies:
### sudo apt-get install libglfw3-dev libglfw3
### sudo apt-get install libglew-dev
### sudo apt-get install libglm-dev
### sudo apt-get install libsoil-dev
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
    particle_generator.cpp \
    game.cpp \
    shader.cpp \
    texture2d.cpp \
    resource_manager.cpp \
    game_object.cpp \
    sprite_renderer.cpp \
    ball_object.cpp \
    ball_object.cpp \
    config.cpp \
    game.cpp \
    game_object.cpp \
    main.cpp \
    particle_generator.cpp \
    resource_manager.cpp \
    shader.cpp \
    sprite_renderer.cpp \
    texture2d.cpp \

LIBS += -lGL -lglfw -lGLEW -lGLU  -lX11 -lpthread -lXrandr -lXi -ldl -lSOIL
LIBS += `pkg-config opencv --libs`


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    config.h \
    particle_generator.h \
    game.h \
    shader.h \
    texture2d.h \
    resource_manager.h \
    game_object.h \
    sprite_renderer.h \
    ball_object.h \
    ball_object.h \
    config.h \
    game.h \
    game_object.h \
    particle_generator.h \
    resource_manager.h \
    shader.h \
    sprite_renderer.h \
    texture2d.h

DISTFILES += \
    res/particle.frag \
    res/particle.vs \
    res/sprite.vs \
    res/sprite.frag
