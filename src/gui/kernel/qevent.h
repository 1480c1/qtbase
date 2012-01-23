/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QEVENT_H
#define QEVENT_H

#include <QtGui/qwindowdefs.h>
#include <QtCore/qobject.h>
#include <QtGui/qregion.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>
#include <QtGui/qkeysequence.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qvariant.h>
#include <QtCore/qmap.h>
#include <QtCore/qvector.h>
#include <QtCore/qset.h>
#include <QtCore/qfile.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qtouchdevice.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QAction;
#ifndef QT_NO_GESTURES
class QGesture;
#endif
class QScreen;

class Q_GUI_EXPORT QInputEvent : public QEvent
{
public:
    QInputEvent(Type type, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    ~QInputEvent();
    inline Qt::KeyboardModifiers modifiers() const { return modState; }
    inline void setModifiers(Qt::KeyboardModifiers amodifiers) { modState = amodifiers; }
    inline ulong timestamp() const { return ts; }
    inline void setTimestamp(ulong atimestamp) { ts = atimestamp; }
protected:
    Qt::KeyboardModifiers modState;
    ulong ts;
};

class Q_GUI_EXPORT QMouseEvent : public QInputEvent
{
public:
    QMouseEvent(Type type, const QPointF &pos, Qt::MouseButton button,
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
    QMouseEvent(Type type, const QPointF &pos, const QPointF &globalPos,
                Qt::MouseButton button, Qt::MouseButtons buttons,
                Qt::KeyboardModifiers modifiers);
    QMouseEvent(Type type, const QPointF &pos, const QPointF &windowPos, const QPointF &globalPos,
                Qt::MouseButton button, Qt::MouseButtons buttons,
                Qt::KeyboardModifiers modifiers);
    ~QMouseEvent();

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
    inline QPoint pos() const { return l.toPoint(); }
    inline QPoint globalPos() const { return s.toPoint(); }
    inline int x() const { return qRound(l.x()); }
    inline int y() const { return qRound(l.y()); }
    inline int globalX() const { return qRound(s.x()); }
    inline int globalY() const { return qRound(s.y()); }
#endif
    const QPointF &localPos() const { return l; }
    const QPointF &windowPos() const { return w; }
    const QPointF &screenPos() const { return s; }

    inline Qt::MouseButton button() const { return b; }
    inline Qt::MouseButtons buttons() const { return mouseState; }

#if QT_DEPRECATED_SINCE(5, 0)
    QT_DEPRECATED inline QPointF posF() const { return l; }
#endif

protected:
    QPointF l, w, s;
    Qt::MouseButton b;
    Qt::MouseButtons mouseState;
};

class Q_GUI_EXPORT QHoverEvent : public QInputEvent
{
public:
    QHoverEvent(Type type, const QPointF &pos, const QPointF &oldPos, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    ~QHoverEvent();

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
    inline QPoint pos() const { return p.toPoint(); }
    inline QPoint oldPos() const { return op.toPoint(); }
#endif

    inline const QPointF &posF() const { return p; }
    inline const QPointF &oldPosF() const { return op; }

protected:
    QPointF p, op;
};

#ifndef QT_NO_WHEELEVENT
class Q_GUI_EXPORT QWheelEvent : public QInputEvent
{
public:
    QWheelEvent(const QPointF &pos, int delta,
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                Qt::Orientation orient = Qt::Vertical);
    QWheelEvent(const QPointF &pos, const QPointF& globalPos, int delta,
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                Qt::Orientation orient = Qt::Vertical);
    ~QWheelEvent();

    inline int delta() const { return d; }
#ifndef QT_NO_INTEGER_EVENT_COORDINATES
    inline QPoint pos() const { return p.toPoint(); }
    inline QPoint globalPos()   const { return g.toPoint(); }
    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return g.x(); }
    inline int globalY() const { return g.y(); }
#endif
    inline const QPointF &posF() const { return p; }
    inline const QPointF &globalPosF()   const { return g; }

