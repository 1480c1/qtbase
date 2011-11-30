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

#ifndef QSCREEN_H
#define QSCREEN_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtCore/QSizeF>

#include <QtGui/QTransform>

#include <QtCore/qnamespace.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QPlatformScreen;
class QScreenPrivate;
class QWindow;
class QRect;

class Q_GUI_EXPORT QScreen : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScreen)

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int depth READ depth CONSTANT)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)
    Q_PROPERTY(QSizeF physicalSize READ physicalSize CONSTANT)
    Q_PROPERTY(qreal physicalDotsPerInchX READ physicalDotsPerInchX NOTIFY physicalDotsPerInchXChanged)
    Q_PROPERTY(qreal physicalDotsPerInchY READ physicalDotsPerInchY NOTIFY physicalDotsPerInchYChanged)
    Q_PROPERTY(qreal physicalDotsPerInch READ physicalDotsPerInch NOTIFY physicalDotsPerInchChanged)
    Q_PROPERTY(qreal logicalDotsPerInchX READ logicalDotsPerInchX NOTIFY logicalDotsPerInchXChanged)
    Q_PROPERTY(qreal logicalDotsPerInchY READ logicalDotsPerInchY NOTIFY logicalDotsPerInchYChanged)
    Q_PROPERTY(qreal logicalDotsPerInch READ logicalDotsPerInch NOTIFY logicalDotsPerInchChanged)
    Q_PROPERTY(QSize availableSize READ availableSize NOTIFY availableSizeChanged)
    Q_PROPERTY(QRect availableGeometry READ availableGeometry NOTIFY availableGeometryChanged)
    Q_PROPERTY(Qt::ScreenOrientation primaryOrientation READ primaryOrientation CONSTANT)
    Q_PROPERTY(Qt::ScreenOrientation currentOrientation READ currentOrientation NOTIFY currentOrientationChanged)

public:
    QPlatformScreen *handle() const;

    QString name() const;

    int depth() const;

    QSize size() const;
    QRect geometry() const;

    QSizeF physicalSize() const;

    qreal physicalDotsPerInchX() const;
    qreal physicalDotsPerInchY() const;
    qreal physicalDotsPerInch() const;

    qreal logicalDotsPerInchX() const;
    qreal logicalDotsPerInchY() const;
    qreal logicalDotsPerInch() const;

    QSize availableSize() const;
    QRect availableGeometry() const;

    QList<QScreen *> virtualSiblings() const;

    QSize virtualSize() const;
    QRect virtualGeometry() const;

    QSize availableVirtualSize() const;
    QRect availableVirtualGeometry() const;

    Qt::ScreenOrientation primaryOrientation() const;
    Qt::ScreenOrientation currentOrientation() const;

    static int angleBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b);
    static QTransform transformBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &target);
    static QRect mapBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &rect);

Q_SIGNALS:
    void sizeChanged(const QSize &size);
    void geometryChanged(const QRect &geometry);
    void physicalDotsPerInchXChanged(qreal dpi);
    void physicalDotsPerInchYChanged(qreal dpi);
    void physicalDotsPerInchChanged(qreal dpi);
    void logicalDotsPerInchXChanged(qreal dpi);
    void logicalDotsPerInchYChanged(qreal dpi);
    void logicalDotsPerInchChanged(qreal dpi);
    void availableSizeChanged(const QSize &size);
    void availableGeometryChanged(const QRect &rect);
    void currentOrientationChanged(Qt::ScreenOrientation orientation);

private:
    QScreen(QPlatformScreen *screen);

    Q_DISABLE_COPY(QScreen)
    friend class QGuiApplicationPrivate;
    friend class QPlatformScreen;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSCREEN_H

