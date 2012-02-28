/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qnetworkmanagerengine.h"

#include <QtNetwork/private/qbearerplugin_p.h>

#include <QtCore/qdebug.h>

#ifndef QT_NO_BEARERMANAGEMENT
#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QNetworkManagerEnginePlugin : public QBearerEnginePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QBearerEngineFactoryInterface" FILE "networkmanager.json")

public:
    QNetworkManagerEnginePlugin();
    ~QNetworkManagerEnginePlugin();

    QStringList keys() const;
    QBearerEngine *create(const QString &key) const;
};

QNetworkManagerEnginePlugin::QNetworkManagerEnginePlugin()
{
}

QNetworkManagerEnginePlugin::~QNetworkManagerEnginePlugin()
{
}

QStringList QNetworkManagerEnginePlugin::keys() const
{
    return QStringList() << QLatin1String("networkmanager");
}

QBearerEngine *QNetworkManagerEnginePlugin::create(const QString &key) const
{
    if (key == QLatin1String("networkmanager")) {
        QNetworkManagerEngine *engine = new QNetworkManagerEngine;
        if (engine->networkManagerAvailable())
            return engine;
        else
            delete engine;
    }

    return 0;
}

QT_END_NAMESPACE

#include "main.moc"

#endif // QT_NO_DBUS
#endif // QT_NO_BEARERMANAGEMENT
