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

#ifndef QSIZE_H
#define QSIZE_H

#include <QtCore/qnamespace.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


class Q_CORE_EXPORT QSize
{
public:
    Q_DECL_CONSTEXPR QSize();
    Q_DECL_CONSTEXPR QSize(int w, int h);

    Q_DECL_CONSTEXPR bool isNull() const;
    Q_DECL_CONSTEXPR bool isEmpty() const;
    Q_DECL_CONSTEXPR bool isValid() const;

    Q_DECL_CONSTEXPR int width() const;
    Q_DECL_CONSTEXPR int height() const;
    void setWidth(int w);
    void setHeight(int h);
    void transpose();
    Q_DECL_CONSTEXPR QSize transposed() const;

    void scale(int w, int h, Qt::AspectRatioMode mode);
    void scale(const QSize &s, Qt::AspectRatioMode mode);
    QSize scaled(int w, int h, Qt::AspectRatioMode mode) const;
    QSize scaled(const QSize &s, Qt::AspectRatioMode mode) const;

    Q_DECL_CONSTEXPR QSize expandedTo(const QSize &) const;
    Q_DECL_CONSTEXPR QSize boundedTo(const QSize &) const;

    int &rwidth();
    int &rheight();

    QSize &operator+=(const QSize &);
    QSize &operator-=(const QSize &);
    QSize &operator*=(qreal c);
    QSize &operator/=(qreal c);

    friend inline Q_DECL_CONSTEXPR bool operator==(const QSize &, const QSize &);
    friend inline Q_DECL_CONSTEXPR bool operator!=(const QSize &, const QSize &);
    friend inline Q_DECL_CONSTEXPR const QSize operator+(const QSize &, const QSize &);
    friend inline Q_DECL_CONSTEXPR const QSize operator-(const QSize &, const QSize &);
    friend inline Q_DECL_CONSTEXPR const QSize operator*(const QSize &, qreal);
    friend inline Q_DECL_CONSTEXPR const QSize operator*(qreal, const QSize &);
    friend inline const QSize operator/(const QSize &, qreal);

private:
    int wd;
    int ht;
};
Q_DECLARE_TYPEINFO(QSize, Q_MOVABLE_TYPE);

/*****************************************************************************
  QSize stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QSize &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QSize &);
#endif


/*****************************************************************************
  QSize inline functions
 *****************************************************************************/

Q_DECL_CONSTEXPR inline QSize::QSize() : wd(-1), ht(-1) {}

Q_DECL_CONSTEXPR inline QSize::QSize(int w, int h) : wd(w), ht(h) {}

Q_DECL_CONSTEXPR inline bool QSize::isNull() const
{ return wd==0 && ht==0; }

Q_DECL_CONSTEXPR inline bool QSize::isEmpty() const
{ return wd<1 || ht<1; }

Q_DECL_CONSTEXPR inline bool QSize::isValid() const
{ return wd>=0 && ht>=0; }

Q_DECL_CONSTEXPR inline int QSize::width() const
{ return wd; }

Q_DECL_CONSTEXPR inline int QSize::height() const
{ return ht; }

inline void QSize::setWidth(int w)
{ wd = w; }

inline void QSize::setHeight(int h)
{ ht = h; }

Q_DECL_CONSTEXPR inline QSize QSize::transposed() const
{ return QSize(ht, wd); }

inline void QSize::scale(int w, int h, Qt::AspectRatioMode mode)
{ scale(QSize(w, h), mode); }

inline void QSize::scale(const QSize &s, Qt::AspectRatioMode mode)
{ *this = scaled(s, mode); }

inline QSize QSize::scaled(int w, int h, Qt::AspectRatioMode mode) const
{ return scaled(QSize(w, h), mode); }

inline int &QSize::rwidth()
{ return wd; }

inline int &QSize::rheight()
{ return ht; }

inline QSize &QSize::operator+=(const QSize &s)
{ wd+=s.wd; ht+=s.ht; return *this; }

inline QSize &QSize::operator-=(const QSize &s)
{ wd-=s.wd; ht-=s.ht; return *this; }

inline QSize &QSize::operator*=(qreal c)
{ wd = qRound(wd*c); ht = qRound(ht*c); return *this; }

Q_DECL_CONSTEXPR inline bool operator==(const QSize &s1, const QSize &s2)
{ return s1.wd == s2.wd && s1.ht == s2.ht; }

