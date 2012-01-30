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

#ifndef QWINDOW_QPA_P_H
#define QWINDOW_QPA_P_H

#include <QtGui/qwindow.h>
#include <QtGui/qplatformwindow_qpa.h>

#include <QtCore/private/qobject_p.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


#define QWINDOWSIZE_MAX ((1<<24)-1)

class Q_GUI_EXPORT QWindowPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWindow)

public:
    enum PositionPolicy
    {
        WindowFrameInclusive,
        WindowFrameExclusive
    };

    QWindowPrivate()
        : QObjectPrivate()
        , surfaceType(QWindow::RasterSurface)
        , windowFlags(Qt::Window)
        , parentWindow(0)
        , platformWindow(0)
        , visible(false)
        , windowState(Qt::WindowNoState)
        , resizeEventPending(true)
        , positionPolicy(WindowFrameExclusive)
        , contentOrientation(Qt::PrimaryOrientation)
        , windowOrientation(Qt::PrimaryOrientation)
        , maximumSize(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX)
        , modality(Qt::NonModal)
        , transientParent(0)
        , screen(0)
    {
        isWindow = true;
    }

    ~QWindowPrivate()
    {
    }

    void maybeQuitOnLastWindowClosed();

    QPoint globalPosition() const {
        Q_Q(const QWindow);
        QPoint offset = q->pos();
        for (const QWindow *p = q->parent(); p; p = p->parent())
            offset += p->pos();
        return offset;
    }


    QWindow::SurfaceType surfaceType;
    Qt::WindowFlags windowFlags;
    QWindow *parentWindow;
    QPlatformWindow *platformWindow;
    bool visible;
    QSurfaceFormat requestedFormat;
    QString windowTitle;
    QRect geometry;
    Qt::WindowState windowState;
    bool resizeEventPending;
    PositionPolicy positionPolicy;
    Qt::ScreenOrientation contentOrientation;
    Qt::ScreenOrientation windowOrientation;

    QSize minimumSize;
    QSize maximumSize;
    QSize baseSize;
    QSize sizeIncrement;

    Qt::WindowModality modality;
    QPointer<QWindow> transientParent;
    QScreen *screen;
};


QT_END_NAMESPACE

QT_END_HEADER

#endif // QWINDOW_QPA_P_H
