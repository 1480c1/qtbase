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

#include "simplewidgets.h"

#include <qabstractbutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>
#include <qmenu.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qstyle.h>
#include <qstyleoption.h>

#ifdef Q_OS_MAC
#include <qfocusframe.h>
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ACCESSIBILITY

using namespace QAccessible2;
extern QList<QWidget*> childWidgets(const QWidget *widget, bool includeTopLevel = false);

QString Q_GUI_EXPORT qt_accStripAmp(const QString &text);
QString Q_GUI_EXPORT qt_accHotKey(const QString &text);

QString Q_GUI_EXPORT qTextBeforeOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
        int *startOffset, int *endOffset, const QString& text);
QString Q_GUI_EXPORT qTextAtOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
        int *startOffset, int *endOffset, const QString& text);
QString Q_GUI_EXPORT qTextAfterOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
        int *startOffset, int *endOffset, const QString& text);

/*!
  \class QAccessibleButton
  \brief The QAccessibleButton class implements the QAccessibleInterface for button type widgets.
  \internal

  \ingroup accessibility
*/

/*!
  Creates a QAccessibleButton object for \a w.
  \a role is propagated to the QAccessibleWidget constructor.
*/
QAccessibleButton::QAccessibleButton(QWidget *w, QAccessible::Role role)
: QAccessibleWidget(w, role)
{
    Q_ASSERT(button());
    if (button()->isCheckable())
        addControllingSignal(QLatin1String("toggled(bool)"));
    else
        addControllingSignal(QLatin1String("clicked()"));
}

/*! Returns the button. */
QAbstractButton *QAccessibleButton::button() const
{
    return qobject_cast<QAbstractButton*>(object());
}

/*! \reimp */
QString QAccessibleButton::text(QAccessible::Text t) const
{
    QString str;
    switch (t) {
    case QAccessible::Accelerator:
    {
#ifndef QT_NO_SHORTCUT
        QPushButton *pb = qobject_cast<QPushButton*>(object());
        if (pb && pb->isDefault())
            str = (QString)QKeySequence(Qt::Key_Enter);
#endif
        if (str.isEmpty())
            str = qt_accHotKey(button()->text());
    }
         break;
    case QAccessible::Name:
        str = widget()->accessibleName();
        if (str.isEmpty())
            str = button()->text();
        break;
    default:
        break;
    }
    if (str.isEmpty())
        str = QAccessibleWidget::text(t);
    return qt_accStripAmp(str);
}

QAccessible::State QAccessibleButton::state() const
{
    QAccessible::State state = QAccessibleWidget::state();

    QAbstractButton *b = button();
    QCheckBox *cb = qobject_cast<QCheckBox *>(b);
    if (b->isChecked())
        state.checked = true;
    else if (cb && cb->checkState() == Qt::PartiallyChecked)
        state.checkStateMixed = true;
    if (b->isDown())
        state.pressed = true;
    QPushButton *pb = qobject_cast<QPushButton*>(b);
    if (pb) {
        if (pb->isDefault())
            state.defaultButton = true;
#ifndef QT_NO_MENU
        if (pb->menu())
            state.hasPopup = true;
#endif
    }

    return state;
}

QStringList QAccessibleButton::actionNames() const
{
    QStringList names;
    if (widget()->isEnabled()) {
        switch (role()) {
        case QAccessible::ButtonMenu:
            names << showMenuAction();
            break;
        case QAccessible::RadioButton:
            names << checkAction();
            break;
        default:
            if (button()->isCheckable()) {
                if (state().checked) {
                    names <<  uncheckAction();
                } else {
                    // FIXME
    //                QCheckBox *cb = qobject_cast<QCheckBox*>(object());
    //                if (!cb || !cb->isTristate() || cb->checkState() == Qt::PartiallyChecked)
                    names <<  checkAction();
                }
            } else {
                names << pressAction();
            }
            break;
        }
    }
    names << QAccessibleWidget::actionNames();
    return names;
}

