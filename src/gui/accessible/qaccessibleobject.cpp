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

#include "qaccessibleobject.h"

#ifndef QT_NO_ACCESSIBILITY

#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>

#include "qpointer.h"
#include "qmetaobject.h"

QT_BEGIN_NAMESPACE

class QAccessibleObjectPrivate
{
public:
    QPointer<QObject> object;

    QList<QByteArray> actionList() const;
};

QList<QByteArray> QAccessibleObjectPrivate::actionList() const
{
    QList<QByteArray> actionList;

    if (!object)
        return actionList;

    const QMetaObject *mo = object->metaObject();
    Q_ASSERT(mo);

    QByteArray defaultAction = QMetaObject::normalizedSignature(
        mo->classInfo(mo->indexOfClassInfo("DefaultSlot")).value());

    for (int i = 0; i < mo->methodCount(); ++i) {
        const QMetaMethod member = mo->method(i);
        if (member.methodType() != QMetaMethod::Slot && member.access() != QMetaMethod::Public)
            continue;

        if (!qstrcmp(member.tag(), "QACCESSIBLE_SLOT")) {
            if (member.signature() == defaultAction)
                actionList.prepend(defaultAction);
            else
                actionList << member.signature();
        }
    }

    return actionList;
}

/*!
    \class QAccessibleObject
    \brief The QAccessibleObject class implements parts of the
    QAccessibleInterface for QObjects.

    \ingroup accessibility
    \inmodule QtWidgets

    This class is part of \l {Accessibility for QWidget Applications}.

    This class is mainly provided for convenience. All subclasses of
    the QAccessibleInterface that provide implementations of non-widget objects
    should use this class as their base class.

    \sa QAccessible, QAccessibleWidget
*/

/*!
    Creates a QAccessibleObject for \a object.
*/
QAccessibleObject::QAccessibleObject(QObject *object)
{
    d = new QAccessibleObjectPrivate;
    d->object = object;
}

/*!
    Destroys the QAccessibleObject.

    This only happens when a call to release() decrements the internal
    reference counter to zero.
*/
QAccessibleObject::~QAccessibleObject()
{
    delete d;
}

/*!
    \reimp
*/
QObject *QAccessibleObject::object() const
{
#ifndef QT_NO_DEBUG
    if (!d->object)
        qWarning("QAccessibleInterface is invalid. Crash pending...");
#endif
    return d->object;
}

/*!
    \reimp
*/
bool QAccessibleObject::isValid() const
{
    return !d->object.isNull();
}

/*! \reimp */
QRect QAccessibleObject::rect() const
{
    return QRect();
}

/*! \reimp */
void QAccessibleObject::setText(QAccessible::Text, const QString &)
{
}

/*! \reimp */
QAccessibleInterface *QAccessibleObject::childAt(int x, int y) const
{
    for (int i = 0; i < childCount(); ++i) {
        QAccessibleInterface *childIface = child(i);
        if (childIface->rect().contains(x,y)) {
            return childIface;
        }
    }
    return 0;
}

/*!
    \class QAccessibleApplication
    \brief The QAccessibleApplication class implements the QAccessibleInterface for QApplication.

    \internal

    \ingroup accessibility
*/

/*!
    Creates a QAccessibleApplication for the QApplication object referenced by qApp.
*/
QAccessibleApplication::QAccessibleApplication()
: QAccessibleObject(qApp)
{
}

QWindow *QAccessibleApplication::window() const
{
    // an application can have several windows, and AFAIK we don't need
    // to notify about changes on the application.
    return 0;
}

// all toplevel windows except popups and the desktop
static QObjectList topLevelObjects()
{
    QObjectList list;
    const QWindowList tlw(QGuiApplication::topLevelWindows());
    for (int i = 0; i < tlw.count(); ++i) {
        QWindow *w = tlw.at(i);
        if (w->windowType() != Qt::Popup && w->windowType() != Qt::Desktop) {
            if (QAccessibleInterface *root = w->accessibleRoot()) {
                if (root->object())
                    list.append(w->accessibleRoot()->object());
                delete root;
            }
        }
    }

    return list;
}

/*! \reimp */
int QAccessibleApplication::childCount() const
{
    return topLevelObjects().count();
}

/*! \reimp */
int QAccessibleApplication::indexOfChild(const QAccessibleInterface *child) const
{
    const QObjectList tlw(topLevelObjects());
    int index = tlw.indexOf(child->object());
    if (index != -1)
        ++index;
    return index;
}

/*! \reimp */
QAccessible::Relation QAccessibleApplication::relationTo(const QAccessibleInterface *other) const
{
    QObject *o = other ? other->object() : 0;
    if (!o)
        return QAccessible::Unrelated;

    if(o == object()) {
        return QAccessible::Self;
    }

    return QAccessible::Unrelated;
}

QAccessibleInterface *QAccessibleApplication::parent() const
{
    return 0;
}

QAccessibleInterface *QAccessibleApplication::child(int index) const
{
    const QObjectList tlo(topLevelObjects());
    if (index >= 0 && index < tlo.count())
        return QAccessible::queryAccessibleInterface(tlo.at(index));
    return 0;
}

/*! \reimp */
int QAccessibleApplication::navigate(QAccessible::RelationFlag relation, int,
                                     QAccessibleInterface **target) const
{
    if (!target)
        return -1;

    *target = 0;
    QObject *targetObject = 0;

    switch (relation) {
    case QAccessible::Self:
        targetObject = object();
        break;
    case QAccessible::FocusChild:
        if (QWindow *window = QGuiApplication::activeWindow()) {
            *target = window->accessibleRoot();
            return 0;
        }
        break;
    default:
        break;
    }
    *target = QAccessible::queryAccessibleInterface(targetObject);
    return *target ? 0 : -1;
}

/*! \reimp */
QString QAccessibleApplication::text(QAccessible::Text t) const
{
    switch (t) {
    case QAccessible::Name:
        return QGuiApplication::applicationName();
    case QAccessible::Description:
        return QGuiApplication::applicationFilePath();
    default:
        break;
    }
    return QString();
}

/*! \reimp */
QAccessible::Role QAccessibleApplication::role() const
{
    return QAccessible::Application;
}

/*! \reimp */
QAccessible::State QAccessibleApplication::state() const
{
    return QAccessible::State();
}


QT_END_NAMESPACE

#endif //QT_NO_ACCESSIBILITY
