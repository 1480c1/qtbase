/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qwidgetwindow_qpa_p.h"

#include "private/qwidget_p.h"
#include "private/qapplication_p.h"

QT_BEGIN_NAMESPACE

QWidget *qt_button_down = 0; // widget got last button-down

// popup control
QWidget *qt_popup_down = 0; // popup that contains the pressed widget
extern int openPopupCount;
bool qt_replay_popup_mouse_event = false;
extern bool qt_try_modal(QWidget *widget, QEvent::Type type);

QWidgetWindow::QWidgetWindow(QWidget *widget)
    : m_widget(widget)
{
}

bool QWidgetWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Close:
        handleCloseEvent(static_cast<QCloseEvent *>(event));
        return true;

    case QEvent::Enter:
    case QEvent::Leave:
        handleEnterLeaveEvent(event);
        return true;

    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        handleKeyEvent(static_cast<QKeyEvent *>(event));
        return true;

    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
        handleMouseEvent(static_cast<QMouseEvent *>(event));
        return true;

    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        handleTouchEvent(static_cast<QTouchEvent *>(event));
        return true;

    case QEvent::Move:
        handleMoveEvent(static_cast<QMoveEvent *>(event));
        return true;

    case QEvent::Resize:
        handleResizeEvent(static_cast<QResizeEvent *>(event));
        return true;

    case QEvent::Wheel:
        handleWheelEvent(static_cast<QWheelEvent *>(event));
        return true;

    case QEvent::DragEnter:
    case QEvent::DragLeave:
    case QEvent::DragMove:
    case QEvent::Drop:
        handleDragEvent(event);
        break;

    case QEvent::Map:
        m_widget->setAttribute(Qt::WA_Mapped);
        m_widget->d_func()->syncBackingStore();
        return true;

    case QEvent::Unmap:
        m_widget->setAttribute(Qt::WA_Mapped, false);
        return true;

    case QEvent::Expose:
        handleExposeEvent(static_cast<QExposeEvent *>(event));
        return true;

    case QEvent::WindowStateChange:
        handleWindowStateChangedEvent(static_cast<QWindowStateChangeEvent *>(event));
        return true;

    default:
        break;
    }

    return m_widget->event(event) || QWindow::event(event);
}

QPointer<QWidget> qt_last_mouse_receiver = 0;

void QWidgetWindow::handleEnterLeaveEvent(QEvent *event)
{
    if (event->type() == QEvent::Leave) {
        QWidget *leave = qt_last_mouse_receiver ? qt_last_mouse_receiver.data() : m_widget;
        QApplicationPrivate::dispatchEnterLeave(0, leave);
        qt_last_mouse_receiver = 0;
    } else {
        QApplicationPrivate::dispatchEnterLeave(m_widget, 0);
        qt_last_mouse_receiver = m_widget;
    }
}