    inline Qt::MouseButtons buttons() const { return mouseState; }
    Qt::Orientation orientation() const { return o; }


protected:
    QPointF p;
    QPointF g;
    int d;
    Qt::MouseButtons mouseState;
    Qt::Orientation o;
};
#endif

#ifndef QT_NO_TABLETEVENT
class Q_GUI_EXPORT QTabletEvent : public QInputEvent
{
public:
    enum TabletDevice { NoDevice, Puck, Stylus, Airbrush, FourDMouse,
                        XFreeEraser /*internal*/, RotationStylus };
    enum PointerType { UnknownPointer, Pen, Cursor, Eraser };
    QTabletEvent(Type t, const QPoint &pos, const QPoint &globalPos, const QPointF &hiResGlobalPos,
                 int device, int pointerType, qreal pressure, int xTilt, int yTilt,
                 qreal tangentialPressure, qreal rotation, int z,
                 Qt::KeyboardModifiers keyState, qint64 uniqueID);
    ~QTabletEvent();

    inline const QPoint &pos() const { return mPos; }
    inline const QPoint &globalPos() const { return mGPos; }
    inline const QPointF &hiResGlobalPos() const { return mHiResGlobalPos; }
    inline int x() const { return mPos.x(); }
    inline int y() const { return mPos.y(); }
    inline int globalX() const { return mGPos.x(); }
    inline int globalY() const { return mGPos.y(); }
    inline qreal hiResGlobalX() const { return mHiResGlobalPos.x(); }
    inline qreal hiResGlobalY() const { return mHiResGlobalPos.y(); }
    inline TabletDevice device() const { return TabletDevice(mDev); }
    inline PointerType pointerType() const { return PointerType(mPointerType); }
    inline qint64 uniqueId() const { return mUnique; }
    inline qreal pressure() const { return mPress; }
    inline int z() const { return mZ; }
    inline qreal tangentialPressure() const { return mTangential; }
    inline qreal rotation() const { return mRot; }
    inline int xTilt() const { return mXT; }
    inline int yTilt() const { return mYT; }

protected:
    QPoint mPos, mGPos;
    QPointF mHiResGlobalPos;
    int mDev, mPointerType, mXT, mYT, mZ;
    qreal mPress, mTangential, mRot;
    qint64 mUnique;

    // I don't know what the future holds for tablets but there could be some
    // new devices coming along, and there seem to be "holes" in the
    // OS-specific events for this.
    void *mExtra;
};
#endif // QT_NO_TABLETEVENT

class Q_GUI_EXPORT QKeyEvent : public QInputEvent
{
public:
    QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers, const QString& text = QString(),
              bool autorep = false, ushort count = 1);
    ~QKeyEvent();

    int key() const { return k; }
#ifndef QT_NO_SHORTCUT
    bool matches(QKeySequence::StandardKey key) const;
#endif
    Qt::KeyboardModifiers modifiers() const;
    inline QString text() const { return txt; }
    inline bool isAutoRepeat() const { return autor; }
    inline int count() const { return int(c); }

    // Functions for the extended key event information
    static QKeyEvent *createExtendedKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers,
                                             quint32 nativeScanCode, quint32 nativeVirtualKey,
                                             quint32 nativeModifiers,
                                             const QString& text = QString(), bool autorep = false,
                                             ushort count = 1);
    inline bool hasExtendedInfo() const { return reinterpret_cast<const QKeyEvent*>(d) == this; }
    quint32 nativeScanCode() const;
    quint32 nativeVirtualKey() const;
    quint32 nativeModifiers() const;

protected:
    QString txt;
    int k;
    ushort c;
    uint autor:1;
};


class Q_GUI_EXPORT QFocusEvent : public QEvent
{
public:
    QFocusEvent(Type type, Qt::FocusReason reason=Qt::OtherFocusReason);
    ~QFocusEvent();

    inline bool gotFocus() const { return type() == FocusIn; }
    inline bool lostFocus() const { return type() == FocusOut; }

    Qt::FocusReason reason() const;

private:
    Qt::FocusReason m_reason;
};


class Q_GUI_EXPORT QPaintEvent : public QEvent
{
public:
    QPaintEvent(const QRegion& paintRegion);
    QPaintEvent(const QRect &paintRect);
    ~QPaintEvent();

    inline const QRect &rect() const { return m_rect; }
    inline const QRegion &region() const { return m_region; }

protected:
    friend class QApplication;
    friend class QCoreApplication;
    QRect m_rect;
    QRegion m_region;
    bool m_erased;
};

