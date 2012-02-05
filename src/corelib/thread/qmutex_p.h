/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef QMUTEX_P_H
#define QMUTEX_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qmutex.cpp, qmutex_unix.cpp, and qmutex_win.cpp.  This header
// file may change from version to version without notice, or even be
// removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qmutex.h>
#include <QtCore/qatomic.h>

#if defined(Q_OS_MAC)
# include <mach/semaphore.h>
#endif

QT_BEGIN_NAMESPACE

class QMutexData
{
public:
    bool recursive;
    QMutexData(QMutex::RecursionMode mode = QMutex::NonRecursive)
        : recursive(mode == QMutex::Recursive) {}
};

#if !defined(Q_OS_LINUX)
class QMutexPrivate : public QMutexData {
public:
    ~QMutexPrivate();
    QMutexPrivate();

    bool wait(int timeout = -1);
    void wakeUp();

    // Conrol the lifetime of the privates
    QAtomicInt refCount;
    int id;

    bool ref() {
        Q_ASSERT(refCount.load() >= 0);
        int c;
        do {
            c = refCount.load();
            if (c == 0)
                return false;
        } while (!refCount.testAndSetRelaxed(c, c + 1));
        Q_ASSERT(refCount.load() >= 0);
        return true;
    }
    void deref() {
        Q_ASSERT(refCount.load() >= 0);
        if (!refCount.deref())
            release();
        Q_ASSERT(refCount.load() >= 0);
    }
    void release();
    static QMutexPrivate *allocate();

    QAtomicInt waiters; //number of thread waiting
    QAtomicInt possiblyUnlocked; //bool saying that a timed wait timed out
    enum { BigNumber = 0x100000 }; //Must be bigger than the possible number of waiters (number of threads)
    void derefWaiters(int value);

    //platform specific stuff
#if defined(Q_OS_MAC)
    semaphore_t mach_semaphore;
#elif defined(Q_OS_UNIX)
    bool wakeup;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
#elif defined(Q_OS_WIN)
    Qt::HANDLE event;
#endif
};
#endif //Q_OS_LINUX

class QRecursiveMutexPrivate : public QMutexData
{
public:
    QRecursiveMutexPrivate()
        : QMutexData(QMutex::Recursive), owner(0), count(0) {}
    Qt::HANDLE owner;
    uint count;
    QMutex mutex;

    bool lock(int timeout);
    void unlock();
};

QT_END_NAMESPACE

#endif // QMUTEX_P_H
