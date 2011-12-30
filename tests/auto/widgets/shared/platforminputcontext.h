/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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

#include <qplatforminputcontext_qpa.h>

class PlatformInputContext : public QPlatformInputContext
{
public:
    PlatformInputContext() :
        m_animating(false),
        m_visible(false),
        m_updateCallCount(0),
        m_resetCallCount(0),
        m_commitCallCount(0),
        m_lastQueries(Qt::ImhNone),
        m_action(QInputPanel::Click),
        m_cursorPosition(0),
        m_lastEventType(QEvent::None)
    {}

    virtual QRectF keyboardRect() const { return m_keyboardRect; }
    virtual bool isAnimating() const { return m_animating; }
    virtual void reset() { m_resetCallCount++; }
    virtual void commit() { m_commitCallCount++; }

    virtual void update(Qt::InputMethodQueries queries)
    {
        m_updateCallCount++;
        m_lastQueries = queries;
    }
    virtual void invokeAction(QInputPanel::Action action, int cursorPosition)
    {
        m_action = action;
        m_cursorPosition = cursorPosition;
    }
    virtual bool filterEvent(const QEvent *event)
    {
        m_lastEventType = event->type(); return false;
    }
    virtual void showInputPanel()
    {
        m_visible = true;
    }
    virtual void hideInputPanel()
    {
        m_visible = false;
    }
    virtual bool isInputPanelVisible() const
    {
        return m_visible;
    }

    bool m_animating;
    bool m_visible;
    int m_updateCallCount;
    int m_resetCallCount;
    int m_commitCallCount;
    Qt::InputMethodQueries m_lastQueries;
    QInputPanel::Action m_action;
    int m_cursorPosition;
    int m_lastEventType;
    QRectF m_keyboardRect;
};
