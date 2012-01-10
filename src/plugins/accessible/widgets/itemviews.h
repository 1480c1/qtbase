/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#ifndef ACCESSIBLE_ITEMVIEWS_H
#define ACCESSIBLE_ITEMVIEWS_H

#include <QtWidgets/qabstractitemview.h>
#include <QtWidgets/qheaderview.h>
#include <QtGui/qaccessible.h>
#include <QtGui/qaccessible2.h>
#include <QtWidgets/qaccessiblewidget.h>


QT_BEGIN_NAMESPACE

#ifndef QT_NO_ACCESSIBILITY

#ifndef QT_NO_ITEMVIEWS

class QAccessibleTableCell;
class QAccessibleTableHeaderCell;

class QAccessibleTable :public QAccessibleTableInterface, public QAccessibleObject
{
public:
    explicit QAccessibleTable(QWidget *w);

    virtual ~QAccessibleTable();

    QObject *object() const { return view; }
    QAccessible::Role role() const;
    QAccessible::State state() const;
    QString text(QAccessible::Text t) const;
    QRect rect() const;

    QAccessibleInterface *childAt(int x, int y) const;
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *) const;

    QAccessibleInterface *parent() const;
    QAccessibleInterface *child(int index) const;
    int navigate(QAccessible::RelationFlag relation, int index, QAccessibleInterface **iface) const;
    QAccessible::Relation relationTo(const QAccessibleInterface *other) const;

    QVariant invokeMethod(QAccessible::Method, const QVariantList &) { return QVariant(); }
    void *interface_cast(QAccessible::InterfaceType t);

    // table interface
    virtual QAccessibleInterface *cellAt(int row, int column) const;
    virtual QAccessibleInterface *caption() const;
    virtual QAccessibleInterface *summary() const;
    virtual QString columnDescription(int column) const;
    virtual QString rowDescription(int row) const;
    virtual int columnCount() const;
    virtual int rowCount() const;
    virtual QAccessible2::TableModelChange modelChange() const;

    // selection
    virtual int selectedCellCount() const;
    virtual int selectedColumnCount() const;
    virtual int selectedRowCount() const;
    virtual QList<QAccessibleInterface*> selectedCells() const;
    virtual QList<int> selectedColumns() const;
    virtual QList<int> selectedRows() const;
    virtual bool isColumnSelected(int column) const;
    virtual bool isRowSelected(int row) const;
    virtual bool selectRow(int row);
    virtual bool selectColumn(int column);
    virtual bool unselectRow(int row);
    virtual bool unselectColumn(int column);

protected:
    virtual void modelReset();
    virtual void rowsInserted(const QModelIndex &parent, int first, int last);
    virtual void rowsRemoved(const QModelIndex &parent, int first, int last);
    virtual void columnsInserted(const QModelIndex &parent, int first, int last);
    virtual void columnsRemoved(const QModelIndex &parent, int first, int last);
    virtual void rowsMoved( const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);
    virtual void columnsMoved( const QModelIndex &parent, int start, int end, const QModelIndex &destination, int column);

protected:
    QAbstractItemView* view;
    QAccessible2::TableModelChange lastChange;
    inline QAccessibleTableCell *cell(const QModelIndex &index) const;
    inline QAccessible::Role cellRole() const {
        switch (m_role) {
        case QAccessible::List:
            return QAccessible::ListItem;
        case QAccessible::Table:
            return QAccessible::Cell;
        case QAccessible::Tree:
            return QAccessible::TreeItem;
        default:
            Q_ASSERT(0);
        }
        return QAccessible::NoRole;
    }

    QHeaderView *horizontalHeader() const;
    QHeaderView *verticalHeader() const;
private:
    // the child index for a model index
    inline int logicalIndex(const QModelIndex &index) const;
    // the model index from the child index
    QAccessibleInterface *childFromLogical(int logicalIndex) const;
    QAccessible::Role m_role;
};

class QAccessibleTree :public QAccessibleTable
{
public:
    explicit QAccessibleTree(QWidget *w)
        : QAccessibleTable(w)
    {}

    virtual ~QAccessibleTree() {}

    QAccessibleInterface *childAt(int x, int y) const;
    int childCount() const;
    QAccessibleInterface *child(int index) const;

    int indexOfChild(const QAccessibleInterface *) const;

    int rowCount() const;

    QAccessible::Relation relationTo(const QAccessibleInterface *other) const;

