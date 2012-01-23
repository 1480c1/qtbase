/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qplatformaccessibility_qpa.h"
#include <private/qfactoryloader_p.h>
#include "qaccessibleplugin.h"
#include "qaccessibleobject.h"
#include "qaccessiblebridge.h"
#include <QtGui/QGuiApplication>

QT_BEGIN_NAMESPACE

/* accessiblebridge plugin discovery stuff */
#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, bridgeloader,
    (QAccessibleBridgeFactoryInterface_iid, QLatin1String("/accessiblebridge")))
#endif

Q_GLOBAL_STATIC(QVector<QAccessibleBridge *>, bridges)

/*!
    \class QPlatformAccessibility
    \brief The QPlatformAccessibility class is the base class for
    integrating accessibility backends

    \preliminary
    \ingroup accessibility

    \sa QAccessible
*/
QPlatformAccessibility::QPlatformAccessibility()
{
}

QPlatformAccessibility::~QPlatformAccessibility()
{
}

void QPlatformAccessibility::notifyAccessibilityUpdate(QObject *o,
                                                       int who,
                                                       QAccessible::Event reason)
{
    initialize();

    if (!bridges() || bridges()->isEmpty())
        return;

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(o);
    if (!iface)
        return;

    // updates for List/Table/Tree should send child
    if (who) {
        QAccessibleInterface *child = iface->child(who - 1);
        if (child) {
            delete iface;
            iface = child;
            who = 0;
        }
    }

    for (int i = 0; i < bridges()->count(); ++i)
        bridges()->at(i)->notifyAccessibilityUpdate(reason, iface, who);
    delete iface;

}

void QPlatformAccessibility::setRootObject(QObject *o)
{
    initialize();
    if (bridges()->isEmpty())
        return;

    if (!o)
        return;

    for (int i = 0; i < bridges()->count(); ++i) {
        QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(o);
        bridges()->at(i)->setRootObject(iface);
    }
}

void QPlatformAccessibility::initialize()
{
    static bool isInit = false;
    if (isInit)
        return;
    isInit = true;      // ### not atomic

#ifndef QT_NO_LIBRARY
    const QStringList l = bridgeloader()->keys();
    for (int i = 0; i < l.count(); ++i) {
        if (QAccessibleBridgeFactoryInterface *factory =
                qobject_cast<QAccessibleBridgeFactoryInterface*>(bridgeloader()->instance(l.at(i)))) {
            QAccessibleBridge * bridge = factory->create(l.at(i));
            if (bridge) {
                bridges()->append(bridge);
            }
        }
    }
#endif
}

void QPlatformAccessibility::cleanup()
{
    qDeleteAll(*bridges());
}

QT_END_NAMESPACE
