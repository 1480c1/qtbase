/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtGui module of the Qt Toolkit.
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
#ifndef QWINDOWSYSTEMINTERFACE_H
#define QWINDOWSYSTEMINTERFACE_H

#include <QtCore/QTime>
#include <QtGui/qwindowdefs.h>
#include <QtCore/QEvent>
#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtCore/QWeakPointer>
#include <QtCore/QMutex>
#include <QtGui/QTouchEvent>
#include <QtCore/QEventLoop>
#include <QtGui/QVector2D>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QMimeData;
class QTouchDevice;


class Q_GUI_EXPORT QWindowSystemInterface
{
public:
    static void handleMouseEvent(QWindow *w, const QPointF & local, const QPointF & global, Qt::MouseButtons b, Qt::KeyboardModifiers mods = Qt::NoModifier);
    static void handleMouseEvent(QWindow *w, ulong timestamp, const QPointF & local, const QPointF & global, Qt::MouseButtons b, Qt::KeyboardModifiers mods = Qt::NoModifier);

    static void handleKeyEvent(QWindow *w, QEvent::Type t, int k, Qt::KeyboardModifiers mods, const QString & text = QString(), bool autorep = false, ushort count = 1);
    static void handleKeyEvent(QWindow *w, ulong timestamp, QEvent::Type t, int k, Qt::KeyboardModifiers mods, const QString & text = QString(), bool autorep = false, ushort count = 1);

    static void handleExtendedKeyEvent(QWindow *w, QEvent::Type type, int key, Qt::KeyboardModifiers modifiers,
                                       quint32 nativeScanCode, quint32 nativeVirtualKey,
                                       quint32 nativeModifiers,
                                       const QString& text = QString(), bool autorep = false,
                                       ushort count = 1);
    static void handleExtendedKeyEvent(QWindow *w, ulong timestamp, QEvent::Type type, int key, Qt::KeyboardModifiers modifiers,
                                       quint32 nativeScanCode, quint32 nativeVirtualKey,
                                       quint32 nativeModifiers,
                                       const QString& text = QString(), bool autorep = false,
                                       ushort count = 1);

    static void handleWheelEvent(QWindow *w, const QPointF & local, const QPointF & global, int d, Qt::Orientation o, Qt::KeyboardModifiers mods = Qt::NoModifier);
    static void handleWheelEvent(QWindow *w, ulong timestamp, const QPointF & local, const QPointF & global, int d, Qt::Orientation o, Qt::KeyboardModifiers mods = Qt::NoModifier);

    struct TouchPoint {
        TouchPoint() : id(0), pressure(0), state(Qt::TouchPointStationary), flags(0) { }
        int id;                 // for application use
        QPointF normalPosition; // touch device coordinates, (0 to 1, 0 to 1)
        QRectF area;            // the touched area, centered at position in screen coordinates
        qreal pressure;         // 0 to 1
        Qt::TouchPointState state; //Qt::TouchPoint{Pressed|Moved|Stationary|Released}
        QVector2D velocity;     // in screen coordinate system, pixels / seconds
        QTouchEvent::TouchPoint::InfoFlags flags;
        QList<QPointF> rawPositions; // in screen coordinates
    };

    static void registerTouchDevice(QTouchDevice *device);
    static void handleTouchEvent(QWindow *w, QTouchDevice *device,
                                 const QList<struct TouchPoint> &points, Qt::KeyboardModifiers mods = Qt::NoModifier);
    static void handleTouchEvent(QWindow *w, ulong timestamp, QTouchDevice *device,
                                 const QList<struct TouchPoint> &points, Qt::KeyboardModifiers mods = Qt::NoModifier);

    static void handleGeometryChange(QWindow *w, const QRect &newRect);
    static void handleSynchronousGeometryChange(QWindow *w, const QRect &newRect);
    static void handleCloseEvent(QWindow *w);
    static void handleSynchronousCloseEvent(QWindow *w);
    static void handleEnterEvent(QWindow *w);
    static void handleLeaveEvent(QWindow *w);
    static void handleWindowActivated(QWindow *w);
    static void handleWindowStateChanged(QWindow *w, Qt::WindowState newState);

    static void handleMapEvent(QWindow *w);
    static void handleUnmapEvent(QWindow *w);

    static void handleSynchronousExposeEvent(QWindow *tlw, const QRegion &region);

    // Drag and drop. These events are sent immediately.
    static Qt::DropAction handleDrag(QWindow *w, QMimeData *dropData, const QPoint &p);
    static Qt::DropAction handleDrop(QWindow *w, QMimeData *dropData, const QPoint &p);

    // Changes to the screen
    static void handleScreenOrientationChange(QScreen *screen, Qt::ScreenOrientation newOrientation);
    static void handleScreenGeometryChange(QScreen *screen, const QRect &newGeometry);
    static void handleScreenAvailableGeometryChange(QScreen *screen, const QRect &newAvailableGeometry);
    static void handleScreenLogicalDotsPerInchChange(QScreen *screen, qreal newDpiX, qreal newDpiY);

    // For event dispatcher implementations
    static bool sendWindowSystemEvents(QAbstractEventDispatcher *eventDispatcher, QEventLoop::ProcessEventsFlags flags);
    static int windowSystemEventsQueued();
};

QT_END_NAMESPACE
QT_END_HEADER
#endif // QWINDOWSYSTEMINTERFACE_H