Q_DECL_CONSTEXPR inline bool operator!=(const QSize &s1, const QSize &s2)
{ return s1.wd != s2.wd || s1.ht != s2.ht; }

Q_DECL_CONSTEXPR inline const QSize operator+(const QSize & s1, const QSize & s2)
{ return QSize(s1.wd+s2.wd, s1.ht+s2.ht); }

Q_DECL_CONSTEXPR inline const QSize operator-(const QSize &s1, const QSize &s2)
{ return QSize(s1.wd-s2.wd, s1.ht-s2.ht); }

Q_DECL_CONSTEXPR inline const QSize operator*(const QSize &s, qreal c)
{ return QSize(qRound(s.wd*c), qRound(s.ht*c)); }

Q_DECL_CONSTEXPR inline const QSize operator*(qreal c, const QSize &s)
{ return QSize(qRound(s.wd*c), qRound(s.ht*c)); }

inline QSize &QSize::operator/=(qreal c)
{
    Q_ASSERT(!qFuzzyIsNull(c));
    wd = qRound(wd/c); ht = qRound(ht/c);
    return *this;
}

inline const QSize operator/(const QSize &s, qreal c)
{
    Q_ASSERT(!qFuzzyIsNull(c));
    return QSize(qRound(s.wd/c), qRound(s.ht/c));
}

Q_DECL_CONSTEXPR inline QSize QSize::expandedTo(const QSize & otherSize) const
{
    return QSize(qMax(wd,otherSize.wd), qMax(ht,otherSize.ht));
}

Q_DECL_CONSTEXPR inline QSize QSize::boundedTo(const QSize & otherSize) const
{
    return QSize(qMin(wd,otherSize.wd), qMin(ht,otherSize.ht));
}

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const QSize &);
#endif


class Q_CORE_EXPORT QSizeF
{
public:
    Q_DECL_CONSTEXPR QSizeF();
    Q_DECL_CONSTEXPR QSizeF(const QSize &sz);
    Q_DECL_CONSTEXPR QSizeF(qreal w, qreal h);

    bool isNull() const;
    Q_DECL_CONSTEXPR bool isEmpty() const;
    Q_DECL_CONSTEXPR bool isValid() const;

    Q_DECL_CONSTEXPR qreal width() const;
    Q_DECL_CONSTEXPR qreal height() const;
    void setWidth(qreal w);
    void setHeight(qreal h);
    void transpose();
    Q_DECL_CONSTEXPR QSizeF transposed() const;

    void scale(qreal w, qreal h, Qt::AspectRatioMode mode);
    void scale(const QSizeF &s, Qt::AspectRatioMode mode);
    QSizeF scaled(qreal w, qreal h, Qt::AspectRatioMode mode) const;
    QSizeF scaled(const QSizeF &s, Qt::AspectRatioMode mode) const;

    Q_DECL_CONSTEXPR QSizeF expandedTo(const QSizeF &) const;
    Q_DECL_CONSTEXPR QSizeF boundedTo(const QSizeF &) const;

    qreal &rwidth();
    qreal &rheight();

    QSizeF &operator+=(const QSizeF &);
    QSizeF &operator-=(const QSizeF &);
    QSizeF &operator*=(qreal c);
    QSizeF &operator/=(qreal c);

    friend Q_DECL_CONSTEXPR inline bool operator==(const QSizeF &, const QSizeF &);
    friend Q_DECL_CONSTEXPR inline bool operator!=(const QSizeF &, const QSizeF &);
    friend Q_DECL_CONSTEXPR inline const QSizeF operator+(const QSizeF &, const QSizeF &);
    friend Q_DECL_CONSTEXPR inline const QSizeF operator-(const QSizeF &, const QSizeF &);
    friend Q_DECL_CONSTEXPR inline const QSizeF operator*(const QSizeF &, qreal);
    friend Q_DECL_CONSTEXPR inline const QSizeF operator*(qreal, const QSizeF &);
    friend inline const QSizeF operator/(const QSizeF &, qreal);

    Q_DECL_CONSTEXPR inline QSize toSize() const;

private:
    qreal wd;
    qreal ht;
};
Q_DECLARE_TYPEINFO(QSizeF, Q_MOVABLE_TYPE);


/*****************************************************************************
  QSizeF stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QSizeF &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QSizeF &);
#endif


/*****************************************************************************
  QSizeF inline functions
 *****************************************************************************/