void QAccessibleButton::doAction(const QString &actionName)
{
    if (!widget()->isEnabled())
        return;
    if (actionName == pressAction() ||
        actionName == showMenuAction()) {
#ifndef QT_NO_MENU
        QPushButton *pb = qobject_cast<QPushButton*>(object());
        if (pb && pb->menu())
            pb->showMenu();
        else
#endif
            button()->animateClick();
    } else if (actionName == checkAction()) {
        button()->setChecked(true);
    } else if (actionName == uncheckAction()) {
        button()->setChecked(false);
    } else {
        QAccessibleWidget::doAction(actionName);
    }
}

QStringList QAccessibleButton::keyBindingsForAction(const QString &actionName) const
{
    if (actionName == pressAction()) {
#ifndef QT_NO_SHORTCUT
        return QStringList() << button()->shortcut().toString();
#endif
    }
    return QStringList();
}


#ifndef QT_NO_TOOLBUTTON
/*!
  \class QAccessibleToolButton
  \brief The QAccessibleToolButton class implements the QAccessibleInterface for tool buttons.
  \internal

  \ingroup accessibility
*/

/*!
  Creates a QAccessibleToolButton object for \a w.
  \a role is propagated to the QAccessibleWidget constructor.
*/
QAccessibleToolButton::QAccessibleToolButton(QWidget *w, QAccessible::Role role)
: QAccessibleButton(w, role)
{
    Q_ASSERT(toolButton());
}

/*! Returns the button. */
QToolButton *QAccessibleToolButton::toolButton() const
{
    return qobject_cast<QToolButton*>(object());
}

/*!
    Returns true if this tool button is a split button.
*/
bool QAccessibleToolButton::isSplitButton() const
{
#ifndef QT_NO_MENU
    return toolButton()->menu() && toolButton()->popupMode() == QToolButton::MenuButtonPopup;
#else
    return false;
#endif
}

QAccessible::State QAccessibleToolButton::state() const
{
    QAccessible::State st = QAccessibleButton::state();
    if (toolButton()->autoRaise())
        st.hotTracked = true;
#ifndef QT_NO_MENU
    if (toolButton()->menu())
        st.hasPopup = true;
#endif
    return st;
}

int QAccessibleToolButton::childCount() const
{
    return isSplitButton() ? 1 : 0;
}

QAccessibleInterface *QAccessibleToolButton::child(int index) const
{
#ifndef QT_NO_MENU
    if (index == 0 && toolButton()->menu())
    {
        return QAccessible::queryAccessibleInterface(toolButton()->menu());
    }
#endif
    return 0;
}

/*!
    \internal

    Returns the button's text label, depending on the text \a t, and
    the \a child.
*/
QString QAccessibleToolButton::text(QAccessible::Text t) const
{
    QString str;
    switch (t) {
    case QAccessible::Name:
        str = toolButton()->accessibleName();
        if (str.isEmpty())
            str = toolButton()->text();
        break;
    default:
        break;
    }
    if (str.isEmpty())
        str = QAccessibleButton::text(t);
    return qt_accStripAmp(str);
}

/*
   The three different tool button types can have the following actions:
| DelayedPopup    | ShowMenuAction + (PressedAction || CheckedAction) |
| MenuButtonPopup | ShowMenuAction + (PressedAction || CheckedAction) |
| InstantPopup    | ShowMenuAction |
*/
QStringList QAccessibleToolButton::actionNames() const
{
    QStringList names;
    if (widget()->isEnabled()) {
        if (toolButton()->menu())
            names << showMenuAction();
        if (toolButton()->popupMode() != QToolButton::InstantPopup)
            names << QAccessibleButton::actionNames();
    }
    return names;
}

void QAccessibleToolButton::doAction(const QString &actionName)
{
    if (!widget()->isEnabled())
        return;

    if (actionName == pressAction()) {
        button()->click();
    } else if (actionName == showMenuAction()) {
        if (toolButton()->popupMode() != QToolButton::InstantPopup) {
            toolButton()->setDown(true);
#ifndef QT_NO_MENU
            toolButton()->showMenu();
#endif
        }
    } else {
        QAccessibleButton::doAction(actionName);
    }

}

#endif // QT_NO_TOOLBUTTON

/*!
  \class QAccessibleDisplay
  \brief The QAccessibleDisplay class implements the QAccessibleInterface for widgets that display information.
  \internal

  \ingroup accessibility
*/