void QWidgetWindow::handleMouseEvent(QMouseEvent *event)
{
    if (qApp->d_func()->inPopupMode()) {
        QWidget *activePopupWidget = qApp->activePopupWidget();
        QWidget *popup = activePopupWidget;
        QPoint mapped = event->pos();
        if (popup != m_widget)
            mapped = popup->mapFromGlobal(event->globalPos());
        bool releaseAfter = false;
        QWidget *popupChild  = popup->childAt(mapped);

        if (popup != qt_popup_down) {
            qt_button_down = 0;
            qt_popup_down = 0;
        }

        switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick:
            qt_button_down = popupChild;
            qt_popup_down = popup;
            break;
        case QEvent::MouseButtonRelease:
            releaseAfter = true;
            break;
        default:
            break; // nothing for mouse move
        }

        int oldOpenPopupCount = openPopupCount;

        if (popup->isEnabled()) {
            // deliver event
            qt_replay_popup_mouse_event = false;
            QWidget *receiver = popup;
            QPoint widgetPos = mapped;
            if (qt_button_down)
                receiver = qt_button_down;
            else if (popupChild)
                receiver = popupChild;
            if (receiver != popup)
                widgetPos = receiver->mapFromGlobal(event->globalPos());
            QWidget *alien = m_widget->childAt(m_widget->mapFromGlobal(event->globalPos()));
            QMouseEvent e(event->type(), widgetPos, event->windowPos(), event->screenPos(), event->button(), event->buttons(), event->modifiers());
            e.setTimestamp(event->timestamp());
            QApplicationPrivate::sendMouseEvent(receiver, &e, alien, m_widget, &qt_button_down, qt_last_mouse_receiver);
        } else {
            // close disabled popups when a mouse button is pressed or released
            switch (event->type()) {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonDblClick:
            case QEvent::MouseButtonRelease:
                popup->close();
                break;
            default:
                break;
            }
        }

        if (qApp->activePopupWidget() != activePopupWidget
            && qt_replay_popup_mouse_event) {
            if (m_widget->windowType() != Qt::Popup)
                qt_button_down = 0;
            qt_replay_popup_mouse_event = false;
        } else if (event->type() == QEvent::MouseButtonPress
                   && event->button() == Qt::RightButton
                   && (openPopupCount == oldOpenPopupCount)) {
            QWidget *popupEvent = popup;
            if (qt_button_down)
                popupEvent = qt_button_down;
            else if(popupChild)
                popupEvent = popupChild;
            QContextMenuEvent e(QContextMenuEvent::Mouse, mapped, event->globalPos(), event->modifiers());
            QApplication::sendSpontaneousEvent(popupEvent, &e);
        }

        if (releaseAfter) {
            qt_button_down = 0;
            qt_popup_down = 0;
        }
        return;
    }

    // modal event handling
    if (QApplicationPrivate::instance()->modalState() && !qt_try_modal(m_widget, event->type()))
        return;

    // which child should have it?
    QWidget *widget = m_widget->childAt(event->pos());
    QPoint mapped = event->pos();

    if (!widget)
        widget = m_widget;

    if (event->type() == QEvent::MouseButtonPress && !qt_button_down)
        qt_button_down = widget;

    QWidget *receiver = QApplicationPrivate::pickMouseReceiver(m_widget, event->globalPos(), mapped, event->type(), event->buttons(),
                                                               qt_button_down, widget);

    if (!receiver) {
        if (event->type() == QEvent::MouseButtonRelease)
            QApplicationPrivate::mouse_buttons &= ~event->button();
        return;
    }

    QMouseEvent translated(event->type(), mapped, event->windowPos(), event->screenPos(), event->button(), event->buttons(), event->modifiers());
    translated.setTimestamp(event->timestamp());
    QApplicationPrivate::sendMouseEvent(receiver, &translated, widget, m_widget, &qt_button_down,
                                        qt_last_mouse_receiver);

    if (event->type() == QEvent::MouseButtonPress && event->button() == Qt::RightButton) {
        QContextMenuEvent e(QContextMenuEvent::Mouse, mapped, event->globalPos(), event->modifiers());
        QGuiApplication::sendSpontaneousEvent(receiver, &e);
    }
}

void QWidgetWindow::handleTouchEvent(QTouchEvent *event)
{
    QApplicationPrivate::translateRawTouchEvent(m_widget, event->deviceType(), event->touchPoints());
}

void QWidgetWindow::handleKeyEvent(QKeyEvent *event)
{
    if (QApplicationPrivate::instance()->modalState() && !qt_try_modal(m_widget, event->type()))
        return;

    QWidget *widget = m_widget->focusWidget();

    if (!widget)
        widget = m_widget;

    QGuiApplication::sendSpontaneousEvent(widget, event);
}

void QWidgetWindow::updateGeometry()
{
    if (m_widget->testAttribute(Qt::WA_OutsideWSRange))
        return;

    QMargins margins = frameMargins();

    m_widget->data->crect = geometry();
    m_widget->d_func()->topData()->frameStrut.setCoords(margins.left(), margins.top(), margins.right(), margins.bottom());
}

void QWidgetWindow::handleMoveEvent(QMoveEvent *event)
{
    updateGeometry();
    QGuiApplication::sendSpontaneousEvent(m_widget, event);
}