Q_DECL_CONSTEXPR inline QSizeF::QSizeF() : wd(-1.), ht(-1.) {}

Q_DECL_CONSTEXPR inline QSizeF::QSizeF(const QSize &sz) : wd(sz.width()), ht(sz.height()) {}

Q_DECL_CONSTEXPR inline QSizeF::QSizeF(qreal w, qreal h) : wd(w), ht(h) {}

inline bool QSizeF::isNull() const
{ return qIsNull(wd) && qIsNull(ht); }

Q_DECL_CONSTEXPR inline bool QSizeF::isEmpty() const
{ return wd <= 0. || ht <= 0.; }

Q_DECL_CONSTEXPR inline bool QSizeF::isValid() const
{ return wd >= 0. && ht >= 0.; }

Q_DECL_CONSTEXPR inline qreal QSizeF::width() const
{ return wd; }

Q_DECL_CONSTEXPR inline qreal QSizeF::height() const
{ return ht; }

inline void QSizeF::setWidth(qreal w)
{ wd = w; }

inline void QSizeF::setHeight(qreal h)
{ ht = h; }

Q_DECL_CONSTEXPR inline QSizeF QSizeF::transposed() const
{ return QSizeF(ht, wd); }

inline void QSizeF::scale(qreal w, qreal h, Qt::AspectRatioMode mode)
{ scale(QSizeF(w, h), mode); }

inline void QSizeF::scale(const QSizeF &s, Qt::AspectRatioMode mode)
{ *this = scaled(s, mode); }

inline QSizeF QSizeF::scaled(qreal w, qreal h, Qt::AspectRatioMode mode) const
{ return scaled(QSizeF(w, h), mode); }

inline qreal &QSizeF::rwidth()
{ return wd; }

inline qreal &QSizeF::rheight()
{ return ht; }

inline QSizeF &QSizeF::operator+=(const QSizeF &s)
{ wd += s.wd; ht += s.ht; return *this; }

inline QSizeF &QSizeF::operator-=(const QSizeF &s)
{ wd -= s.wd; ht -= s.ht; return *this; }

inline QSizeF &QSizeF::operator*=(qreal c)
{ wd *= c; ht *= c; return *this; }

Q_DECL_CONSTEXPR inline bool operator==(const QSizeF &s1, const QSizeF &s2)
{ return qFuzzyCompare(s1.wd, s2.wd) && qFuzzyCompare(s1.ht, s2.ht); }

Q_DECL_CONSTEXPR inline bool operator!=(const QSizeF &s1, const QSizeF &s2)
{ return !qFuzzyCompare(s1.wd, s2.wd) || !qFuzzyCompare(s1.ht, s2.ht); }

Q_DECL_CONSTEXPR inline const QSizeF operator+(const QSizeF & s1, const QSizeF & s2)
{ return QSizeF(s1.wd+s2.wd, s1.ht+s2.ht); }

Q_DECL_CONSTEXPR inline const QSizeF operator-(const QSizeF &s1, const QSizeF &s2)
{ return QSizeF(s1.wd-s2.wd, s1.ht-s2.ht); }

Q_DECL_CONSTEXPR inline const QSizeF operator*(const QSizeF &s, qreal c)
{ return QSizeF(s.wd*c, s.ht*c); }

Q_DECL_CONSTEXPR inline const QSizeF operator*(qreal c, const QSizeF &s)
{ return QSizeF(s.wd*c, s.ht*c); }

inline QSizeF &QSizeF::operator/=(qreal c)
{
    Q_ASSERT(!qFuzzyIsNull(c));
    wd = wd/c; ht = ht/c;
    return *this;
}

inline const QSizeF operator/(const QSizeF &s, qreal c)
{
    Q_ASSERT(!qFuzzyIsNull(c));
    return QSizeF(s.wd/c, s.ht/c);
}

Q_DECL_CONSTEXPR inline QSizeF QSizeF::expandedTo(const QSizeF & otherSize) const
{
    return QSizeF(qMax(wd,otherSize.wd), qMax(ht,otherSize.ht));
}

Q_DECL_CONSTEXPR inline QSizeF QSizeF::boundedTo(const QSizeF & otherSize) const
{
    return QSizeF(qMin(wd,otherSize.wd), qMin(ht,otherSize.ht));
}

Q_DECL_CONSTEXPR inline QSize QSizeF::toSize() const
{
    return QSize(qRound(wd), qRound(ht));
}

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const QSizeF &);
#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSIZE_H
