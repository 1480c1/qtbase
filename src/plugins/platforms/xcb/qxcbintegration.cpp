/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qxcbintegration.h"
#include "qxcbconnection.h"
#include "qxcbscreen.h"
#include "qxcbwindow.h"
#include "qxcbbackingstore.h"
#include "qxcbnativeinterface.h"
#include "qxcbclipboard.h"
#include "qxcbdrag.h"
#include "qxcbsharedgraphicscache.h"

#include <xcb/xcb.h>

#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixservices_p.h>

#include <stdio.h>

//this has to be included before egl, since egl pulls in X headers
#include <QtGui/private/qguiapplication_p.h>

#ifdef XCB_USE_EGL
#include <EGL/egl.h>
#endif

#ifdef XCB_USE_XLIB
#include <X11/Xlib.h>
#endif

#include <private/qplatforminputcontextfactory_qpa_p.h>
#include <private/qgenericunixthemes_p.h>
#include <qplatforminputcontext_qpa.h>

#if defined(XCB_USE_GLX)
#include "qglxintegration.h"
#elif defined(XCB_USE_EGL)
#include "qxcbeglsurface.h"
#include <QtPlatformSupport/private/qeglplatformcontext_p.h>
#endif

#include <QtGui/QOpenGLContext>
#include <QtGui/QScreen>
#include <QtGui/QPlatformAccessibility>

QT_BEGIN_NAMESPACE

QXcbIntegration::QXcbIntegration(const QStringList &parameters)
    : m_eventDispatcher(createUnixEventDispatcher()),
      m_services(new QGenericUnixServices),
      m_theme(QGenericUnixTheme::createUnixTheme())
{
    QGuiApplicationPrivate::instance()->setEventDispatcher(m_eventDispatcher);

#ifdef XCB_USE_XLIB
    XInitThreads();
#endif
    m_nativeInterface.reset(new QXcbNativeInterface);

    m_connections << new QXcbConnection(m_nativeInterface.data());

    for (int i = 0; i < parameters.size() - 1; i += 2) {
        qDebug() << parameters.at(i) << parameters.at(i+1);
        QString display = parameters.at(i) + ':' + parameters.at(i+1);
        m_connections << new QXcbConnection(m_nativeInterface.data(), display.toAscii().constData());
    }

    foreach (QXcbConnection *connection, m_connections)
        foreach (QXcbScreen *screen, connection->screens())
            screenAdded(screen);

    m_fontDatabase.reset(new QGenericUnixFontDatabase());
    m_inputContext.reset(QPlatformInputContextFactory::create());
    m_accessibility.reset(new QPlatformAccessibility());

#if defined(QT_USE_XCB_SHARED_GRAPHICS_CACHE)
    m_sharedGraphicsCache.reset(new QXcbSharedGraphicsCache);
#endif
}

QXcbIntegration::~QXcbIntegration()
{
    qDeleteAll(m_connections);
}

QPlatformWindow *QXcbIntegration::createPlatformWindow(QWindow *window) const
{
    return new QXcbWindow(window);
}

#if defined(XCB_USE_EGL)
class QEGLXcbPlatformContext : public QEGLPlatformContext
{
public:
    QEGLXcbPlatformContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share,
                           EGLDisplay display, QXcbConnection *c)
        : QEGLPlatformContext(glFormat, share, display)
        , m_connection(c)
    {
        Q_XCB_NOOP(m_connection);
    }

    void swapBuffers(QPlatformSurface *surface)
    {
        Q_XCB_NOOP(m_connection);
        QEGLPlatformContext::swapBuffers(surface);
        Q_XCB_NOOP(m_connection);
    }

    bool makeCurrent(QPlatformSurface *surface)
    {
        Q_XCB_NOOP(m_connection);
        bool ret = QEGLPlatformContext::makeCurrent(surface);
        Q_XCB_NOOP(m_connection);
        return ret;
    }

    void doneCurrent()
    {
        Q_XCB_NOOP(m_connection);
        QEGLPlatformContext::doneCurrent();
        Q_XCB_NOOP(m_connection);
    }

    EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface *surface)
    {
        return static_cast<QXcbWindow *>(surface)->eglSurface()->surface();
    }

private:
    QXcbConnection *m_connection;
};
#endif

#ifndef QT_NO_OPENGL
QPlatformOpenGLContext *QXcbIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    QXcbScreen *screen = static_cast<QXcbScreen *>(context->screen()->handle());
#if defined(XCB_USE_GLX)
    return new QGLXContext(screen, context->format(), context->shareHandle());
#elif defined(XCB_USE_EGL)
    return new QEGLXcbPlatformContext(context->format(), context->shareHandle(),
        screen->connection()->egl_display(), screen->connection());
#elif defined(XCB_USE_DRI2)
    return new QDri2Context(context->format(), context->shareHandle());
#endif
    qWarning("Cannot create platform GL context, none of GLX, EGL, DRI2 is enabled");
    return 0;
}
#endif

QPlatformBackingStore *QXcbIntegration::createPlatformBackingStore(QWindow *window) const
{
    return new QXcbBackingStore(window);
}

bool QXcbIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
#if defined(QT_USE_XCB_SHARED_GRAPHICS_CACHE)
    case SharedGraphicsCache: return true;
#endif

    case ThreadedPixmaps: return true;
    case OpenGL: return true;
    case ThreadedOpenGL: return false;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QAbstractEventDispatcher *QXcbIntegration::guiThreadEventDispatcher() const
{
    return m_eventDispatcher;
}

void QXcbIntegration::moveToScreen(QWindow *window, int screen)
{
    Q_UNUSED(window);
    Q_UNUSED(screen);
}

QPlatformFontDatabase *QXcbIntegration::fontDatabase() const
{
    return m_fontDatabase.data();
}

QPlatformNativeInterface * QXcbIntegration::nativeInterface() const
{
    return m_nativeInterface.data();
}

QPlatformClipboard *QXcbIntegration::clipboard() const
{
    return m_connections.at(0)->clipboard();
}

QPlatformDrag *QXcbIntegration::drag() const
{
    return m_connections.at(0)->drag();
}

QPlatformInputContext *QXcbIntegration::inputContext() const
{
    return m_inputContext.data();
}

QPlatformAccessibility *QXcbIntegration::accessibility() const
{
    return m_accessibility.data();
}

#if defined(QT_USE_XCB_SHARED_GRAPHICS_CACHE)
static bool sharedGraphicsCacheDisabled()
{
    static const char *environmentVariable = "QT_DISABLE_SHARED_CACHE";
    static bool cacheDisabled = !qgetenv(environmentVariable).isEmpty()
            && qgetenv(environmentVariable).toInt() != 0;
    return cacheDisabled;
}

QPlatformSharedGraphicsCache *QXcbIntegration::createPlatformSharedGraphicsCache(const char *cacheId) const
{
    Q_UNUSED(cacheId);

    if (sharedGraphicsCacheDisabled())
        return 0;

    return m_sharedGraphicsCache.data();
}
#endif

QPlatformServices *QXcbIntegration::services() const
{
    return m_services.data();
}

QPlatformTheme *QXcbIntegration::platformTheme() const
{
    return m_theme.data();
}

QT_END_NAMESPACE