/*!
  Constructs a QAccessibleDisplay object for \a w.
  \a role is propagated to the QAccessibleWidget constructor.
*/
QAccessibleDisplay::QAccessibleDisplay(QWidget *w, QAccessible::Role role)
: QAccessibleWidget(w, role)
{
}

QAccessible::Role QAccessibleDisplay::role() const
{
    QLabel *l = qobject_cast<QLabel*>(object());
    if (l) {
        if (l->pixmap())
            return QAccessible::Graphic;
#ifndef QT_NO_PICTURE
        if (l->picture())
            return QAccessible::Graphic;
#endif
#ifndef QT_NO_MOVIE
        if (l->movie())
            return QAccessible::Animation;
#endif
#ifndef QT_NO_PROGRESSBAR
    } else if (qobject_cast<QProgressBar*>(object())) {
        return QAccessible::ProgressBar;
#endif
    }
    return QAccessibleWidget::role();
}

QString QAccessibleDisplay::text(QAccessible::Text t) const
{
    QString str;
    switch (t) {
    case QAccessible::Name:
        str = widget()->accessibleName();
        if (str.isEmpty()) {
            if (qobject_cast<QLabel*>(object())) {
                str = qobject_cast<QLabel*>(object())->text();
#ifndef QT_NO_GROUPBOX
            } else if (qobject_cast<QGroupBox*>(object())) {
                str = qobject_cast<QGroupBox*>(object())->title();
#endif
#ifndef QT_NO_LCDNUMBER
            } else if (qobject_cast<QLCDNumber*>(object())) {
                QLCDNumber *l = qobject_cast<QLCDNumber*>(object());
                if (l->digitCount())
                    str = QString::number(l->value());
                else
                    str = QString::number(l->intValue());
#endif
            }
        }
        break;
    case QAccessible::Value:
#ifndef QT_NO_PROGRESSBAR
        if (qobject_cast<QProgressBar*>(object()))
            str = QString::number(qobject_cast<QProgressBar*>(object())->value());
#endif
        break;
    default:
        break;
    }
    if (str.isEmpty())
        str = QAccessibleWidget::text(t);
    return qt_accStripAmp(str);
}

QAccessible::Relation QAccessibleDisplay::relationTo(const QAccessibleInterface *other) const
{
    QAccessible::Relation relation = QAccessibleWidget::relationTo(other);

    QObject *o = other->object();
    QLabel *label = qobject_cast<QLabel*>(object());
    if (label) {
#ifndef QT_NO_SHORTCUT
        if (o == label->buddy())
            relation |= QAccessible::Label;
#endif
#ifndef QT_NO_GROUPBOX
    } else {
        QGroupBox *groupbox = qobject_cast<QGroupBox*>(object());
        if (groupbox && !groupbox->title().isEmpty())
            if (groupbox->children().contains(o))
                relation |= QAccessible::Label;
#endif
    }
    return relation;
}

int QAccessibleDisplay::navigate(QAccessible::RelationFlag rel, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (rel == QAccessible::Labelled) {
        QObject *targetObject = 0;
        QLabel *label = qobject_cast<QLabel*>(object());
        if (label) {
#ifndef QT_NO_SHORTCUT
            if (entry == 1)
                targetObject = label->buddy();
#endif
#ifndef QT_NO_GROUPBOX
        } else {
            QGroupBox *groupbox = qobject_cast<QGroupBox*>(object());
            if (groupbox && !groupbox->title().isEmpty())
                *target = child(entry - 1);
#endif
        }
        if (targetObject)
            *target = QAccessible::queryAccessibleInterface(targetObject);
        if (*target)
            return 0;
    }
    return QAccessibleWidget::navigate(rel, entry, target);
}

void *QAccessibleDisplay::interface_cast(QAccessible::InterfaceType t)
{
    if (t == QAccessible::ImageInterface)
        return static_cast<QAccessibleImageInterface*>(this);
    return QAccessibleWidget::interface_cast(t);
}

/*! \internal */
QString QAccessibleDisplay::imageDescription() const
{
#ifndef QT_NO_TOOLTIP
    return widget()->toolTip();
#else
    return QString::null;
#endif
}

