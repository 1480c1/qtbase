/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QXCBWINDOW_H
#define QXCBWINDOW_H

#include <QtGui/QPlatformWindow>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QImage>

#include <xcb/xcb.h>
#include <xcb/sync.h>

#include "qxcbobject.h"

QT_BEGIN_NAMESPACE

class QXcbScreen;
class QXcbEGLSurface;

class QXcbWindow : public QXcbObject, public QPlatformWindow
{
public:
    QXcbWindow(QWindow *window);
    ~QXcbWindow();

    void setGeometry(const QRect &rect);

    QMargins frameMargins() const;

    void setVisible(bool visible);
    Qt::WindowFlags setWindowFlags(Qt::WindowFlags flags);
    Qt::WindowState setWindowState(Qt::WindowState state);
    WId winId() const;
    void setParent(const QPlatformWindow *window);

    void setWindowTitle(const QString &title);
    void raise();
    void lower();
    void propagateSizeHints();

    void requestActivateWindow();

    bool setKeyboardGrabEnabled(bool grab);
    bool setMouseGrabEnabled(bool grab);

    void setCursor(xcb_cursor_t cursor);

    QSurfaceFormat format() const;

    xcb_window_t xcb_window() const { return m_window; }
    uint depth() const { return m_depth; }
    QImage::Format imageFormat() const { return m_imageFormat; }

    void handleExposeEvent(const xcb_expose_event_t *event);
    void handleClientMessageEvent(const xcb_client_message_event_t *event);
    void handleConfigureNotifyEvent(const xcb_configure_notify_event_t *event);
    void handleMapNotifyEvent(const xcb_map_notify_event_t *event);
    void handleUnmapNotifyEvent(const xcb_unmap_notify_event_t *event);
    void handleButtonPressEvent(const xcb_button_press_event_t *event);
    void handleButtonReleaseEvent(const xcb_button_release_event_t *event);
    void handleMotionNotifyEvent(const xcb_motion_notify_event_t *event);

    void handleEnterNotifyEvent(const xcb_enter_notify_event_t *event);
    void handleLeaveNotifyEvent(const xcb_leave_notify_event_t *event);
    void handleFocusInEvent(const xcb_focus_in_event_t *event);
    void handleFocusOutEvent(const xcb_focus_out_event_t *event);
    void handlePropertyNotifyEvent(const xcb_property_notify_event_t *event);

    void handleMouseEvent(xcb_button_t detail, uint16_t state, xcb_timestamp_t time, const QPoint &local, const QPoint &global, Qt::KeyboardModifiers modifiers);

    void updateSyncRequestCounter();
    void updateNetWmUserTime(xcb_timestamp_t timestamp);
    void netWmUserTime() const;

#if defined(XCB_USE_EGL)
    QXcbEGLSurface *eglSurface() const;
#endif

private:
    void changeNetWmState(bool set, xcb_atom_t one, xcb_atom_t two = 0);
    QVector<xcb_atom_t> getNetWmState();
    void setNetWmState(const QVector<xcb_atom_t> &atoms);
    void printNetWmState(const QVector<xcb_atom_t> &state);

    void setNetWmWindowFlags(Qt::WindowFlags flags);
    void setMotifWindowFlags(Qt::WindowFlags flags);

    void updateMotifWmHintsBeforeMap();
    void updateNetWmStateBeforeMap();

    void setTransparentForMouseEvents(bool transparent);

    void create();
    void destroy();

    void show();
    void hide();

    QXcbScreen *m_screen;

    xcb_window_t m_window;

    uint m_depth;
    QImage::Format m_imageFormat;

    xcb_sync_int64_t m_syncValue;
    xcb_sync_counter_t m_syncCounter;

    Qt::WindowState m_windowState;

    bool m_mapped;
    bool m_transparent;
    xcb_window_t m_netWmUserTimeWindow;

    QSurfaceFormat m_requestedFormat;

    mutable bool m_dirtyFrameMargins;
    mutable QMargins m_frameMargins;

#if defined(XCB_USE_EGL)
    mutable QXcbEGLSurface *m_eglSurface;
#endif

    QRegion m_exposeRegion;

    xcb_visualid_t m_visualId;
};

QT_END_NAMESPACE

#endif
