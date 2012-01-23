/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QXCBINTEGRATION_H
#define QXCBINTEGRATION_H

#include <QtGui/QPlatformIntegration>
#include <QtGui/QPlatformScreen>

QT_BEGIN_NAMESPACE

class QXcbConnection;
class QAbstractEventDispatcher;
class QXcbNativeInterface;

class QXcbIntegration : public QPlatformIntegration
{
public:
    QXcbIntegration(const QStringList &parameters);
    ~QXcbIntegration();

    QPlatformWindow *createPlatformWindow(QWindow *window) const;
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const;

    bool hasCapability(Capability cap) const;
    QAbstractEventDispatcher *guiThreadEventDispatcher() const;

    void moveToScreen(QWindow *window, int screen);

    QPlatformFontDatabase *fontDatabase() const;

    QPlatformNativeInterface *nativeInterface()const;

    QPlatformClipboard *clipboard() const;
    QPlatformDrag *drag() const;

    QPlatformInputContext *inputContext() const;

    QPlatformAccessibility *accessibility() const;

#if defined(QT_USE_XCB_SHARED_GRAPHICS_CACHE)
    QPlatformSharedGraphicsCache *createPlatformSharedGraphicsCache(const char *cacheId) const;
#endif

private:
    QList<QXcbConnection *> m_connections;

    QScopedPointer<QPlatformFontDatabase> m_fontDatabase;
    QScopedPointer<QXcbNativeInterface> m_nativeInterface;

    QScopedPointer<QPlatformInputContext> m_inputContext;
    QAbstractEventDispatcher *m_eventDispatcher;

    QScopedPointer<QPlatformAccessibility> m_accessibility;

#if defined(QT_USE_XCB_SHARED_GRAPHICS_CACHE)
    QScopedPointer<QPlatformSharedGraphicsCache> m_sharedGraphicsCache;
#endif
};

QT_END_NAMESPACE

#endif