/*! \internal */
QSize QAccessibleDisplay::imageSize() const
{
    QLabel *label = qobject_cast<QLabel *>(widget());
    if (!label)
        return QSize();
    const QPixmap *pixmap = label->pixmap();
    if (!pixmap)
        return QSize();
    return pixmap->size();
}

/*! \internal */
QRect QAccessibleDisplay::imagePosition(QAccessible2::CoordinateType coordType) const
{
    QLabel *label = qobject_cast<QLabel *>(widget());
    if (!label)
        return QRect();
    const QPixmap *pixmap = label->pixmap();
    if (!pixmap)
        return QRect();

    switch (coordType) {
    case QAccessible2::RelativeToScreen:
        return QRect(label->mapToGlobal(label->pos()), label->size());
    case QAccessible2::RelativeToParent:
        return label->geometry();
    }

    return QRect();
}

#ifndef QT_NO_LINEEDIT
/*!
  \class QAccessibleLineEdit
  \brief The QAccessibleLineEdit class implements the QAccessibleInterface for widgets with editable text
  \internal

  \ingroup accessibility
*/

/*!
  Constructs a QAccessibleLineEdit object for \a w.
  \a name is propagated to the QAccessibleWidget constructor.
*/
QAccessibleLineEdit::QAccessibleLineEdit(QWidget *w, const QString &name)
: QAccessibleWidget(w, QAccessible::EditableText, name), QAccessibleSimpleEditableTextInterface(this)
{
    addControllingSignal(QLatin1String("textChanged(const QString&)"));
    addControllingSignal(QLatin1String("returnPressed()"));
}

/*! Returns the line edit. */
QLineEdit *QAccessibleLineEdit::lineEdit() const
{
    return qobject_cast<QLineEdit*>(object());
}

QString QAccessibleLineEdit::text(QAccessible::Text t) const
{
    QString str;
    switch (t) {
    case QAccessible::Value:
        if (lineEdit()->echoMode() == QLineEdit::Normal)
            str = lineEdit()->text();
        break;
    default:
        break;
    }
    if (str.isEmpty())
        str = QAccessibleWidget::text(t);;
    return qt_accStripAmp(str);
}

void QAccessibleLineEdit::setText(QAccessible::Text t, const QString &text)
{
    if (t != QAccessible::Value) {
        QAccessibleWidget::setText(t, text);
        return;
    }

    QString newText = text;
    if (lineEdit()->validator()) {
        int pos = 0;
        if (lineEdit()->validator()->validate(newText, pos) != QValidator::Acceptable)
            return;
    }
    lineEdit()->setText(newText);
}

QAccessible::State QAccessibleLineEdit::state() const
{
    QAccessible::State state = QAccessibleWidget::state();

    QLineEdit *l = lineEdit();
    if (l->isReadOnly())
        state.readOnly = true;
    if (l->echoMode() != QLineEdit::Normal)
        state.passwordEdit = true;
    state.selectable = true;
    if (l->hasSelectedText())
        state.selected = true;

    if (l->contextMenuPolicy() != Qt::NoContextMenu
        && l->contextMenuPolicy() != Qt::PreventContextMenu)
        state.hasPopup = true;

    return state;
}

void *QAccessibleLineEdit::interface_cast(QAccessible::InterfaceType t)
{
    if (t == QAccessible::TextInterface)
        return static_cast<QAccessibleTextInterface*>(this);
    else if (t == QAccessible::EditableTextInterface)
        return static_cast<QAccessibleEditableTextInterface*>(this);
    return QAccessibleWidget::interface_cast(t);
}

void QAccessibleLineEdit::addSelection(int startOffset, int endOffset)
{
    setSelection(0, startOffset, endOffset);
}

QString QAccessibleLineEdit::attributes(int offset, int *startOffset, int *endOffset) const
{
    // QLineEdit doesn't have text attributes
    *startOffset = *endOffset = offset;
    return QString();
}

int QAccessibleLineEdit::cursorPosition() const
{
    return lineEdit()->cursorPosition();
}