void QWidgetWindow::handleResizeEvent(QResizeEvent *event)
{
    QSize oldSize = m_widget->data->crect.size();

    updateGeometry();
    QGuiApplication::sendSpontaneousEvent(m_widget, event);

    if (m_widget->d_func()->paintOnScreen()) {
        QRegion updateRegion(geometry());
        if (m_widget->testAttribute(Qt::WA_StaticContents))
            updateRegion -= QRect(0, 0, oldSize.width(), oldSize.height());
        m_widget->d_func()->syncBackingStore(updateRegion);
    } else {
        m_widget->d_func()->syncBackingStore();
    }
}

void QWidgetWindow::handleCloseEvent(QCloseEvent *)
{
    m_widget->d_func()->close_helper(QWidgetPrivate::CloseWithSpontaneousEvent);
}

void QWidgetWindow::handleWheelEvent(QWheelEvent *event)
{
    if (QApplicationPrivate::instance()->modalState() && !qt_try_modal(m_widget, event->type()))
        return;

    // which child should have it?
    QWidget *widget = m_widget->childAt(event->pos());

    if (!widget)
        widget = m_widget;

    QPoint mapped = widget->mapFrom(m_widget, event->pos());

    QWheelEvent translated(mapped, event->globalPos(), event->delta(), event->buttons(), event->modifiers(), event->orientation());
    QGuiApplication::sendSpontaneousEvent(widget, &translated);
}

void QWidgetWindow::handleDragEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::DragEnter:
        Q_ASSERT(!m_dragTarget);
        // fall through
    case QEvent::DragMove:
    {
        QDragMoveEvent *de = static_cast<QDragMoveEvent *>(event);
        QWidget *widget = m_widget->childAt(de->pos());
        if (!widget)
            widget = m_widget;

        if (widget != m_dragTarget.data()) {
            if (m_dragTarget.data()) {
                QDragLeaveEvent le;
                QGuiApplication::sendSpontaneousEvent(m_dragTarget.data(), &le);
            }
            m_dragTarget = widget;
            QPoint mapped = widget->mapFrom(m_widget, de->pos());
            QDragEnterEvent translated(mapped, de->possibleActions(), de->mimeData(), de->mouseButtons(), de->keyboardModifiers());
            QGuiApplication::sendSpontaneousEvent(widget, &translated);
            if (translated.isAccepted())
                event->accept();
            de->setDropAction(translated.dropAction());
        } else {
            Q_ASSERT(event->type() == QEvent::DragMove);
            QPoint mapped = widget->mapFrom(m_widget, de->pos());
            QDragMoveEvent translated(mapped, de->possibleActions(), de->mimeData(), de->mouseButtons(), de->keyboardModifiers());
            translated.setDropAction(de->dropAction());
            QGuiApplication::sendSpontaneousEvent(widget, &translated);
            if (translated.isAccepted())
                event->accept();
            de->setDropAction(translated.dropAction());
        }
        break;
    }
    case QEvent::DragLeave:
        if (m_dragTarget)
            QGuiApplication::sendSpontaneousEvent(m_dragTarget.data(), event);
        m_dragTarget = (QWidget *)0;
        break;
    case QEvent::Drop:
    {
        QDropEvent *de = static_cast<QDropEvent *>(event);
        QPoint mapped = m_dragTarget.data()->mapFrom(m_widget, de->pos());
        QDropEvent translated(mapped, de->possibleActions(), de->mimeData(), de->mouseButtons(), de->keyboardModifiers());
        QGuiApplication::sendSpontaneousEvent(m_dragTarget.data(), &translated);
        if (translated.isAccepted())
            event->accept();
        de->setDropAction(translated.dropAction());
        m_dragTarget = (QWidget *)0;
    }
    default:
        break;
    }
}

void QWidgetWindow::handleExposeEvent(QExposeEvent *event)
{
    m_widget->d_func()->syncBackingStore(event->region());
}

void QWidgetWindow::handleWindowStateChangedEvent(QWindowStateChangeEvent *event)
{
    // QWindow does currently not know 'active'.
    Qt::WindowStates eventState = event->oldState();
    if (m_widget->windowState() & Qt::WindowActive)
        eventState |= Qt::WindowActive;
    QWindowStateChangeEvent widgetEvent(eventState);
    QGuiApplication::sendSpontaneousEvent(m_widget, &widgetEvent);
}

QT_END_NAMESPACE
