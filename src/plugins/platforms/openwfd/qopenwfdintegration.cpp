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

#include "qopenwfdintegration.h"
#include "qopenwfdscreen.h"
#include "qopenwfdnativeinterface.h"
#include "qopenwfddevice.h"
#include "qopenwfdwindow.h"
#include "qopenwfdglcontext.h"
#include "qopenwfdbackingstore.h"

#include <QtPlatformSupport/private/qgenericunixprintersupport_p.h>

#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/QGuiGLContext>
#include <QtGui/QScreen>

#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>

#include <stdio.h>

#include <WF/wfd.h>

QOpenWFDIntegration::QOpenWFDIntegration()
    : QPlatformIntegration()
    , mPrinterSupport(new QGenericUnixPrinterSupport)
{
    QGuiApplicationPrivate::instance()->setEventDispatcher(createEventDispatcher());
    int numberOfDevices = wfdEnumerateDevices(0,0,0);

    WFDint devices[numberOfDevices];
    int actualNumberOfDevices = wfdEnumerateDevices(devices,numberOfDevices,0);
    Q_ASSERT(actualNumberOfDevices == numberOfDevices);

    for (int i = 0; i < actualNumberOfDevices; i++) {
        mDevices.append(new QOpenWFDDevice(this,devices[i]));
    }

    mFontDatabase = new QGenericUnixFontDatabase();
    mNativeInterface = new QOpenWFDNativeInterface;
}

QOpenWFDIntegration::~QOpenWFDIntegration()
{
    //dont delete screens since they are deleted by the devices
    qDebug() << "deleting platform integration";
    for (int i = 0; i < mDevices.size(); i++) {
        delete mDevices[i];
    }

    delete mFontDatabase;
    delete mNativeInterface;
    delete mPrinterSupport;
}

bool QOpenWFDIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformWindow *QOpenWFDIntegration::createPlatformWindow(QWindow *window) const
{
    return new QOpenWFDWindow(window);
}

QPlatformGLContext *QOpenWFDIntegration::createPlatformGLContext(QGuiGLContext *context) const
{
    QOpenWFDScreen *screen = static_cast<QOpenWFDScreen *>(context->screen()->handle());

    return new QOpenWFDGLContext(screen->port()->device());
}

QPlatformBackingStore *QOpenWFDIntegration::createPlatformBackingStore(QWindow *window) const
{
    return new QOpenWFDBackingStore(window);
}

QAbstractEventDispatcher *QOpenWFDIntegration::createEventDispatcher() const
{
    QAbstractEventDispatcher *eventDispatcher = createUnixEventDispatcher();
    return eventDispatcher;
}

QPlatformFontDatabase *QOpenWFDIntegration::fontDatabase() const
{
    return mFontDatabase;
}

QPlatformNativeInterface * QOpenWFDIntegration::nativeInterface() const
{
    return mNativeInterface;
}

QPlatformPrinterSupport * QOpenWFDIntegration::printerSupport() const
{
    return mPrinterSupport;
}

void QOpenWFDIntegration::addScreen(QOpenWFDScreen *screen)
{
    screenAdded(screen);
}
