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

#ifndef QWINDOWSDRAG_H
#define QWINDOWSDRAG_H

#include "qwindowsinternalmimedata.h"

#include <QtGui/QPlatformDrag>

QT_BEGIN_NAMESPACE

class QWindowsDropMimeData : public QWindowsInternalMimeData {
public:
    QWindowsDropMimeData() {}
    virtual IDataObject *retrieveDataObject() const;
};

class QWindowsOleDropTarget : public IDropTarget
{
public:
    explicit QWindowsOleDropTarget(QWindow *w);
    virtual ~QWindowsOleDropTarget();

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // IDropTarget methods
    STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

private:
    inline QWindow *findDragOverWindow(const POINTL &pt) const;
    void sendDragEnterEvent(QWindow *to, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

    ULONG m_refs;
    QWindow *const m_window;
    QWindow *m_currentWindow;
    QRect m_answerRect;
    QPoint m_lastPoint;
    DWORD m_chosenEffect;
    DWORD m_lastKeyState;
};

class QWindowsDrag : public QPlatformDrag
{
public:
    QWindowsDrag();
    virtual ~QWindowsDrag();

    virtual QMimeData *platformDropData() { return &m_dropData; }

    virtual void startDrag();
    virtual void move(const QMouseEvent *me);
    virtual void drop(const QMouseEvent *me);
    virtual void cancel();

    static QWindowsDrag *instance();

    IDataObject *dropDataObject() const             { return m_dropDataObject; }
    void setDropDataObject(IDataObject *dataObject) { m_dropDataObject = dataObject; }
    void releaseDropDataObject();

    bool dragBeingCancelled() const { return m_dragBeingCancelled; }

    QPixmap defaultCursor(Qt::DropAction action) const;

private:
    QWindowsDropMimeData m_dropData;
    IDataObject *m_dropDataObject;
    bool m_dragBeingCancelled;

    mutable QPixmap m_copyDragCursor;
    mutable QPixmap m_moveDragCursor;
    mutable QPixmap m_linkDragCursor;
    mutable QPixmap m_ignoreDragCursor;
};

QT_END_NAMESPACE

#endif // QWINDOWSDRAG_H
