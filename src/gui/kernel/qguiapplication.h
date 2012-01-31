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

#ifndef QGUIAPPLICATION_QPA_H
#define QGUIAPPLICATION_QPA_H

#include <QtCore/qcoreapplication.h>
#include <QtGui/qwindowdefs.h>
#include <QtCore/qlocale.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsize.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


class QGuiApplicationPrivate;
class QPlatformNativeInterface;
class QPalette;
class QScreen;
class QStyleHints;
class QInputPanel;

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<QGuiApplication *>(QCoreApplication::instance()))

#if defined(qGuiApp)
#undef qGuiApp
#endif
#define qGuiApp (static_cast<QGuiApplication *>(QCoreApplication::instance()))

class Q_GUI_EXPORT QGuiApplication : public QCoreApplication
{
    Q_OBJECT
    Q_PROPERTY(Qt::LayoutDirection layoutDirection READ layoutDirection WRITE setLayoutDirection)

    Q_PROPERTY(bool quitOnLastWindowClosed  READ quitOnLastWindowClosed WRITE setQuitOnLastWindowClosed)

public:
    QGuiApplication(int &argc, char **argv, int = ApplicationFlags);
    virtual ~QGuiApplication();

    static QWindowList allWindows();
    static QWindowList topLevelWindows();
    static QWindow *topLevelAt(const QPoint &pos);

#ifdef QT_DEPRECATED
    static QT_DEPRECATED QWindow *activeWindow() { return focusWindow(); }
#endif
    static QWindow *focusWindow();
    static QObject *focusObject();

    static QScreen *primaryScreen();
    static QList<QScreen *> screens();

#ifndef QT_NO_CURSOR
    static QCursor *overrideCursor();
    static void setOverrideCursor(const QCursor &);
    static void changeOverrideCursor(const QCursor &);
    static void restoreOverrideCursor();
#endif

    static QFont font();
    static void setFont(const QFont &);

#ifndef QT_NO_CLIPBOARD
    static QClipboard *clipboard();
#endif

    static QPalette palette();
    static void setPalette(const QPalette &pal);

    static Qt::KeyboardModifiers keyboardModifiers();
    static Qt::MouseButtons mouseButtons();

    static void setLayoutDirection(Qt::LayoutDirection direction);
    static Qt::LayoutDirection layoutDirection();

    static inline bool isRightToLeft() { return layoutDirection() == Qt::RightToLeft; }
    static inline bool isLeftToRight() { return layoutDirection() == Qt::LeftToRight; }

    QStyleHints *styleHints() const;
    QInputPanel *inputPanel() const;

    static QPlatformNativeInterface *platformNativeInterface();

    static void setQuitOnLastWindowClosed(bool quit);
    static bool quitOnLastWindowClosed();

    static int exec();
    bool notify(QObject *, QEvent *);

Q_SIGNALS:
    void fontDatabaseChanged();
    void screenAdded(QScreen *screen);
    void lastWindowClosed();
    void focusObjectChanged(QObject *focusObject);

protected:
    bool event(QEvent *);
    bool compressEvent(QEvent *, QObject *receiver, QPostEventList *);

    QGuiApplication(QGuiApplicationPrivate &p);

private:
    Q_DISABLE_COPY(QGuiApplication)
    Q_DECLARE_PRIVATE(QGuiApplication)

#ifndef QT_NO_GESTURES
    friend class QGestureManager;
#endif
    friend class QFontDatabasePrivate;
    friend class QPlatformIntegration;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QGUIAPPLICATION_QPA_H