// ### Qt5: make internal
class Q_GUI_EXPORT QUpdateLaterEvent : public QEvent
{
public:
    QUpdateLaterEvent(const QRegion& paintRegion);
    ~QUpdateLaterEvent();

    inline const QRegion &region() const { return m_region; }

protected:
    QRegion m_region;
};

class Q_GUI_EXPORT QMoveEvent : public QEvent
{
public:
    QMoveEvent(const QPoint &pos, const QPoint &oldPos);
    ~QMoveEvent();

    inline const QPoint &pos() const { return p; }
    inline const QPoint &oldPos() const { return oldp;}
protected:
    QPoint p, oldp;
    friend class QApplication;
    friend class QCoreApplication;
};

class Q_GUI_EXPORT QExposeEvent : public QEvent
{
public:
    QExposeEvent(const QRegion &rgn);
    ~QExposeEvent();

    inline const QRegion &region() const { return rgn; }

protected:
    QRegion rgn;
};

class Q_GUI_EXPORT QResizeEvent : public QEvent
{
public:
    QResizeEvent(const QSize &size, const QSize &oldSize);
    ~QResizeEvent();

    inline const QSize &size() const { return s; }
    inline const QSize &oldSize()const { return olds;}
protected:
    QSize s, olds;
    friend class QApplication;
    friend class QCoreApplication;
};


class Q_GUI_EXPORT QCloseEvent : public QEvent
{
public:
    QCloseEvent();
    ~QCloseEvent();
};


class Q_GUI_EXPORT QIconDragEvent : public QEvent
{
public:
    QIconDragEvent();
    ~QIconDragEvent();
};


class Q_GUI_EXPORT QShowEvent : public QEvent
{
public:
    QShowEvent();
    ~QShowEvent();
};


class Q_GUI_EXPORT QHideEvent : public QEvent
{
public:
    QHideEvent();
    ~QHideEvent();
};

#ifndef QT_NO_CONTEXTMENU
class Q_GUI_EXPORT QContextMenuEvent : public QInputEvent
{
public:
    enum Reason { Mouse, Keyboard, Other };

    QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos,
                      Qt::KeyboardModifiers modifiers);
    QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos);
    QContextMenuEvent(Reason reason, const QPoint &pos);
    ~QContextMenuEvent();

    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return gp.x(); }
    inline int globalY() const { return gp.y(); }

    inline const QPoint& pos() const { return p; }
    inline const QPoint& globalPos() const { return gp; }

    inline Reason reason() const { return Reason(reas); }

protected:
    QPoint p;
    QPoint gp;
    uint reas : 8;
};
#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_INPUTMETHOD
class Q_GUI_EXPORT QInputMethodEvent : public QEvent
{
public:
    enum AttributeType {
       TextFormat,
       Cursor,
       Language,
       Ruby,
       Selection
    };
    class Attribute {
    public:
        Attribute(AttributeType t, int s, int l, QVariant val) : type(t), start(s), length(l), value(val) {}
        AttributeType type;

        int start;
        int length;
        QVariant value;
    };
    QInputMethodEvent();
    QInputMethodEvent(const QString &preeditText, const QList<Attribute> &attributes);
    void setCommitString(const QString &commitString, int replaceFrom = 0, int replaceLength = 0);
    void setTentativeCommitString(const QString &tentativeCommitString);

    inline const QList<Attribute> &attributes() const { return attrs; }
    inline const QString &preeditString() const { return preedit; }

    inline const QString &commitString() const { return commit; }
    inline int replacementStart() const { return replace_from; }
    inline int replacementLength() const { return replace_length; }
    inline const QString &tentativeCommitString() const { return tentativeCommit; }

    QInputMethodEvent(const QInputMethodEvent &other);

private:
    QString preedit;
    QList<Attribute> attrs;
    QString commit;
    int replace_from;
    int replace_length;
    QString tentativeCommit;
};

class Q_GUI_EXPORT QInputMethodQueryEvent : public QEvent
{
public:
    QInputMethodQueryEvent(Qt::InputMethodQueries queries);
    ~QInputMethodQueryEvent();

