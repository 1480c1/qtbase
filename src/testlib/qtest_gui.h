/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtTest module of the Qt Toolkit.
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

#ifndef QTEST_GUI_H
#define QTEST_GUI_H

// enable GUI features
#ifndef QT_GUI_LIB
#define QT_GUI_LIB
#endif
#if 0
#pragma qt_class(QtTestGui)
#endif

#include <QtTest/qtestassert.h>
#include <QtTest/qtest.h>
#include <QtTest/qtestevent.h>
#include <QtTest/qtestmouse.h>
#include <QtTest/qtesttouch.h>
#include <QtTest/qtestkeyboard.h>

#include <QtWidgets/qicon.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qimage.h>

#if 0
// inform syncqt
#pragma qt_no_master_include
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Test)

namespace QTest
{

#ifdef QT_WIDGETS_LIB
template<>
inline bool qCompare(QIcon const &t1, QIcon const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    QTEST_ASSERT(sizeof(QIcon) == sizeof(void *));
    return qCompare<void *>(*reinterpret_cast<void * const *>(&t1),
                   *reinterpret_cast<void * const *>(&t2), actual, expected, file, line);
}
#endif

#ifndef QTEST_NO_SPECIALIZATIONS
template<>
#endif
inline bool qCompare(QImage const &t1, QImage const &t2,
                     const char *actual, const char *expected, const char *file, int line)
{
    char msg[1024];
    msg[0] = '\0';
    const bool t1Null = t1.isNull();
    const bool t2Null = t2.isNull();
    if (t1Null != t2Null) {
        qsnprintf(msg, 1024, "Compared QImages differ.\n"
                  "   Actual (%s).isNull()  : %d\n"
                  "   Expected (%s).isNull(): %d", actual, t1Null, expected, t2Null);
        return compare_helper(false, msg, file, line);
    }
    if (t1Null && t2Null)
        return compare_helper(true, "COMPARE()", file, line);
    if (t1.width() != t2.width() || t2.height() != t2.height()) {
        qsnprintf(msg, 1024, "Compared QImages differ in size.\n"
                  "   Actual (%s)  : %dx%d\n"
                  "   Expected (%s): %dx%d",
                  actual, t1.width(), t1.height(),
                  expected, t2.width(), t2.height());
        return compare_helper(false, msg, file, line);
    }
    if (t1.format() != t2.format()) {
        qsnprintf(msg, 1024, "Compared QImages differ in format.\n"
                  "   Actual (%s)  : %d\n"
                  "   Expected (%s): %d",
                  actual, t1.format(), expected, t2.format());
        return compare_helper(false, msg, file, line);
    }
    return (t1 == t2)
            ? compare_helper(true, "COMPARE()", file, line)
            : compare_helper(false, "Compared values are not the same",
                             toString(t1), toString(t2), actual, expected, file, line);
}

#ifndef QTEST_NO_SPECIALIZATIONS
template<>
#endif
inline bool qCompare(QPixmap const &t1, QPixmap const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    char msg[1024];
    msg[0] = '\0';
    const bool t1Null = t1.isNull();
    const bool t2Null = t2.isNull();
    if (t1Null != t2Null) {
        qsnprintf(msg, 1024, "Compared QPixmaps differ.\n"
                  "   Actual (%s).isNull()  : %d\n"
                  "   Expected (%s).isNull(): %d", actual, t1Null, expected, t2Null);
        return compare_helper(false, msg, file, line);
    }
    if (t1Null && t2Null)
        return compare_helper(true, "COMPARE()", file, line);
    if (t1.width() != t2.width() || t2.height() != t2.height()) {
        qsnprintf(msg, 1024, "Compared QPixmaps differ in size.\n"
                  "   Actual (%s)  : %dx%d\n"
                  "   Expected (%s): %dx%d",
                  actual, t1.width(), t1.height(),
                  expected, t2.width(), t2.height());
        return compare_helper(false, msg, file, line);
    }
    return qCompare(t1.toImage(), t2.toImage(), actual, expected, file, line);
}

}

#ifdef Q_WS_X11
extern void qt_x11_wait_for_window_manager(QWidget *w);
#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif
