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

#ifndef QMENUBAR_H
#define QMENUBAR_H

#include <QtWidgets/qmenu.h>
#ifdef Q_OS_MAC
#include "QtWidgets/qmacdefines_mac.h"
#endif


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_MENUBAR

class QMenuBarPrivate;
class QStyleOptionMenuItem;
class QWindowsStyle;
#ifdef QT3_SUPPORT
class QMenuItem;
#endif

class Q_WIDGETS_EXPORT QMenuBar : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool defaultUp READ isDefaultUp WRITE setDefaultUp)
    Q_PROPERTY(bool nativeMenuBar READ isNativeMenuBar WRITE setNativeMenuBar)

public:
    explicit QMenuBar(QWidget *parent = 0);
    ~QMenuBar();

#ifdef Q_NO_USING_KEYWORD
    void addAction(QAction *action) { QWidget::addAction(action); }
#else
    using QWidget::addAction;
#endif
    QAction *addAction(const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char* member);

    QAction *addMenu(QMenu *menu);
    QMenu *addMenu(const QString &title);
    QMenu *addMenu(const QIcon &icon, const QString &title);


    QAction *addSeparator();
    QAction *insertSeparator(QAction *before);

    QAction *insertMenu(QAction *before, QMenu *menu);

    void clear();

    QAction *activeAction() const;
    void setActiveAction(QAction *action);

    void setDefaultUp(bool);
    bool isDefaultUp() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    int heightForWidth(int) const;

    QRect actionGeometry(QAction *) const;
    QAction *actionAt(const QPoint &) const;

    void setCornerWidget(QWidget *w, Qt::Corner corner = Qt::TopRightCorner);
    QWidget *cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;

#ifdef Q_WS_WINCE
    void setDefaultAction(QAction *);
    QAction *defaultAction() const;

    static void wceCommands(uint command);
    static void wceRefresh();
#endif

    bool isNativeMenuBar() const;
    void setNativeMenuBar(bool nativeMenuBar);
    QPlatformMenuBar *platformMenuBar();
public Q_SLOTS:
    virtual void setVisible(bool visible);

Q_SIGNALS:
    void triggered(QAction *action);
    void hovered(QAction *action);

protected:
    void changeEvent(QEvent *);
    void keyPressEvent(QKeyEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void actionEvent(QActionEvent *);
    void focusOutEvent(QFocusEvent *);
    void focusInEvent(QFocusEvent *);
    void timerEvent(QTimerEvent *);
    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);
    void initStyleOption(QStyleOptionMenuItem *option, const QAction *action) const;

private:
    Q_DECLARE_PRIVATE(QMenuBar)
    Q_DISABLE_COPY(QMenuBar)
    Q_PRIVATE_SLOT(d_func(), void _q_actionTriggered())
    Q_PRIVATE_SLOT(d_func(), void _q_actionHovered())
    Q_PRIVATE_SLOT(d_func(), void _q_internalShortcutActivated(int))
    Q_PRIVATE_SLOT(d_func(), void _q_updateLayout())

#ifdef Q_WS_WINCE
    Q_PRIVATE_SLOT(d_func(), void _q_updateDefaultAction())
#endif

    friend class QMenu;
    friend class QMenuPrivate;
    friend class QWindowsStyle;

#ifdef Q_OS_MAC
    friend class QApplicationPrivate;
    friend class QWidgetPrivate;
    friend bool qt_mac_activate_action(MenuRef, uint, QAction::ActionEvent, bool);
#endif
};

#endif // QT_NO_MENUBAR

QT_END_NAMESPACE

QT_END_HEADER

#endif // QMENUBAR_H