    Qt::InputMethodQueries queries() const { return m_queries; }

    void setValue(Qt::InputMethodQuery query, const QVariant &value);
    QVariant value(Qt::InputMethodQuery query) const;
private:
    Qt::InputMethodQueries m_queries;
    struct QueryPair {
        Qt::InputMethodQuery query;
        QVariant value;
    };
    QVector<QueryPair> m_values;
};

#endif // QT_NO_INPUTMETHOD

#ifndef QT_NO_DRAGANDDROP

class QMimeData;

class Q_GUI_EXPORT QDropEvent : public QEvent
{
public:
    QDropEvent(const QPoint& pos, Qt::DropActions actions, const QMimeData *data,
               Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Type type = Drop);
    ~QDropEvent();

    inline const QPoint &pos() const { return p; }
    inline Qt::MouseButtons mouseButtons() const { return mouseState; }
    inline Qt::KeyboardModifiers keyboardModifiers() const { return modState; }

    inline Qt::DropActions possibleActions() const { return act; }
    inline Qt::DropAction proposedAction() const { return default_action; }
    inline void acceptProposedAction() { drop_action = default_action; accept(); }

    inline Qt::DropAction dropAction() const { return drop_action; }
    void setDropAction(Qt::DropAction action);

    QObject* source() const;
    inline const QMimeData *mimeData() const { return mdata; }

protected:
    friend class QApplication;
    QPoint p;
    Qt::MouseButtons mouseState;
    Qt::KeyboardModifiers modState;
    Qt::DropActions act;
    Qt::DropAction drop_action;
    Qt::DropAction default_action;
    const QMimeData *mdata;
};


class Q_GUI_EXPORT QDragMoveEvent : public QDropEvent
{
public:
    QDragMoveEvent(const QPoint &pos, Qt::DropActions actions, const QMimeData *data,
                   Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Type type = DragMove);
    ~QDragMoveEvent();

    inline QRect answerRect() const { return rect; }

    inline void accept() { QDropEvent::accept(); }
    inline void ignore() { QDropEvent::ignore(); }

    inline void accept(const QRect & r) { accept(); rect = r; }
    inline void ignore(const QRect & r) { ignore(); rect = r; }

protected:
    friend class QApplication;
    QRect rect;
};


class Q_GUI_EXPORT QDragEnterEvent : public QDragMoveEvent
{
public:
    QDragEnterEvent(const QPoint &pos, Qt::DropActions actions, const QMimeData *data,
                    Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
    ~QDragEnterEvent();
};


class Q_GUI_EXPORT QDragLeaveEvent : public QEvent
{
public:
    QDragLeaveEvent();
    ~QDragLeaveEvent();
};
#endif // QT_NO_DRAGANDDROP


class Q_GUI_EXPORT QHelpEvent : public QEvent
{
public:
    QHelpEvent(Type type, const QPoint &pos, const QPoint &globalPos);
    ~QHelpEvent();

    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return gp.x(); }
    inline int globalY() const { return gp.y(); }

    inline const QPoint& pos()  const { return p; }
    inline const QPoint& globalPos() const { return gp; }

private:
    QPoint p;
    QPoint gp;
};

#ifndef QT_NO_STATUSTIP
class Q_GUI_EXPORT QStatusTipEvent : public QEvent
{
public:
    QStatusTipEvent(const QString &tip);
    ~QStatusTipEvent();

    inline QString tip() const { return s; }
private:
    QString s;
};
#endif

#ifndef QT_NO_WHATSTHIS
class Q_GUI_EXPORT QWhatsThisClickedEvent : public QEvent
{
public:
    QWhatsThisClickedEvent(const QString &href);
    ~QWhatsThisClickedEvent();

    inline QString href() const { return s; }
private:
    QString s;
};
#endif

#ifndef QT_NO_ACTION
class Q_GUI_EXPORT QActionEvent : public QEvent
{
    QAction *act, *bef;
public:
    QActionEvent(int type, QAction *action, QAction *before = 0);
    ~QActionEvent();

    inline QAction *action() const { return act; }
    inline QAction *before() const { return bef; }
};
#endif

class Q_GUI_EXPORT QFileOpenEvent : public QEvent
{
public:
    QFileOpenEvent(const QString &file);
    QFileOpenEvent(const QUrl &url);
    ~QFileOpenEvent();

    inline QString file() const { return f; }
    QUrl url() const;
    bool openFile(QFile &file, QIODevice::OpenMode flags) const;
private:
    QString f;
};

#ifndef QT_NO_TOOLBAR
class Q_GUI_EXPORT QToolBarChangeEvent : public QEvent
{
public:
    QToolBarChangeEvent(bool t);
    ~QToolBarChangeEvent();

    inline bool toggle() const { return tog; }
private:
    uint tog : 1;
};
#endif

#ifndef QT_NO_SHORTCUT
class Q_GUI_EXPORT QShortcutEvent : public QEvent
{
public:
    QShortcutEvent(const QKeySequence &key, int id, bool ambiguous = false);
    ~QShortcutEvent();

    inline const QKeySequence &key() const { return sequence; }
    inline int shortcutId() const { return sid; }
    inline bool isAmbiguous() const { return ambig; }
protected:
    QKeySequence sequence;
    bool ambig;
    int  sid;
};
#endif

#ifndef QT_NO_CLIPBOARD
class Q_GUI_EXPORT QClipboardEvent : public QEvent
{
public:
    QClipboardEvent(QEventPrivate *data);
    ~QClipboardEvent();

    QEventPrivate *data() { return d; }
};
#endif

class Q_GUI_EXPORT QWindowStateChangeEvent: public QEvent
{
public:
    QWindowStateChangeEvent(Qt::WindowStates aOldState);
    QWindowStateChangeEvent(Qt::WindowStates aOldState, bool isOverride);
    ~QWindowStateChangeEvent();

    inline Qt::WindowStates oldState() const { return ostate; }
    bool isOverride() const;

private:
    Qt::WindowStates ostate;
};

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QEvent *);
#endif

#ifndef QT_NO_SHORTCUT
inline bool operator==(QKeyEvent *e, QKeySequence::StandardKey key){return (e ? e->matches(key) : false);}
inline bool operator==(QKeySequence::StandardKey key, QKeyEvent *e){return (e ? e->matches(key) : false);}
#endif // QT_NO_SHORTCUT

class QTouchEventTouchPointPrivate;
class Q_GUI_EXPORT QTouchEvent : public QInputEvent
{
public:
    class Q_GUI_EXPORT TouchPoint
    {
    public:
        enum InfoFlag {
            Pen  = 0x0001
        };
        Q_DECLARE_FLAGS(InfoFlags, InfoFlag)

        TouchPoint(int id = -1);
        TouchPoint(const QTouchEvent::TouchPoint &other);
        ~TouchPoint();

        int id() const;

        Qt::TouchPointState state() const;

        QPointF pos() const;
        QPointF startPos() const;
        QPointF lastPos() const;

        QPointF scenePos() const;
        QPointF startScenePos() const;
        QPointF lastScenePos() const;

        QPointF screenPos() const;
        QPointF startScreenPos() const;
        QPointF lastScreenPos() const;

        QPointF normalizedPos() const;
        QPointF startNormalizedPos() const;
        QPointF lastNormalizedPos() const;

        QRectF rect() const;
        QRectF sceneRect() const;
        QRectF screenRect() const;

        qreal pressure() const;
        QVector2D velocity() const;
        InfoFlags flags() const;
        QList<QPointF> rawScreenPositions() const;

        // internal
        void setId(int id);
        void setState(Qt::TouchPointStates state);
        void setPos(const QPointF &pos);
        void setScenePos(const QPointF &scenePos);
        void setScreenPos(const QPointF &screenPos);
        void setNormalizedPos(const QPointF &normalizedPos);
        void setStartPos(const QPointF &startPos);
        void setStartScenePos(const QPointF &startScenePos);
        void setStartScreenPos(const QPointF &startScreenPos);
        void setStartNormalizedPos(const QPointF &startNormalizedPos);
        void setLastPos(const QPointF &lastPos);
        void setLastScenePos(const QPointF &lastScenePos);
        void setLastScreenPos(const QPointF &lastScreenPos);
        void setLastNormalizedPos(const QPointF &lastNormalizedPos);
        void setRect(const QRectF &rect);
        void setSceneRect(const QRectF &sceneRect);
        void setScreenRect(const QRectF &screenRect);
        void setPressure(qreal pressure);
        void setVelocity(const QVector2D &v);
        void setFlags(InfoFlags flags);
        void setRawScreenPositions(const QList<QPointF> &positions);
        QTouchEvent::TouchPoint &operator=(const QTouchEvent::TouchPoint &other);

