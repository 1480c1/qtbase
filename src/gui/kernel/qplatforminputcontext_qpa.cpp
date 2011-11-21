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

#include <qplatforminputcontext_qpa.h>
#include <qguiapplication.h>
#include <QRect>

QT_BEGIN_NAMESPACE

/*!
    \class QPlatformInputContext
    \brief The QPlatformInputContext class abstracts the input method dependent data and composing state.

    An input method is responsible for inputting complex text that cannot
    be inputted via simple keymap. It converts a sequence of input
    events (typically key events) into a text string through the input
    method specific converting process. The class of the processes are
    widely ranging from simple finite state machine to complex text
    translator that pools a whole paragraph of a text with text
    editing capability to perform grammar and semantic analysis.

    To abstract such different input method specific intermediate
    information, Qt offers the QPlatformInputContext as base class. The
    concept is well known as 'input context' in the input method
    domain. An input context is created for a text widget in response
    to a demand. It is ensured that an input context is prepared for
    an input method before input to a text widget.

    QPlatformInputContext provides an interface the actual input methods
    can derive from by reimplementing methods.

    \sa QInputPanel
*/

/*!
    \internal
 */
QPlatformInputContext::QPlatformInputContext()
{
}

/*!
    \internal
 */
QPlatformInputContext::~QPlatformInputContext()
{
}

/*!
    Returns input context validity. Deriving implementations should return true.
 */
bool QPlatformInputContext::isValid() const
{
    return false;
}

/*!
    Method to be called when input method needs to be reset. Called by QInputPanel::reset().
    No further QInputMethodEvents should be sent as response.
 */
void QPlatformInputContext::reset()
{
}

void QPlatformInputContext::commit()
{
}

/*!
    Notification on editor updates. Called by QInputPanel::update().
 */
void QPlatformInputContext::update(Qt::InputMethodQueries)
{
}

/*!
    Called when when the word currently being composed in input item is tapped by
    the user. Input methods often use this information to offer more word
    suggestions to the user.
 */
void QPlatformInputContext::invokeAction(QInputPanel::Action action, int cursorPosition)
{
    Q_UNUSED(cursorPosition)
    // Default behavior for simple ephemeral input contexts. Some
    // complex input contexts should not be reset here.
    if (action == QInputPanel::Click)
        reset();
}

/*!
    This function can be reimplemented to filter input events.
    Return true if the event has been consumed. Otherwise, the unfiltered event will
    be forwarded to widgets as ordinary way. Although the input events have accept()
    and ignore() methods, leave it untouched.
*/
bool QPlatformInputContext::filterEvent(const QEvent *event)
{
    Q_UNUSED(event)
    return false;
}

/*!
    This function can be reimplemented to return virtual keyboard rectangle in currently active
    window coordinates. Default implementation returns invalid rectangle.
 */
QRectF QPlatformInputContext::keyboardRect() const
{
    return QRectF();
}

/*!
    Active QPlatformInputContext is responsible for providing keyboardRectangle property to QInputPanel.
    In addition of providing the value in keyboardRect function, it also needs to call this emit
    function whenever the property changes.
 */
void QPlatformInputContext::emitKeyboardRectChanged()
{
    emit qApp->inputPanel()->keyboardRectangleChanged();
}

/*!
    This function can be reimplemented to return true whenever input panel is animating
    shown or hidden. Default implementation returns false.
 */
bool QPlatformInputContext::isAnimating() const
{
    return false;
}

/*!
    Active QPlatformInputContext is responsible for providing animating property to QInputPanel.
    In addition of providing the value in isAnimation function, it also needs to call this emit
    function whenever the property changes.
 */
void QPlatformInputContext::emitAnimatingChanged()
{
    emit qApp->inputPanel()->animatingChanged();
}

/*!
    Request to show input panel.
 */
void QPlatformInputContext::showInputPanel()
{
}

/*!
    Request to hide input panel.
 */
void QPlatformInputContext::hideInputPanel()
{
}

/*!
    Returns input panel visibility status. Default implementation returns false.
 */
bool QPlatformInputContext::isInputPanelVisible() const
{
    return false;
}

/*!
    Active QPlatformInputContext is responsible for providing visible property to QInputPanel.
    In addition of providing the value in isInputPanelVisible function, it also needs to call this emit
    function whenever the property changes.
 */
void QPlatformInputContext::emitInputPanelVisibleChanged()
{
    emit qApp->inputPanel()->visibleChanged();
}

QT_END_NAMESPACE