    // table interface
    QAccessibleInterface *cellAt(int row, int column) const;
    QString rowDescription(int row) const;
    bool isRowSelected(int row) const;
    bool selectRow(int row);

private:
    QModelIndex indexFromLogical(int row, int column = 0) const;
};

class QAccessibleTableCell: public QAccessibleInterface, public QAccessibleTableCellInterface
{
public:
    QAccessibleTableCell(QAbstractItemView *view, const QModelIndex &m_index, QAccessible::Role role);

    void *interface_cast(QAccessible::InterfaceType t);
    QObject *object() const { return 0; }
    QAccessible::Role role() const;
    QAccessible::State state() const;
    QRect rect() const;
    bool isValid() const;

    QAccessibleInterface *childAt(int, int) const { return 0; }
    int childCount() const { return 0; }
    int indexOfChild(const QAccessibleInterface *) const  { return -1; }

    QString text(QAccessible::Text t) const;
    void setText(QAccessible::Text t, const QString &text);

    QAccessibleInterface *parent() const;
    QAccessibleInterface *child(int) const;
    int navigate(QAccessible::RelationFlag relation, int m_index, QAccessibleInterface **iface) const;

    // cell interface
    virtual int columnExtent() const;
    virtual QList<QAccessibleInterface*> columnHeaderCells() const;
    virtual int columnIndex() const;
    virtual int rowExtent() const;
    virtual QList<QAccessibleInterface*> rowHeaderCells() const;
    virtual int rowIndex() const;
    virtual bool isSelected() const;
    virtual void rowColumnExtents(int *row, int *column, int *rowExtents, int *columnExtents, bool *selected) const;
    virtual QAccessibleInterface* table() const;

private:
    QHeaderView *verticalHeader() const;
    QHeaderView *horizontalHeader() const;
    QAbstractItemView *view;
    QModelIndex m_index;
    QAccessible::Role m_role;

friend class QAccessibleTable;
friend class QAccessibleTree;
};


class QAccessibleTableHeaderCell: public QAccessibleInterface
{
public:
    // For header cells, pass the header view in addition
    QAccessibleTableHeaderCell(QAbstractItemView *view, int index, Qt::Orientation orientation);

    QObject *object() const { return 0; }
    QAccessible::Role role() const;
    QAccessible::State state() const;
    QRect rect() const;
    bool isValid() const;

    QAccessibleInterface *childAt(int, int) const { return 0; }
    int childCount() const { return 0; }
    int indexOfChild(const QAccessibleInterface *) const  { return -1; }

    QString text(QAccessible::Text t) const;
    void setText(QAccessible::Text t, const QString &text);

    QAccessibleInterface *parent() const;
    QAccessibleInterface *child(int index) const;
    int navigate(QAccessible::RelationFlag relation, int index, QAccessibleInterface **iface) const;
    QAccessible::Relation relationTo(const QAccessibleInterface *other) const;

private:
    QAbstractItemView *view;
    int index;
    Qt::Orientation orientation;

friend class QAccessibleTable;
friend class QAccessibleTree;
};

// This is the corner button on the top left of a table.
// It can be used to select all cells or it is not active at all.
// For now it is ignored.
class QAccessibleTableCornerButton: public QAccessibleInterface
{
public:
    QAccessibleTableCornerButton(QAbstractItemView *view_)
        :view(view_)
    {}

    QObject *object() const { return 0; }
    QAccessible::Role role() const { return QAccessible::Pane; }
    QAccessible::State state() const { return QAccessible::State(); }
    QRect rect() const { return QRect(); }
    bool isValid() const { return true; }

    QAccessibleInterface *childAt(int, int) const { return 0; }
    int childCount() const { return 0; }
    int indexOfChild(const QAccessibleInterface *) const  { return -1; }

    QString text(QAccessible::Text) const { return QString(); }
    void setText(QAccessible::Text, const QString &) {}

    QAccessibleInterface *parent() const {
        return QAccessible::queryAccessibleInterface(view);
    }
    QAccessibleInterface *child(int) const {
        return 0;
    }
    int navigate(QAccessible::RelationFlag relation, int, QAccessibleInterface **iface) const
    {
        Q_UNUSED(relation);
        Q_UNUSED(iface);
        return -1;
    }

    QAccessible::Relation relationTo(const QAccessibleInterface *) const
    {
        return QAccessible::Unrelated;
    }

private:
    QAbstractItemView *view;
};


#endif

#endif // QT_NO_ACCESSIBILITY

QT_END_NAMESPACE

#endif // ACCESSIBLE_ITEMVIEWS_H