QRect QAccessibleLineEdit::characterRect(int /*offset*/, CoordinateType /*coordType*/) const
{
    // QLineEdit doesn't hand out character rects
    return QRect();
}

int QAccessibleLineEdit::selectionCount() const
{
    return lineEdit()->hasSelectedText() ? 1 : 0;
}

int QAccessibleLineEdit::offsetAtPoint(const QPoint &point, CoordinateType coordType) const
{
    QPoint p = point;
    if (coordType == RelativeToScreen)
        p = lineEdit()->mapFromGlobal(p);

    return lineEdit()->cursorPositionAt(p);
}

void QAccessibleLineEdit::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
    *startOffset = *endOffset = 0;
    if (selectionIndex != 0)
        return;

    *startOffset = lineEdit()->selectionStart();
    *endOffset = *startOffset + lineEdit()->selectedText().count();
}

QString QAccessibleLineEdit::text(int startOffset, int endOffset) const
{
    if (startOffset > endOffset)
        return QString();

    if (lineEdit()->echoMode() != QLineEdit::Normal)
        return QString();

    return lineEdit()->text().mid(startOffset, endOffset - startOffset);
}

QString QAccessibleLineEdit::textBeforeOffset(int offset, BoundaryType boundaryType,
        int *startOffset, int *endOffset) const
{
    if (lineEdit()->echoMode() != QLineEdit::Normal) {
        *startOffset = *endOffset = -1;
        return QString();
    }
    return qTextBeforeOffsetFromString(offset, boundaryType, startOffset, endOffset, lineEdit()->text());
}

QString QAccessibleLineEdit::textAfterOffset(int offset, BoundaryType boundaryType,
        int *startOffset, int *endOffset) const
{
    if (lineEdit()->echoMode() != QLineEdit::Normal) {
        *startOffset = *endOffset = -1;
        return QString();
    }
    return qTextAfterOffsetFromString(offset, boundaryType, startOffset, endOffset, lineEdit()->text());
}

QString QAccessibleLineEdit::textAtOffset(int offset, BoundaryType boundaryType,
        int *startOffset, int *endOffset) const
{
    if (lineEdit()->echoMode() != QLineEdit::Normal) {
        *startOffset = *endOffset = -1;
        return QString();
    }
    return qTextAtOffsetFromString(offset, boundaryType, startOffset, endOffset, lineEdit()->text());
}

void QAccessibleLineEdit::removeSelection(int selectionIndex)
{
    if (selectionIndex != 0)
        return;

    lineEdit()->deselect();
}

void QAccessibleLineEdit::setCursorPosition(int position)
{
    lineEdit()->setCursorPosition(position);
}

void QAccessibleLineEdit::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    if (selectionIndex != 0)
        return;

    lineEdit()->setSelection(startOffset, endOffset - startOffset);
}

int QAccessibleLineEdit::characterCount() const
{
    return lineEdit()->text().count();
}

void QAccessibleLineEdit::scrollToSubstring(int startIndex, int endIndex)
{
    lineEdit()->setCursorPosition(endIndex);
    lineEdit()->setCursorPosition(startIndex);
}

#endif // QT_NO_LINEEDIT

#ifndef QT_NO_PROGRESSBAR
QAccessibleProgressBar::QAccessibleProgressBar(QWidget *o)
    : QAccessibleDisplay(o)
{
    Q_ASSERT(progressBar());
}

void *QAccessibleProgressBar::interface_cast(QAccessible::InterfaceType t)
{
    if (t == QAccessible::ValueInterface)
        return static_cast<QAccessibleValueInterface*>(this);
    return QAccessibleDisplay::interface_cast(t);
}

QVariant QAccessibleProgressBar::currentValue() const
{
    return progressBar()->value();
}

QVariant QAccessibleProgressBar::maximumValue() const
{
    return progressBar()->maximum();
}

QVariant QAccessibleProgressBar::minimumValue() const
{
    return progressBar()->minimum();
}

QProgressBar *QAccessibleProgressBar::progressBar() const
{
    return qobject_cast<QProgressBar *>(object());
}
#endif

#endif // QT_NO_ACCESSIBILITY

QT_END_NAMESPACE

