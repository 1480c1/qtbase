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

#include <qgenericplugin_qpa.h>
#include "qevdevkeyboardmanager.h"

QT_BEGIN_NAMESPACE

class QEvdevKeyboardPlugin : public QGenericPlugin
{
public:
    QEvdevKeyboardPlugin();

    QStringList keys() const;
    QObject* create(const QString &key, const QString &specification);
};

QEvdevKeyboardPlugin::QEvdevKeyboardPlugin()
    : QGenericPlugin()
{
}

QStringList QEvdevKeyboardPlugin::keys() const
{
    return (QStringList()
            << QLatin1String("EvdevKeyboard"));
}

QObject* QEvdevKeyboardPlugin::create(const QString &key,
                                                 const QString &specification)
{
    if (!key.compare(QLatin1String("EvdevKeyboard"), Qt::CaseInsensitive))
        return new QEvdevKeyboardManager(key, specification);
    return 0;
}

Q_EXPORT_PLUGIN2(qevdevkeyboardplugin, QEvdevKeyboardPlugin)

QT_END_NAMESPACE