    private:
        QTouchEventTouchPointPrivate *d;
        friend class QGuiApplication;
        friend class QGuiApplicationPrivate;
        friend class QApplication;
        friend class QApplicationPrivate;
    };

    QT_DEPRECATED enum DeviceType {
        TouchScreen,
        TouchPad
    };

    QTouchEvent(QEvent::Type eventType,
                QTouchDevice *device = 0,
                Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                Qt::TouchPointStates touchPointStates = 0,
                const QList<QTouchEvent::TouchPoint> &touchPoints = QList<QTouchEvent::TouchPoint>());
    ~QTouchEvent();

    inline QWindow *window() const { return _window; }
    inline QObject *target() const { return _target; }
    QT_DEPRECATED inline QTouchEvent::DeviceType deviceType() const { return static_cast<DeviceType>(int(_device->type())); }
    inline Qt::TouchPointStates touchPointStates() const { return _touchPointStates; }
    inline const QList<QTouchEvent::TouchPoint> &touchPoints() const { return _touchPoints; }
    inline QTouchDevice *device() const { return _device; }

    // internal
    inline void setWindow(QWindow *awindow) { _window = awindow; }
    inline void setTarget(QObject *atarget) { _target = atarget; }
    inline void setTouchPointStates(Qt::TouchPointStates aTouchPointStates) { _touchPointStates = aTouchPointStates; }
    inline void setTouchPoints(const QList<QTouchEvent::TouchPoint> &atouchPoints) { _touchPoints = atouchPoints; }
    inline void setDevice(QTouchDevice *adevice) { _device = adevice; }

protected:
    QWindow *_window;
    QObject *_target;
    QTouchDevice *_device;
    Qt::TouchPointStates _touchPointStates;
    QList<QTouchEvent::TouchPoint> _touchPoints;

    friend class QGuiApplication;
    friend class QGuiApplicationPrivate;
    friend class QApplication;
    friend class QApplicationPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTouchEvent::TouchPoint::InfoFlags)

class QScrollPrepareEventPrivate;
class Q_GUI_EXPORT QScrollPrepareEvent : public QEvent
{
public:
    QScrollPrepareEvent(const QPointF &startPos);
    ~QScrollPrepareEvent();

    QPointF startPos() const;

    QSizeF viewportSize() const;
    QRectF contentPosRange() const;
    QPointF contentPos() const;

    void setViewportSize(const QSizeF &size);
    void setContentPosRange(const QRectF &rect);
    void setContentPos(const QPointF &pos);

private:
    QScrollPrepareEventPrivate *d_func();
    const QScrollPrepareEventPrivate *d_func() const;
};


class QScrollEventPrivate;
class Q_GUI_EXPORT QScrollEvent : public QEvent
{
public:
    enum ScrollState
    {
        ScrollStarted,
        ScrollUpdated,
        ScrollFinished
    };

    QScrollEvent(const QPointF &contentPos, const QPointF &overshoot, ScrollState scrollState);
    ~QScrollEvent();

    QPointF contentPos() const;
    QPointF overshootDistance() const;
    ScrollState scrollState() const;

private:
    QScrollEventPrivate *d_func();
    const QScrollEventPrivate *d_func() const;
};

class QScreenOrientationChangeEventPrivate;
class Q_GUI_EXPORT QScreenOrientationChangeEvent : public QEvent
{
public:
    QScreenOrientationChangeEvent(QScreen *screen, Qt::ScreenOrientation orientation);
    ~QScreenOrientationChangeEvent();

    QScreen *screen() const;
    Qt::ScreenOrientation orientation() const;

private:
    QScreenOrientationChangeEventPrivate *d_func();
    const QScreenOrientationChangeEventPrivate *d_func() const;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QEVENT_H
