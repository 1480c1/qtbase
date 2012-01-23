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

#include "qmeegointegration.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusArgument>
#include <qguiapplication.h>
#include <qwindowsysteminterface_qpa.h>

QMeeGoIntegration::QMeeGoIntegration()
    : screenTopEdge(QStringLiteral("com.nokia.SensorService"), QStringLiteral("Screen.TopEdge"))
{
    connect(&screenTopEdge, SIGNAL(valueChanged(QVariant)),
            this, SLOT(updateScreenOrientation(QVariant)));
    updateScreenOrientation(screenTopEdge.value());
}

QMeeGoIntegration::~QMeeGoIntegration()
{
}

void QMeeGoIntegration::updateScreenOrientation(const QVariant& topEdgeValue)
{
    QString edge = topEdgeValue.toString();
    Qt::ScreenOrientation orientation = Qt::UnknownOrientation;

    // ### FIXME: This isn't perfect. We should obey the video_route (tv connected) and
    // the keyboard slider.

    if (edge == QLatin1String("top"))
        orientation = Qt::LandscapeOrientation;
    else if (edge == QLatin1String("left"))
        orientation = Qt::PortraitOrientation;
    else if (edge == QLatin1String("right"))
        orientation = Qt::InvertedPortraitOrientation;
    else if (edge == QLatin1String("bottom"))
        orientation = Qt::InvertedLandscapeOrientation;

    QWindowSystemInterface::handleScreenOrientationChange(QGuiApplication::primaryScreen(), orientation);
}

