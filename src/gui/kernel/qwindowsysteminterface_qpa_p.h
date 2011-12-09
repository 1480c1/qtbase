/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QWINDOWSYSTEMINTERFACE_QPA_P_H
#define QWINDOWSYSTEMINTERFACE_QPA_P_H

#include "qwindowsysteminterface_qpa.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QWindowSystemInterfacePrivate {
public:
    enum EventType {
        Close,
        GeometryChange,
        Enter,
        Leave,
        ActivatedWindow,
        WindowStateChanged,
        Mouse,
        Wheel,
        Key,
        Touch,
        ScreenOrientation,
        ScreenGeometry,
        ScreenAvailableGeometry,
        ScreenLogicalDotsPerInch,
        Map,
        Unmap,
        Expose
    };

    class WindowSystemEvent {
    public:
        explicit WindowSystemEvent(EventType t)
            : type(t) { }
        EventType type;
    };

    class CloseEvent : public WindowSystemEvent {
    public:
        explicit CloseEvent(QWindow *w)
            : WindowSystemEvent(Close), window(w) { }
        QWeakPointer<QWindow> window;
    };

    class GeometryChangeEvent : public WindowSystemEvent {
    public:
        GeometryChangeEvent(QWindow *tlw, const QRect &newGeometry)
            : WindowSystemEvent(GeometryChange), tlw(tlw), newGeometry(newGeometry)
        { }
        QWeakPointer<QWindow> tlw;
        QRect newGeometry;
    };

    class EnterEvent : public WindowSystemEvent {
    public:
        explicit EnterEvent(QWindow *enter)
            : WindowSystemEvent(Enter), enter(enter)
        { }
        QWeakPointer<QWindow> enter;
    };

    class LeaveEvent : public WindowSystemEvent {
    public:
        explicit LeaveEvent(QWindow *leave)
            : WindowSystemEvent(Leave), leave(leave)
        { }
        QWeakPointer<QWindow> leave;
    };

    class ActivatedWindowEvent : public WindowSystemEvent {
    public:
        explicit ActivatedWindowEvent(QWindow *activatedWindow)
            : WindowSystemEvent(ActivatedWindow), activated(activatedWindow)
        { }
        QWeakPointer<QWindow> activated;
    };

    class WindowStateChangedEvent : public WindowSystemEvent {
    public:
        WindowStateChangedEvent(QWindow *_window, Qt::WindowState _newState)
            : WindowSystemEvent(WindowStateChanged), window(_window), newState(_newState)
        { }

        QWeakPointer<QWindow> window;
        Qt::WindowState newState;
    };

    class UserEvent : public WindowSystemEvent {
    public:
        UserEvent(QWindow * w, ulong time, EventType t)
            : WindowSystemEvent(t), window(w), nullWindow(w == 0), timestamp(time) { }
        QWeakPointer<QWindow> window;
        bool nullWindow;
        unsigned long timestamp;
    };

    class InputEvent: public UserEvent {
    public:
        InputEvent(QWindow * w, ulong time, EventType t, Qt::KeyboardModifiers mods)
            : UserEvent(w, time, t), modifiers(mods) {}
        Qt::KeyboardModifiers modifiers;
    };

    class MouseEvent : public InputEvent {
    public:
        MouseEvent(QWindow * w, ulong time, const QPointF & local, const QPointF & global,
                   Qt::MouseButtons b, Qt::KeyboardModifiers mods)
            : InputEvent(w, time, Mouse, mods), localPos(local), globalPos(global), buttons(b) { }
        QPointF localPos;
        QPointF globalPos;
        Qt::MouseButtons buttons;
    };

    class WheelEvent : public InputEvent {
    public:
        WheelEvent(QWindow *w, ulong time, const QPointF & local, const QPointF & global, int d,
                   Qt::Orientation o, Qt::KeyboardModifiers mods)
            : InputEvent(w, time, Wheel, mods), delta(d), localPos(local), globalPos(global), orient(o) { }
        int delta;
        QPointF localPos;
        QPointF globalPos;
        Qt::Orientation orient;
    };

    class KeyEvent : public InputEvent {
    public:
        KeyEvent(QWindow *w, ulong time, QEvent::Type t, int k, Qt::KeyboardModifiers mods, const QString & text = QString(), bool autorep = false, ushort count = 1)
            :InputEvent(w, time, Key, mods), key(k), unicode(text), repeat(autorep),
             repeatCount(count), keyType(t),
             nativeScanCode(0), nativeVirtualKey(0), nativeModifiers(0) { }
        KeyEvent(QWindow *w, ulong time, QEvent::Type t, int k, Qt::KeyboardModifiers mods,
                 quint32 nativeSC, quint32 nativeVK, quint32 nativeMods,
                 const QString & text = QString(), bool autorep = false, ushort count = 1)
            :InputEvent(w, time, Key, mods), key(k), unicode(text), repeat(autorep),
             repeatCount(count), keyType(t),
             nativeScanCode(nativeSC), nativeVirtualKey(nativeVK), nativeModifiers(nativeMods) { }
        int key;
        QString unicode;
        bool repeat;
        ushort repeatCount;
        QEvent::Type keyType;
        quint32 nativeScanCode;
        quint32 nativeVirtualKey;
        quint32 nativeModifiers;
    };

    class TouchEvent : public InputEvent {
    public:
        TouchEvent(QWindow *w, ulong time, QEvent::Type t, QTouchDevice *dev,
                   const QList<QTouchEvent::TouchPoint> &p, Qt::KeyboardModifiers mods)
            :InputEvent(w, time, Touch, mods), device(dev), points(p), touchType(t) { }
        QTouchDevice *device;
        QList<QTouchEvent::TouchPoint> points;
        QEvent::Type touchType;
    };

    class ScreenOrientationEvent : public WindowSystemEvent {
    public:
        ScreenOrientationEvent(QScreen *s)
            : WindowSystemEvent(ScreenOrientation), screen(s) { }
        QWeakPointer<QScreen> screen;
    };

    class ScreenGeometryEvent : public WindowSystemEvent {
    public:
        ScreenGeometryEvent(QScreen *s)
            : WindowSystemEvent(ScreenGeometry), screen(s) { }
        QWeakPointer<QScreen> screen;
    };

    class ScreenAvailableGeometryEvent : public WindowSystemEvent {
    public:
        ScreenAvailableGeometryEvent(QScreen *s)
            : WindowSystemEvent(ScreenAvailableGeometry), screen(s) { }
        QWeakPointer<QScreen> screen;
    };

    class ScreenLogicalDotsPerInchEvent : public WindowSystemEvent {
    public:
        ScreenLogicalDotsPerInchEvent(QScreen *s)
            : WindowSystemEvent(ScreenLogicalDotsPerInch), screen(s) { }
        QWeakPointer<QScreen> screen;
    };

    class MapEvent : public WindowSystemEvent {
    public:
        MapEvent(QWindow *mapped)
            : WindowSystemEvent(Map), mapped(mapped)
        { }
        QWeakPointer<QWindow> mapped;
    };

    class UnmapEvent : public WindowSystemEvent {
    public:
        UnmapEvent(QWindow *unmapped)
            : WindowSystemEvent(Unmap), unmapped(unmapped)
        { }
        QWeakPointer<QWindow> unmapped;
    };

    class ExposeEvent : public WindowSystemEvent {
    public:
        ExposeEvent(QWindow *exposed, const QRegion &region)
            : WindowSystemEvent(Expose), exposed(exposed), region(region)
        { }
        QWeakPointer<QWindow> exposed;
        QRegion region;
    };

    static QList<WindowSystemEvent *> windowSystemEventQueue;
    static QMutex queueMutex;

    static int windowSystemEventsQueued();
    static WindowSystemEvent * getWindowSystemEvent();
    static void queueWindowSystemEvent(WindowSystemEvent *ev);

    static QTime eventTime;
};

QT_END_HEADER
QT_END_NAMESPACE

#endif // QWINDOWSYSTEMINTERFACE_QPA_P_H
