TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(release, debug|release) {
    #This is a release build
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS += -s
} else {
    #This is a debug build
    DEFINES += DEBUG
    TARGET = $$join(TARGET,,,_d)
}

DESTDIR = $$PWD/bin

QMAKE_CXXFLAGS += -std=c++17 -Wno-unused-parameter -Wconversion -Wold-style-cast

INCLUDEPATH += $$PWD/include

LIBS += -L$$PWD/lib

unix:{
    LIBS += -lglfw -lGL -lGLEW
}
win32:{
    LIBS += -lglfw3dll -lopengl32 -lglew32.dll
    LIBS += -static-libgcc -static-libstdc++
    LIBS += -static -lpthread
}

SOURCES += \
    src/input/input.cpp \
    src/input/inputglfw.cpp \
    src/main.cpp \
    src/render/renderer.cpp \
    src/res/imagedata.cpp \
    src/scene/camera.cpp \
    src/scene/frustum.cpp \
    src/scene/light.cpp \
	src/scene/model.cpp \
    src/scene/material.cpp \
    src/scene/scene.cpp \
    src/scene/sceneentitybuilder.cpp \
    src/utils/controller.cpp \
    src/window.cpp

HEADERS += \
    src/ent/entt_traits.hpp \
    src/ent/family.hpp \
    src/ent/registry.hpp \
    src/ent/sparse_set.hpp \
    src/ent/view.hpp \
    src/input/input.h \
    src/input/inputglfw.h \
    src/input/key_codes.h \
    src/render/render_states.h \
    src/render/renderer.h \
    src/res/imagedata.h \
    src/scene/AABB.h \
    src/scene/camera.h \
    src/scene/frustum.h \
    src/scene/light.h \
    src/scene/material.h \
	src/scene/model.h \
    src/scene/plane.h \
    src/scene/scene.h \
    src/scene/sceneentitybuilder.h \
    src/utils/controller.h \
    src/window.h

DISTFILES += \
    bin/bump2.0.frag \
    bin/bump2.0.vert
