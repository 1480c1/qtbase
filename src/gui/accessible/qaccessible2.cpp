/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qaccessible2.h"
#include <QtGui/QGuiApplication>
#include "qclipboard.h"
#include "qtextboundaryfinder.h"

#ifndef QT_NO_ACCESSIBILITY

QT_BEGIN_NAMESPACE

/*!
    \namespace QAccessible2
    \ingroup accessibility

    \brief The QAccessible2 namespace defines constants relating to
    IAccessible2-based interfaces

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleTextInterface

    \ingroup accessibility

    \brief The QAccessibleTextInterface class implements support for text handling.

    This interface corresponds to the IAccessibleText interface.
    It should be implemented for widgets that display more text than a plain label.
    Labels should be represented by only \l QAccessibleInterface
    and return their text as name (\l QAccessibleInterface::text() with \l QAccessible::Name as type).
    The QAccessibleTextInterface is typically for text that a screen reader
    might want to read line by line, and for widgets that support text selection and input.
    This interface is, for example, implemented for QLineEdit.

    Editable text objects should also implement \l QAccessibleEditableTextInterface.
    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \fn QAccessibleTextInterface::~QAccessibleTextInterface()
    Destructor.
*/

/*!
    \fn void QAccessibleTextInterface::addSelection(int startOffset, int endOffset)
    Select the text from \a startOffset to \a endOffset.
    The \a startOffset is the first character that will be selected.
    The \a endOffset is the first character that will not be selected.

    When the object supports multiple selections (e.g. in a word processor),
    this adds a new selection, otherwise it replaces the previous selection.

    The selection will be \a endOffset - \a startOffset characters long.
*/

/*!
    \fn QString QAccessibleTextInterface::attributes(int offset, int *startOffset, int *endOffset) const
*/

/*!
    \fn int QAccessibleTextInterface::cursorPosition() const

    Returns the current cursor position.
*/

/*!
    \fn QRect QAccessibleTextInterface::characterRect(int offset, QAccessible2::CoordinateType coordType) const
*/

/*!
    \fn int QAccessibleTextInterface::selectionCount() const

    Returns the number of selections in this text.
*/

/*!
    \fn int QAccessibleTextInterface::offsetAtPoint(const QPoint &point, QAccessible2::CoordinateType coordType) const
*/

/*!
    \fn void QAccessibleTextInterface::selection(int selectionIndex, int *startOffset, int *endOffset) const
*/

/*!
    \fn QString QAccessibleTextInterface::text(int startOffset, int endOffset) const

    Returns the text from \a startOffset to \a endOffset.
    The \a startOffset is the first character that will be returned.
    The \a endOffset is the first character that will not be returned.
*/

/*!
    \fn QString QAccessibleTextInterface::textBeforeOffset (int offset, QAccessible2::BoundaryType boundaryType,
                      int *startOffset, int *endOffset) const
*/

/*!
    \fn QString QAccessibleTextInterface::textAfterOffset(int offset, QAccessible2::BoundaryType boundaryType,
                    int *startOffset, int *endOffset) const
*/

/*!
    \fn QString QAccessibleTextInterface::textAtOffset(int offset, QAccessible2::BoundaryType boundaryType,
                 int *startOffset, int *endOffset) const
*/

/*!
    \fn void QAccessibleTextInterface::removeSelection(int selectionIndex)

    Clears the selection with \a index selectionIndex.
*/

/*!
    \fn void QAccessibleTextInterface::setCursorPosition(int position)

    Moves the cursor to \a position.
*/

/*!
    \fn void QAccessibleTextInterface::setSelection(int selectionIndex, int startOffset, int endOffset)

    Set the selection \a selectionIndex to the range from \a startOffset to \a endOffset.

    \sa addSelection(), removeSelection()
*/

/*!
    \fn int QAccessibleTextInterface::characterCount() const

    Returns the lenght of the text (total size including spaces).
*/

/*!
    \fn void QAccessibleTextInterface::scrollToSubstring(int startIndex, int endIndex)

    Ensures that the text between \a startIndex and \a endIndex is visible.
*/

/*!
    \class QAccessibleEditableTextInterface
    \ingroup accessibility

    \brief The QAccessibleEditableTextInterface class implements support for objects with editable text.

    When implementing this interface you will almost certainly also want to implement \l QAccessibleTextInterface.

    Since this interface can be implemented by means of the normal \l QAccessibleTextInterface,
    \l QAccessibleSimpleEditableTextInterface provides a convenience implementation of this interface.
    Consider inheriting \l QAccessibleSimpleEditableTextInterface instead.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleSimpleEditableTextInterface
    \ingroup accessibility

    \brief The QAccessibleSimpleEditableTextInterface class is a convenience class for
    text-based widgets.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleValueInterface
    \ingroup accessibility

    \brief The QAccessibleValueInterface class implements support for objects that manipulate a value.

    This interface should be implemented by accessible objects that represent a value.
    Examples are spinner, slider, dial and scroll bar.

    Instead of forcing the user to deal with the individual parts of the widgets, this interface
    gives an easier approach to the kind of widget it represents.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \fn QAccessibleValueInterface::~QAccessibleValueInterface()
    Destructor.
*/

