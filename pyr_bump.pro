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
    src/res/imagedata.cpp \
    src/res/mesh.cpp \
    src/scene/camera.cpp \
    src/scene/frustum.cpp \
    src/scene/scene.cpp \
    src/scene/sceneentitybuilder.cpp \
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
    src/res/imagedata.h \
    src/res/mesh.h \
    src/scene/AABB.h \
    src/scene/camera.h \
    src/scene/frustum.h \
    src/scene/plane.h \
    src/scene/scene.h \
    src/scene/sceneentitybuilder.h \
    src/window.h

DISTFILES += \
    bin/bump2.0.frag \
    bin/bump2.0.vert
