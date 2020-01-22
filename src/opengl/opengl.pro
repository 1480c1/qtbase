TARGET     = QtOpenGL
QT         = core-private gui-private
qtConfig(widgets): QT += widgets widgets-private

DEFINES   += QT_NO_USING_NAMESPACE QT_NO_FOREACH

QMAKE_DOCS = $$PWD/doc/qtopengl.qdocconf

qtConfig(opengl): CONFIG += opengl
qtConfig(opengles2): CONFIG += opengles2

HEADERS += \
    qopengl2pexvertexarray_p.h \
    qopenglcustomshaderstage_p.h \
    qopengldebug.h \
    qopenglengineshadermanager_p.h \
    qopenglengineshadersource_p.h \
    qopenglgradientcache_p.h \
    qopenglpaintdevice.h \
    qopenglpaintdevice_p.h \
    qopenglpaintengine_p.h \
    qopenglshadercache_p.h \
    qopengltexturecache_p.h \
    qopengltextureglyphcache_p.h \
    qopengltextureuploader_p.h \
    qopenglwindow.h \
    qtopenglglobal.h

SOURCES += \
    qopengl2pexvertexarray.cpp \
    qopenglcustomshaderstage.cpp \
    qopenglengineshadermanager.cpp \
    qopenglgradientcache.cpp \
    qopenglpaintdevice.cpp \
    qopenglpaintengine.cpp \
    qopengltexturecache.cpp \
    qopengltextureglyphcache.cpp \
    qopengltextureuploader.cpp \
    qopenglwindow.cpp \
    qopengldebug.cpp

!qtConfig(opengles2) {
    HEADERS += \
        qopenglqueryhelper_p.h \
        qopengltimerquery.h

    SOURCES += qopengltimerquery.cpp
}

qtConfig(widgets) {
    HEADERS += qopenglwidget.h
    SOURCES += qopenglwidget.cpp
}

load(qt_module)