/*!
    \fn QVariant QAccessibleValueInterface::currentValue() const

    Returns the current value of the widget. This is usually a double or int.
    \sa setCurrentValue()
*/

/*!
    \fn void QAccessibleValueInterface::setCurrentValue(const QVariant &value)

    Sets the \a value. If the desired \a value is out of the range of permissible values,
    this call will be ignored.

    \sa currentValue(), minimumValue(), maximumValue()
*/

/*!
    \fn QVariant QAccessibleValueInterface::maximumValue() const

    Returns the maximum value this object accepts.
    \sa minimumValue(), currentValue()
*/

/*!
    \fn QVariant QAccessibleValueInterface::minimumValue() const

    Returns the minimum value this object accepts.
    \sa maximumValue(), currentValue()
*/

/*!
    \class QAccessibleImageInterface
    \ingroup accessibility
    \internal
    \preliminary

    \brief The QAccessibleImageInterface class implements support for
    the IAccessibleImage interface.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleTableCellInterface
    \ingroup accessibility

    \brief The QAccessibleTableCellInterface class implements support for
    the IAccessibleTable2 Cell interface.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleTableInterface
    \ingroup accessibility

    \brief The QAccessibleTableInterface class implements support for
    the IAccessibleTable2 interface.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/


/*!
    \class QAccessibleActionInterface
    \ingroup accessibility

    \brief The QAccessibleActionInterface class implements support for
    invocable actions in the interface.

    Each accessible should implement the action interface if it supports any actions.
    The supported actions should use the predefined actions offered in this class unless they do not
    fit a predefined action. In that case a custom action can be added.

    When subclassing QAccessibleActionInterface you need to provide a list of actionNames which
    is the primary means to discover the available actions. Action names are never localized.
    In order to present actions to the user there are two functions that need to return localized versions
    of the name and give a description of the action. For the predefined action names use
    \l QAccessibleActionInterface::localizedActionName() and \l QAccessibleActionInterface::localizedActionDescription()
    to return their localized counterparts.

    In general you should use one of the predefined action names, unless describing an action that does not fit these:
    \table
    \header \o Action name         \o Description
    \row    \o \l checkAction()    \o checks the item (checkbox, radio button, ...)
    \row    \o \l decreaseAction() \o decrease the value of the accessible (e.g. spinbox)
    \row    \o \l increaseAction() \o increase the value of the accessible (e.g. spinbox)
    \row    \o \l pressAction()    \o press or click or activate the accessible (should correspont to clicking the object with the mouse)
    \row    \o \l setFocusAction() \o set the focus to this accessible
    \row    \o \l showMenuAction() \o show a context menu, corresponds to right-clicks
    \row    \o \l uncheckAction()  \o uncheck the item (checkbox, radio button, ...)
    \endtable

    In order to invoke the action, \l doAction() is called with an action name.

    Most widgets will simply implement \l pressAction(). This is what happens when the widget is activated by
    being clicked, space pressed or similar.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \fn QStringList QAccessibleActionInterface::actionNames() const

    Returns the list of actions supported by this accessible object.
    The actions returned should be in preferred order,
    i.e. the action that the user most likely wants to trigger should be returned first,
    while the least likely action should be returned last.

    The list does only contain actions that can be invoked.
    It won't return disabled actions, or actions associated with disabled UI controls.

    The list can be empty.

    Note that this list is not localized. For a localized representation re-implement \l localizedActionName()
    and \l localizedActionDescription()

    \sa doAction(), localizedActionName(), localizedActionDescription()
*/

/*!
    \fn QString QAccessibleActionInterface::localizedActionName(const QString &actionName) const

    Returns a localized action name of \a actionName.

    For custom actions this function has to be re-implemented.
    When using one of the default names, you can call this function in QAccessibleActionInterface
    to get the localized string.

    \sa actionNames(), localizedActionDescription()
*/

/*!
    \fn QString QAccessibleActionInterface::localizedActionDescription(const QString &actionName) const

    Returns a localized action description of the action \a actionName.

    When using one of the default names, you can call this function in QAccessibleActionInterface
    to get the localized string.

    \sa actionNames(), localizedActionName()
*/

