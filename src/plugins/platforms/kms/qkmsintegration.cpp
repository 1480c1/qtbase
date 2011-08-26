/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qkmsintegration.h"
#include "qkmsdevice.h"
#include "qkmsscreen.h"
#include "qkmswindow.h"
#include "qkmsbackingstore.h"
#include "qkmscontext.h"

#include <QtPlatformSupport/private/qgenericunixprintersupport_p.h>
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/QGuiGLContext>
#include <QtGui/QScreen>

QT_BEGIN_NAMESPACE

QKmsIntegration::QKmsIntegration()
    : QPlatformIntegration(),
      m_fontDatabase(new QGenericUnixFontDatabase()),
      m_printerSupport(new QGenericUnixPrinterSupport()),
      m_eventDispatcher(createUnixEventDispatcher())
{
    QGuiApplicationPrivate::instance()->setEventDispatcher(m_eventDispatcher);
    setenv("EGL_PLATFORM", "drm",1);
    QStringList drmDevices = findDrmDevices();
    foreach (QString path, drmDevices) {
        m_devices.append(new QKmsDevice(path, this));
    }
}

QKmsIntegration::~QKmsIntegration()
{
    foreach (QKmsDevice *device, m_devices) {
        delete device;
    }
    foreach (QPlatformScreen *screen, m_screens) {
        delete screen;
    }
    delete m_printerSupport;
    delete m_fontDatabase;
}

bool QKmsIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL: return true;
    case ThreadedOpenGL: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformGLContext *QKmsIntegration::createPlatformGLContext(QGuiGLContext *context) const
{
    QKmsScreen *screen = static_cast<QKmsScreen *>(context->screen()->handle());
    return new QKmsContext(screen->device());
}

QPlatformWindow *QKmsIntegration::createPlatformWindow(QWindow *window) const
{
    return new QKmsWindow(window);
}

QPlatformBackingStore *QKmsIntegration::createPlatformBackingStore(QWindow *window) const
{
    return new QKmsBackingStore(window);
}

QPlatformFontDatabase *QKmsIntegration::fontDatabase() const
{
    return m_fontDatabase;
}

QStringList QKmsIntegration::findDrmDevices()
{
    //Return a list addresses of DRM supported devices
    //Hardcoded now, but could use udev to return a list
    //of multiple devices.
    return QStringList(QString::fromLatin1("/dev/dri/card0"));
}

void QKmsIntegration::addScreen(QKmsScreen *screen)
{
    m_screens.append(screen);
    screenAdded(screen);
}

QAbstractEventDispatcher *QKmsIntegration::guiThreadEventDispatcher() const
{
    return m_eventDispatcher;
}

QPlatformPrinterSupport *QKmsIntegration::printerSupport() const
{
    return m_printerSupport;
}

QT_END_NAMESPACE
