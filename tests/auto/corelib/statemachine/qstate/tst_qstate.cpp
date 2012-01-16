/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite module of the Qt Toolkit.
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

#include <QtTest/QtTest>

#include "qstate.h"
#include "qstatemachine.h"
#include "qsignaltransition.h"

class tst_QState : public QObject
{
    Q_OBJECT

public:
    tst_QState();

private slots:
    void assignProperty();
    void assignPropertyTwice();
    void historyInitialState();
    void transitions();

private:
    bool functionCalled;
};

tst_QState::tst_QState() : functionCalled(false)
{
}

class TestClass: public QObject
{
    Q_OBJECT
public:
    TestClass() : called(false) {}
    bool called;

public slots:
    void slot() { called = true; }

    
};

void tst_QState::assignProperty()
{
    QStateMachine machine;

    QObject *object = new QObject();
    object->setProperty("fooBar", 10);

    QState *s1 = new QState(&machine);
    s1->assignProperty(object, "fooBar", 20);
    
    machine.setInitialState(s1);
    machine.start();
    QCoreApplication::processEvents();

    QCOMPARE(object->property("fooBar").toInt(), 20);
}

void tst_QState::assignPropertyTwice()
{
    QStateMachine machine;

    QObject *object = new QObject();
    object->setProperty("fooBar", 10);

    QState *s1 = new QState(&machine);
    s1->assignProperty(object, "fooBar", 20);
    s1->assignProperty(object, "fooBar", 30);
    
    machine.setInitialState(s1);
    machine.start();
    QCoreApplication::processEvents();

    QCOMPARE(object->property("fooBar").toInt(), 30);
}

class EventTestTransition: public QAbstractTransition
{
public:
    EventTestTransition(QEvent::Type type, QState *targetState) 
        : QAbstractTransition(), m_type(type)
    {
        setTargetState(targetState);
    }

protected:
    bool eventTest(QEvent *e)
    {
        return e->type() == m_type;
    }

    void onTransition(QEvent *) {}

private:
    QEvent::Type m_type;
    
};

void tst_QState::historyInitialState() 
{
    QStateMachine machine;

    QState *s1 = new QState(&machine);
    
    QState *s2 = new QState(&machine);
    QHistoryState *h1 = new QHistoryState(s2);
    
    s2->setInitialState(h1);

    QState *s3 = new QState(s2);
    h1->setDefaultState(s3);

    QState *s4 = new QState(s2);

    s1->addTransition(new EventTestTransition(QEvent::User, s2));
    s2->addTransition(new EventTestTransition(QEvent::User, s1));
    s3->addTransition(new EventTestTransition(QEvent::Type(QEvent::User+1), s4));

    machine.setInitialState(s1);
    machine.start();
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 1);
    QVERIFY(machine.configuration().contains(s1));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 2);
    QVERIFY(machine.configuration().contains(s2));
    QVERIFY(machine.configuration().contains(s3));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 1);
    QVERIFY(machine.configuration().contains(s1));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 2);
    QVERIFY(machine.configuration().contains(s2));
    QVERIFY(machine.configuration().contains(s3));

    machine.postEvent(new QEvent(QEvent::Type(QEvent::User+1)));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 2);
    QVERIFY(machine.configuration().contains(s2));
    QVERIFY(machine.configuration().contains(s4));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 1);
    QVERIFY(machine.configuration().contains(s1));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QCOMPARE(machine.configuration().size(), 2);
    QVERIFY(machine.configuration().contains(s2));
    QVERIFY(machine.configuration().contains(s4));
}

void tst_QState::transitions()
{
    QState s1;
    QState s2;

    QVERIFY(s1.transitions().isEmpty());

    QAbstractTransition *t1 = s1.addTransition(this, SIGNAL(destroyed()), &s2);
    QVERIFY(t1 != 0);
    QCOMPARE(s1.transitions().count(), 1);
    QCOMPARE(s1.transitions().first(), t1);
    QVERIFY(s2.transitions().isEmpty());

    s1.removeTransition(t1);
    QVERIFY(s1.transitions().isEmpty());

    s1.addTransition(t1);
    QCOMPARE(s1.transitions().count(), 1);
    QCOMPARE(s1.transitions().first(), t1);

    QAbstractTransition *t2 = new QEventTransition(&s1);
    QCOMPARE(s1.transitions().count(), 2);
    QVERIFY(s1.transitions().contains(t1));
    QVERIFY(s1.transitions().contains(t2));

    // Transitions from child states should not be reported.
    QState *s21 = new QState(&s2);
    QAbstractTransition *t3 = s21->addTransition(this, SIGNAL(destroyed()), &s2);
    QVERIFY(s2.transitions().isEmpty());
    QCOMPARE(s21->transitions().count(), 1);
    QCOMPARE(s21->transitions().first(), t3);
}

QTEST_MAIN(tst_QState)
#include "tst_qstate.moc"