/*!
    \fn void QAccessibleActionInterface::doAction(const QString &actionName)

    Invokes the action specified by \a actionName.
    Note that \a actionName is the non-localized name as returned by \l actionNames()
    This function is usually implemented by calling the same functions
    that other user interaction, such as clicking the object, would trigger.

    \sa actionNames()
*/

/*!
    \fn QStringList QAccessibleActionInterface::keyBindingsForAction(const QString &actionName) const

    Returns a list of the keyboard shortcuts available for invoking the action named \a actionName.

    This is important to let users learn alternative ways of using the application by emphasizing the keyboard.

    \sa actionNames()
*/


struct QAccessibleActionStrings
{
    QAccessibleActionStrings() :
        pressAction(QStringLiteral(QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Press"))),
        increaseAction(QStringLiteral(QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Increase"))),
        decreaseAction(QStringLiteral(QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Decrease"))),
        showMenuAction(QStringLiteral(QT_TRANSLATE_NOOP("QAccessibleActionInterface", "ShowMenu"))),
        setFocusAction(QStringLiteral(QT_TRANSLATE_NOOP("QAccessibleActionInterface", "SetFocus"))),
        checkAction(QStringLiteral(QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Check"))),
        uncheckAction(QStringLiteral(QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Uncheck"))) {}

    const QString pressAction;
    const QString increaseAction;
    const QString decreaseAction;
    const QString showMenuAction;
    const QString setFocusAction;
    const QString checkAction;
    const QString uncheckAction;
};

Q_GLOBAL_STATIC(QAccessibleActionStrings, accessibleActionStrings)

QString QAccessibleActionInterface::localizedActionName(const QString &actionName) const
{
    return QAccessibleActionInterface::tr(qPrintable(actionName));
}

QString QAccessibleActionInterface::localizedActionDescription(const QString &actionName) const
{
    const QAccessibleActionStrings *strings = accessibleActionStrings();
    if (actionName == strings->pressAction)
        return tr("Triggers the action");
    else if (actionName == strings->increaseAction)
        return tr("Increase the value");
    else if (actionName == strings->decreaseAction)
        return tr("Decrease the value");
    else if (actionName == strings->showMenuAction)
        return tr("Shows the menu");
    else if (actionName == strings->setFocusAction)
        return tr("Sets the focus");
    else if (actionName == strings->checkAction)
        return tr("Checks the checkbox");
    else if (actionName == strings->uncheckAction)
        return tr("Unchecks the checkbox");

    return QString();
}

/*!
    Returns the name of the press default action.
    \sa actionNames(), localizedActionName()
  */
const QString &QAccessibleActionInterface::pressAction()
{
    return accessibleActionStrings()->pressAction;
}

/*!
    Returns the name of the increase default action.
    \sa actionNames(), localizedActionName()
  */
const QString &QAccessibleActionInterface::increaseAction()
{
    return accessibleActionStrings()->increaseAction;
}

/*!
    Returns the name of the decrease default action.
    \sa actionNames(), localizedActionName()
  */
const QString &QAccessibleActionInterface::decreaseAction()
{
    return accessibleActionStrings()->decreaseAction;
}

/*!
    Returns the name of the show menu default action.
    \sa actionNames(), localizedActionName()
  */
const QString &QAccessibleActionInterface::showMenuAction()
{
    return accessibleActionStrings()->showMenuAction;
}

/*!
    Returns the name of the set focus default action.
    \sa actionNames(), localizedActionName()
  */
const QString &QAccessibleActionInterface::setFocusAction()
{
    return accessibleActionStrings()->setFocusAction;
}

/*!
    Returns the name of the check default action.
    \sa actionNames(), localizedActionName()
  */
const QString &QAccessibleActionInterface::checkAction()
{
    return accessibleActionStrings()->checkAction;
}

/*!
    Returns the name of the uncheck default action.
    \sa actionNames(), localizedActionName()
  */
const QString &QAccessibleActionInterface::uncheckAction()
{
    return accessibleActionStrings()->uncheckAction;
}

/*!
  \internal
*/
QString Q_GUI_EXPORT qTextBeforeOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
        int *startOffset, int *endOffset, const QString& text)
{
    QTextBoundaryFinder::BoundaryType type;
    switch (boundaryType) {
    case QAccessible2::CharBoundary:
        type = QTextBoundaryFinder::Grapheme;
        break;
    case QAccessible2::WordBoundary:
        type = QTextBoundaryFinder::Word;
        break;
    case QAccessible2::SentenceBoundary:
        type = QTextBoundaryFinder::Sentence;
        break;
    default:
        // in any other case return the whole line
        *startOffset = 0;
        *endOffset = text.length();
        return text;
    }

    QTextBoundaryFinder boundary(type, text);
    boundary.setPosition(offset);

    if (!boundary.isAtBoundary()) {
        boundary.toPreviousBoundary();
    }
    boundary.toPreviousBoundary();
    *startOffset = boundary.position();
    boundary.toNextBoundary();
    *endOffset = boundary.position();

    return text.mid(*startOffset, *endOffset - *startOffset);
}

/*!
  \internal
*/
QString Q_GUI_EXPORT qTextAfterOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
        int *startOffset, int *endOffset, const QString& text)
{
    QTextBoundaryFinder::BoundaryType type;
    switch (boundaryType) {
    case QAccessible2::CharBoundary:
        type = QTextBoundaryFinder::Grapheme;
        break;
    case QAccessible2::WordBoundary:
        type = QTextBoundaryFinder::Word;
        break;
    case QAccessible2::SentenceBoundary:
        type = QTextBoundaryFinder::Sentence;
        break;
    default:
        // in any other case return the whole line
        *startOffset = 0;
        *endOffset = text.length();
        return text;
    }

    QTextBoundaryFinder boundary(type, text);
    boundary.setPosition(offset);

    boundary.toNextBoundary();
    *startOffset = boundary.position();
    boundary.toNextBoundary();
    *endOffset = boundary.position();

    return text.mid(*startOffset, *endOffset - *startOffset);
}

/*!
  \internal
*/
QString Q_GUI_EXPORT qTextAtOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
        int *startOffset, int *endOffset, const QString& text)
{
    QTextBoundaryFinder::BoundaryType type;
    switch (boundaryType) {
    case QAccessible2::CharBoundary:
        type = QTextBoundaryFinder::Grapheme;
        break;
    case QAccessible2::WordBoundary:
        type = QTextBoundaryFinder::Word;
        break;
    case QAccessible2::SentenceBoundary:
        type = QTextBoundaryFinder::Sentence;
        break;
    default:
        // in any other case return the whole line
        *startOffset = 0;
        *endOffset = text.length();
        return text;
    }

    QTextBoundaryFinder boundary(type, text);
    boundary.setPosition(offset);

    if (!boundary.isAtBoundary()) {
        boundary.toPreviousBoundary();
    }
    *startOffset = boundary.position();
    boundary.toNextBoundary();
    *endOffset = boundary.position();

    return text.mid(*startOffset, *endOffset - *startOffset);
}

QAccessibleSimpleEditableTextInterface::QAccessibleSimpleEditableTextInterface(
                QAccessibleInterface *accessibleInterface)
    : iface(accessibleInterface)
{
    Q_ASSERT(iface);
}

#ifndef QT_NO_CLIPBOARD
static QString textForRange(QAccessibleInterface *iface, int startOffset, int endOffset)
{
    return iface->text(QAccessible::Value).mid(startOffset, endOffset - startOffset);
}
#endif

void QAccessibleSimpleEditableTextInterface::copyText(int startOffset, int endOffset) const
{
#ifdef QT_NO_CLIPBOARD
    Q_UNUSED(startOffset);
    Q_UNUSED(endOffset);
#else
    QGuiApplication::clipboard()->setText(textForRange(iface, startOffset, endOffset));
#endif
}

void QAccessibleSimpleEditableTextInterface::deleteText(int startOffset, int endOffset)
{
    QString txt = iface->text(QAccessible::Value);
    txt.remove(startOffset, endOffset - startOffset);
    iface->setText(QAccessible::Value, txt);
}

void QAccessibleSimpleEditableTextInterface::insertText(int offset, const QString &text)
{
    QString txt = iface->text(QAccessible::Value);
    txt.insert(offset, text);
    iface->setText(QAccessible::Value, txt);
}

void QAccessibleSimpleEditableTextInterface::cutText(int startOffset, int endOffset)
{
#ifdef QT_NO_CLIPBOARD
    Q_UNUSED(startOffset);
    Q_UNUSED(endOffset);
#else
    QString sub = textForRange(iface, startOffset, endOffset);
    deleteText(startOffset, endOffset);
    QGuiApplication::clipboard()->setText(sub);
#endif
}

void QAccessibleSimpleEditableTextInterface::pasteText(int offset)
{
#ifdef QT_NO_CLIPBOARD
    Q_UNUSED(offset);
#else
    QString txt = iface->text(QAccessible::Value);
    txt.insert(offset, QGuiApplication::clipboard()->text());
    iface->setText(QAccessible::Value, txt);
#endif
}

void QAccessibleSimpleEditableTextInterface::replaceText(int startOffset, int endOffset, const QString &text)
{
    QString txt = iface->text(QAccessible::Value);
    txt.replace(startOffset, endOffset - startOffset, text);
    iface->setText(QAccessible::Value, txt);
}

QT_END_NAMESPACE

#endif // QT_NO_ACCESSIBILITY
