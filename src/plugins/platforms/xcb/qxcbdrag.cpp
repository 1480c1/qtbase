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

#include "qxcbdrag.h"
#include <xcb/xcb.h>
#include "qxcbconnection.h"
#include "qxcbclipboard.h"
#include "qxcbmime.h"
#include "qxcbwindow.h"
#include "qxcbscreen.h"
#include "qwindow.h"
#include <private/qdnd_p.h>
#include <qdebug.h>
#include <qevent.h>
#include <qguiapplication.h>
#include <qrect.h>

QT_BEGIN_NAMESPACE

//#define DND_DEBUG
#ifdef DND_DEBUG
#define DEBUG qDebug
#else
#define DEBUG if(0) qDebug
#endif

#ifdef DND_DEBUG
#define DNDDEBUG qDebug()
#else
#define DNDDEBUG if(0) qDebug()
#endif

const int xdnd_version = 5;

static inline xcb_window_t xcb_window(QWindow *w)
{
    return static_cast<QXcbWindow *>(w->handle())->xcb_window();
}


static xcb_window_t xdndProxy(QXcbConnection *c, xcb_window_t w)
{
    xcb_window_t proxy = XCB_NONE;

    xcb_get_property_cookie_t cookie = Q_XCB_CALL2(xcb_get_property(c->xcb_connection(), false, w, c->atom(QXcbAtom::XdndProxy),
                                                        XCB_ATOM_WINDOW, 0, 1), c);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(c->xcb_connection(), cookie, 0);

    if (reply && reply->type == XCB_ATOM_WINDOW)
        proxy = *((xcb_window_t *)xcb_get_property_value(reply));
    free(reply);

    if (proxy == XCB_NONE)
        return proxy;

    // exists and is real?
    cookie = Q_XCB_CALL2(xcb_get_property(c->xcb_connection(), false, proxy, c->atom(QXcbAtom::XdndProxy),
                                                        XCB_ATOM_WINDOW, 0, 1), c);
    reply = xcb_get_property_reply(c->xcb_connection(), cookie, 0);

    if (reply && reply->type == XCB_ATOM_WINDOW) {
        xcb_window_t p = *((xcb_window_t *)xcb_get_property_value(reply));
        if (proxy != p)
            proxy = 0;
    } else {
        proxy = 0;
    }

    free(reply);

    return proxy;
}


class QDropData : public QXcbMime
{
public:
    QDropData(QXcbDrag *d);
    ~QDropData();

protected:
    bool hasFormat_sys(const QString &mimeType) const;
    QStringList formats_sys() const;
    QVariant retrieveData_sys(const QString &mimeType, QVariant::Type type) const;

    QVariant xdndObtainData(const QByteArray &format, QVariant::Type requestedType) const;

    QXcbDrag *drag;
};


QXcbDrag::QXcbDrag(QXcbConnection *c)
    : QXcbObject(c)
{
    dropData = new QDropData(this);

    init();
    heartbeat = -1;

    transaction_expiry_timer = -1;
}

QXcbDrag::~QXcbDrag()
{
    delete dropData;
}

void QXcbDrag::init()
{
    currentWindow.clear();

    xdnd_dragsource = XCB_NONE;
    last_target_accepted_action = Qt::IgnoreAction;

    waiting_for_status = false;
    current_target = XCB_NONE;
    current_proxy_target = XCB_NONE;
    xdnd_dragging = false;

    source_time = XCB_CURRENT_TIME;
    target_time = XCB_CURRENT_TIME;

    current_screen = 0;
    drag_types.clear();
}

QMimeData *QXcbDrag::platformDropData()
{
    return dropData;
}

void QXcbDrag::startDrag()
{
    init();

    heartbeat = startTimer(200);
    xdnd_dragging = true;

    xcb_set_selection_owner(xcb_connection(), connection()->clipboard()->owner(),
                            atom(QXcbAtom::XdndSelection), connection()->time());

    QDragManager *manager = QDragManager::self();
    QStringList fmts = QXcbMime::formatsHelper(manager->dropData());
    for (int i = 0; i < fmts.size(); ++i) {
        QList<xcb_atom_t> atoms = QXcbMime::mimeAtomsForFormat(connection(), fmts.at(i));
        for (int j = 0; j < atoms.size(); ++j) {
            if (!drag_types.contains(atoms.at(j)))
                drag_types.append(atoms.at(j));
        }
    }
    if (drag_types.size() > 3)
        xcb_change_property(xcb_connection(), XCB_PROP_MODE_REPLACE, connection()->clipboard()->owner(),
                            atom(QXcbAtom::XdndTypelist),
                            XCB_ATOM_ATOM, 32, drag_types.size(), (const void *)drag_types.constData());

    QPointF pos = QCursor::pos();
    QMouseEvent me(QEvent::MouseMove, pos, pos, pos, Qt::LeftButton,
                   QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());
    move(&me);

//    if (!QWidget::mouseGrabber())
//        manager->shapedPixmapWindow->grabMouse();
}

void QXcbDrag::endDrag()
{
    Q_ASSERT(heartbeat != -1);
    killTimer(heartbeat);
    heartbeat = -1;

    xdnd_dragging = false;
}

static xcb_translate_coordinates_reply_t *
translateCoordinates(QXcbConnection *c, xcb_window_t from, xcb_window_t to, int x, int y)
{
    xcb_translate_coordinates_cookie_t cookie =
            xcb_translate_coordinates(c->xcb_connection(), from, to, x, y);
    return xcb_translate_coordinates_reply(c->xcb_connection(), cookie, 0);
}

xcb_window_t QXcbDrag::findRealWindow(const QPoint & pos, xcb_window_t w, int md)
{
    if (w == QDragManager::self()->shapedPixmapWindow->handle()->winId())
        return 0;

    if (md) {
        xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(xcb_connection(), w);
        xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(xcb_connection(), cookie, 0);
        if (!reply)
            return 0;

        if (reply->map_state != XCB_MAP_STATE_VIEWABLE)
            return 0;

        xcb_get_geometry_cookie_t gcookie = xcb_get_geometry(xcb_connection(), w);
        xcb_get_geometry_reply_t *greply = xcb_get_geometry_reply(xcb_connection(), gcookie, 0);
        if (reply && QRect(greply->x, greply->y, greply->width, greply->height).contains(pos)) {
            bool windowContainsMouse = true;
            {
                xcb_get_property_cookie_t cookie =
                        Q_XCB_CALL(xcb_get_property(xcb_connection(), false, w, connection()->atom(QXcbAtom::XdndAware),
                                                    XCB_GET_PROPERTY_TYPE_ANY, 0, 0));
                xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection(), cookie, 0);

                bool isAware = reply && reply->type != XCB_NONE;
                free(reply);
                if (isAware) {
                    xcb_xfixes_region_t region = xcb_generate_id(xcb_connection());
                    xcb_xfixes_create_region_from_window(xcb_connection(), region, w, XCB_SHAPE_SK_BOUNDING);
                    xcb_xfixes_fetch_region_reply_t *reply = xcb_xfixes_fetch_region_reply(xcb_connection(), xcb_xfixes_fetch_region(xcb_connection(), region), NULL);
                    if (reply) {
                        xcb_rectangle_t *rectangles = xcb_xfixes_fetch_region_rectangles(reply);
                        if (rectangles) {
                            windowContainsMouse = false;
                            const int nRectangles = xcb_xfixes_fetch_region_rectangles_length(reply);
                            for (int i = 0; !windowContainsMouse && i < nRectangles; ++i) {
                                windowContainsMouse = QRect(rectangles[i].x, rectangles[i].y, rectangles[i].width, rectangles[i].height).contains(pos);
                            }
                        }
                        free(reply);
                    }
                    xcb_xfixes_destroy_region(xcb_connection(), region);

                    if (windowContainsMouse)
                        return w;
                }
            }

            xcb_query_tree_cookie_t cookie = xcb_query_tree (xcb_connection(), w);
            xcb_query_tree_reply_t *reply = xcb_query_tree_reply(xcb_connection(), cookie, 0);

            if (!reply)
                return 0;
            int nc = xcb_query_tree_children_length(reply);
            xcb_window_t *c = xcb_query_tree_children(reply);

            xcb_window_t r = 0;
            for (uint i = nc; !r && i--;)
                r = findRealWindow(pos - QPoint(greply->x, greply->y), c[i], md-1);

            free(reply);
            if (r)
                return r;

            // We didn't find a client window!  Just use the
            // innermost window.

            // No children!
            if (!windowContainsMouse)
                return 0;
            else
                return w;
        }
    }
    return 0;
}

void QXcbDrag::move(const QMouseEvent *me)
{
    DEBUG() << "QDragManager::move enter" << me->globalPos();

    // ###
    QPoint globalPos = me->globalPos();

    if (source_sameanswer.contains(globalPos) && source_sameanswer.isValid())
        return;

    const QList<QXcbScreen *> &screens = connection()->screens();
    QXcbScreen *screen = screens.at(connection()->primaryScreen());
    for (int i = 0; i < screens.size(); ++i) {
        if (screens.at(i)->geometry().contains(globalPos)) {
            screen = screens.at(i);
            break;
        }
    }
    if (screen != current_screen) {
        // ### need to recreate the shaped pixmap window?
//    int screen = QCursor::x11Screen();
//    if ((qt_xdnd_current_screen == -1 && screen != X11->defaultScreen) || (screen != qt_xdnd_current_screen)) {
//        // recreate the pixmap on the new screen...
//        delete xdnd_data.deco;
//        QWidget* parent = object->source()->window()->x11Info().screen() == screen
//            ? object->source()->window() : QApplication::desktop()->screen(screen);
//        xdnd_data.deco = new QShapedPixmapWidget(parent);
//        if (!QWidget::mouseGrabber()) {
//            updatePixmap();
//            xdnd_data.deco->grabMouse();
//        }
//    }
//    xdnd_data.deco->move(QCursor::pos() - xdnd_data.deco->pm_hot);
        current_screen = screen;
    }


//    qt_xdnd_current_screen = screen;
    xcb_window_t rootwin = current_screen->root();
    xcb_translate_coordinates_reply_t *translate =
            ::translateCoordinates(connection(), rootwin, rootwin, globalPos.x(), globalPos.y());
    if (!translate)
        return;
    xcb_window_t target = translate->child;
    int lx = translate->dst_x;
    int ly = translate->dst_y;
    free (translate);

    if (target == rootwin) {
        // Ok.
    } else if (target) {
        //me
        xcb_window_t src = rootwin;
        while (target != 0) {
            DNDDEBUG << "checking target for XdndAware" << target << lx << ly;

            // translate coordinates
            translate = ::translateCoordinates(connection(), src, target, lx, ly);
            if (!translate) {
                target = 0;
                break;
            }
            lx = translate->dst_x;
            ly = translate->dst_y;
            src = target;
            xcb_window_t child = translate->child;
            free(translate);

            // check if it has XdndAware
            xcb_get_property_cookie_t cookie = Q_XCB_CALL(xcb_get_property(xcb_connection(), false, target,
                                                          atom(QXcbAtom::XdndAware), XCB_GET_PROPERTY_TYPE_ANY, 0, 0));
            xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection(), cookie, 0);
            bool aware = reply && reply->type != XCB_NONE;
            free(reply);
            if (aware) {
                DNDDEBUG << "Found XdndAware on " << target;
                break;
            }

            target = child;
        }

        if (!target || target == QDragManager::self()->shapedPixmapWindow->handle()->winId()) {
            DNDDEBUG << "need to find real window";
            target = findRealWindow(globalPos, rootwin, 6);
            DNDDEBUG << "real window found" << target;
        }
    }

    QXcbWindow *w = 0;
    if (target) {
        w = connection()->platformWindowFromId(target);
        if (w && (w->window()->windowType() == Qt::Desktop) /*&& !w->acceptDrops()*/)
            w = 0;
    } else {
        w = 0;
        target = rootwin;
    }

    DNDDEBUG << "and the final target is " << target;
    DNDDEBUG << "the widget w is" << (w ? w->window() : 0);

    xcb_window_t proxy_target = xdndProxy(connection(), target);
    if (!proxy_target)
        proxy_target = target;
    int target_version = 1;

    if (proxy_target) {
        xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection(), false, target,
                                                            atom(QXcbAtom::XdndAware), XCB_GET_PROPERTY_TYPE_ANY, 0, 1);
        xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection(), cookie, 0);
        if (!reply || reply->type == XCB_NONE)
            target = 0;
        target_version = xcb_get_property_value_length(reply) == 1 ? *(uint32_t *)xcb_get_property_value(reply) : 1;
        if (target_version > xdnd_version)
            target_version = xdnd_version;

        free(reply);
    }

    DEBUG() << "target=" << target << "current_target=" << current_target;
    if (target != current_target) {
        if (current_target)
            send_leave();

        current_target = target;
        current_proxy_target = proxy_target;
        if (target) {
            int flags = target_version << 24;
            if (drag_types.size() > 3)
                flags |= 0x0001;

            xcb_client_message_event_t enter;
            enter.response_type = XCB_CLIENT_MESSAGE;
            enter.window = target;
            enter.format = 32;
            enter.type = atom(QXcbAtom::XdndEnter);
            enter.data.data32[0] = connection()->clipboard()->owner();
            enter.data.data32[1] = flags;
            enter.data.data32[2] = drag_types.size()>0 ? drag_types.at(0) : 0;
            enter.data.data32[3] = drag_types.size()>1 ? drag_types.at(1) : 0;
            enter.data.data32[4] = drag_types.size()>2 ? drag_types.at(2) : 0;
            // provisionally set the rectangle to 5x5 pixels...
            source_sameanswer = QRect(globalPos.x() - 2, globalPos.y() -2 , 5, 5);

            DEBUG() << "sending Xdnd enter source=" << enter.data.data32[0];
            if (w)
                handleEnter(w->window(), &enter);
            else if (target)
                xcb_send_event(xcb_connection(), false, proxy_target, XCB_EVENT_MASK_NO_EVENT, (const char *)&enter);
            waiting_for_status = false;
        }
    }
    if (waiting_for_status)
        return;

    QDragManager *m = QDragManager::self();

    if (target) {
        waiting_for_status = true;

        xcb_client_message_event_t move;
        move.response_type = XCB_CLIENT_MESSAGE;
        move.window = target;
        move.format = 32;
        move.type = atom(QXcbAtom::XdndPosition);
        move.window = target;
        move.data.data32[0] = connection()->clipboard()->owner();
        move.data.data32[1] = 0; // flags
        move.data.data32[2] = (globalPos.x() << 16) + globalPos.y();
        move.data.data32[3] = connection()->time();
        move.data.data32[4] = toXdndAction(m->defaultAction(m->dragPrivate()->possible_actions, QGuiApplication::keyboardModifiers()));
        DEBUG() << "sending Xdnd position source=" << move.data.data32[0] << "target=" << move.window;

        source_time = connection()->time();

        if (w)
            handle_xdnd_position(w->window(), &move, false);
        else
            xcb_send_event(xcb_connection(), false, proxy_target, XCB_EVENT_MASK_NO_EVENT, (const char *)&move);
    } else {
        if (m->willDrop) {
            m->willDrop = false;
        }
    }
    m->updateCursor();
    DEBUG() << "QDragManager::move leave";
}

void QXcbDrag::drop(const QMouseEvent *)
{
    endDrag();

    if (!current_target)
        return;

    xcb_client_message_event_t drop;
    drop.response_type = XCB_CLIENT_MESSAGE;
    drop.window = current_target;
    drop.format = 32;
    drop.type = atom(QXcbAtom::XdndDrop);
    drop.data.data32[0] = connection()->clipboard()->owner();
    drop.data.data32[1] = 0; // flags
    drop.data.data32[2] = connection()->time();

    drop.data.data32[3] = 0;
    drop.data.data32[4] = 0;

    QXcbWindow *w = connection()->platformWindowFromId(current_proxy_target);

    if (w && (w->window()->windowType() == Qt::Desktop) /*&& !w->acceptDrops()*/)
        w = 0;

    QDragManager *manager = QDragManager::self();

    Transaction t = {
        connection()->time(),
        current_target,
        current_proxy_target,
        (w ? w->window() : 0),
//        current_embedding_widget,
        manager->object
    };
    transactions.append(t);
    restartDropExpiryTimer();

    if (w)
        handleDrop(w->window(), &drop, false);
    else
        xcb_send_event(xcb_connection(), false, current_proxy_target, XCB_EVENT_MASK_NO_EVENT, (const char *)&drop);

    current_target = 0;
    current_proxy_target = 0;
    source_time = 0;
//    current_embedding_widget = 0;
    manager->object = 0;
}

Qt::DropAction QXcbDrag::toDropAction(xcb_atom_t a) const
{
    if (a == atom(QXcbAtom::XdndActionCopy) || a == 0)
        return Qt::CopyAction;
    if (a == atom(QXcbAtom::XdndActionLink))
        return Qt::LinkAction;
    if (a == atom(QXcbAtom::XdndActionMove))
        return Qt::MoveAction;
    return Qt::CopyAction;
}

xcb_atom_t QXcbDrag::toXdndAction(Qt::DropAction a) const
{
    switch (a) {
    case Qt::CopyAction:
        return atom(QXcbAtom::XdndActionCopy);
    case Qt::LinkAction:
        return atom(QXcbAtom::XdndActionLink);
    case Qt::MoveAction:
    case Qt::TargetMoveAction:
        return atom(QXcbAtom::XdndActionMove);
    case Qt::IgnoreAction:
        return XCB_NONE;
    default:
        return atom(QXcbAtom::XdndActionCopy);
    }
}

// timer used to discard old XdndDrop transactions
enum { XdndDropTransactionTimeout = 5000 }; // 5 seconds

void QXcbDrag::restartDropExpiryTimer()
{
    if (transaction_expiry_timer != -1)
        killTimer(transaction_expiry_timer);
    transaction_expiry_timer = startTimer(XdndDropTransactionTimeout);
}

int QXcbDrag::findTransactionByWindow(xcb_window_t window)
{
    int at = -1;
    for (int i = 0; i < transactions.count(); ++i) {
        const Transaction &t = transactions.at(i);
        if (t.target == window || t.proxy_target == window) {
            at = i;
            break;
        }
    }
    return at;
}

int QXcbDrag::findTransactionByTime(xcb_timestamp_t timestamp)
{
    int at = -1;
    for (int i = 0; i < transactions.count(); ++i) {
        const Transaction &t = transactions.at(i);
        if (t.timestamp == timestamp) {
            at = i;
            break;
        }
    }
    return at;
}

#if 0

// find an ancestor with XdndAware on it
static Window findXdndAwareParent(Window window)
{
    Window target = 0;
    forever {
        // check if window has XdndAware
        Atom type = 0;
        int f;
        unsigned long n, a;
        unsigned char *data = 0;
        if (XGetWindowProperty(X11->display, window, ATOM(XdndAware), 0, 0, False,
                               AnyPropertyType, &type, &f,&n,&a,&data) == Success) {
	    if (data)
                XFree(data);
	    if (type) {
                target = window;
                break;
            }
        }

        // try window's parent
        Window root;
        Window parent;
        Window *children;
        uint unused;
        if (!XQueryTree(X11->display, window, &root, &parent, &children, &unused))
            break;
        if (children)
            XFree(children);
        if (window == root)
            break;
        window = parent;
    }
    return target;
}


// for embedding only
static QWidget* current_embedding_widget  = 0;
static xcb_client_message_event_t last_enter_event;


static bool checkEmbedded(QWidget* w, const XEvent* xe)
{
    if (!w)
        return false;

    if (current_embedding_widget != 0 && current_embedding_widget != w) {
        current_target = ((QExtraWidget*)current_embedding_widget)->extraData()->xDndProxy;
        current_proxy_target = current_target;
        qt_xdnd_send_leave();
        current_target = 0;
        current_proxy_target = 0;
        current_embedding_widget = 0;
    }

    QWExtra* extra = ((QExtraWidget*)w)->extraData();
    if (extra && extra->xDndProxy != 0) {

        if (current_embedding_widget != w) {

            last_enter_event.xany.window = extra->xDndProxy;
            XSendEvent(X11->display, extra->xDndProxy, False, NoEventMask, &last_enter_event);
            current_embedding_widget = w;
        }

        ((XEvent*)xe)->xany.window = extra->xDndProxy;
        XSendEvent(X11->display, extra->xDndProxy, False, NoEventMask, (XEvent*)xe);
        if (currentWindow != w) {
            currentWindow = w;
        }
        return true;
    }
    current_embedding_widget = 0;
    return false;
}
#endif


void QXcbDrag::handleEnter(QWindow *window, const xcb_client_message_event_t *event)
{
    Q_UNUSED(window);
    DEBUG() << "handleEnter" << window;

    xdnd_types.clear();
//    motifdnd_active = false;
//    last_enter_event.xclient = xe->xclient;

    int version = (int)(event->data.data32[1] >> 24);
    if (version > xdnd_version)
        return;

    xdnd_dragsource = event->data.data32[0];

    if (event->data.data32[1] & 1) {
        // get the types from XdndTypeList
        xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection(), false, xdnd_dragsource,
                                                            atom(QXcbAtom::XdndTypelist), XCB_ATOM_ATOM,
                                                            0, xdnd_max_type);
        xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection(), cookie, 0);
        if (reply && reply->type != XCB_NONE && reply->format == 32) {
            int length = xcb_get_property_value_length(reply) / 4;
            if (length > xdnd_max_type)
                length = xdnd_max_type;

            xcb_atom_t *atoms = (xcb_atom_t *)xcb_get_property_value(reply);
            for (int i = 0; i < length; ++i)
                xdnd_types.append(atoms[i]);
        }
        free(reply);
    } else {
        // get the types from the message
        for(int i = 2; i < 5; i++) {
            if (event->data.data32[i])
                xdnd_types.append(event->data.data32[i]);
        }
    }
    for(int i = 0; i < xdnd_types.length(); ++i)
        DEBUG() << "    " << connection()->atomName(xdnd_types.at(i));
}

void QXcbDrag::handle_xdnd_position(QWindow *w, const xcb_client_message_event_t *e, bool passive)
{
    QPoint p((e->data.data32[2] & 0xffff0000) >> 16, e->data.data32[2] & 0x0000ffff);
    Q_ASSERT(w);
    QRect geometry = w->geometry();

    p -= geometry.topLeft();

    // ####
//    if (!passive && checkEmbedded(w, e))
//        return;

    if (!w || (/*!w->acceptDrops() &&*/ (w->windowType() == Qt::Desktop)))
        return;

    if (e->data.data32[0] != xdnd_dragsource) {
        DEBUG("xdnd drag position from unexpected source (%x not %x)", e->data.data32[0], xdnd_dragsource);
        return;
    }

    // timestamp from the source
    if (e->data.data32[3] != XCB_NONE)
        target_time /*= X11->userTime*/ = e->data.data32[3];

    QDragManager *manager = QDragManager::self();
    QMimeData *dropData = manager->dropData();

    xcb_client_message_event_t response;
    response.response_type = XCB_CLIENT_MESSAGE;
    response.window = xdnd_dragsource;
    response.format = 32;
    response.type = atom(QXcbAtom::XdndStatus);
    response.data.data32[0] = xcb_window(w);
    response.data.data32[1] = 0; // flags
    response.data.data32[2] = 0; // x, y
    response.data.data32[3] = 0; // w, h
    response.data.data32[4] = 0; // action

    if (!passive) { // otherwise just reject
        QRect answerRect(p + geometry.topLeft(), QSize(1,1));

        if (manager->object) {
            manager->possible_actions = manager->dragPrivate()->possible_actions;
        } else {
            manager->possible_actions = Qt::DropActions(toDropAction(e->data.data32[4]));
        }
        QDragMoveEvent me(p, manager->possible_actions, dropData,
                          QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());

        Qt::DropAction accepted_action = Qt::IgnoreAction;

        currentPosition = p;

        if (w != currentWindow.data()) {
            if (currentWindow) {
                QDragLeaveEvent e;
                QGuiApplication::sendEvent(currentWindow.data(), &e);
            }
            currentWindow = w;

            last_target_accepted_action = Qt::IgnoreAction;
            QDragEnterEvent de(p, manager->possible_actions, dropData,
                               QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());
            QGuiApplication::sendEvent(w, &de);
            if (de.isAccepted() && de.dropAction() != Qt::IgnoreAction)
                last_target_accepted_action = de.dropAction();
        }

        DEBUG() << "qt_handle_xdnd_position action=" << connection()->atomName(e->data.data32[4]);

        if (last_target_accepted_action != Qt::IgnoreAction) {
            me.setDropAction(last_target_accepted_action);
            me.accept();
        }
        QGuiApplication::sendEvent(w, &me);
        if (me.isAccepted()) {
            response.data.data32[1] = 1; // yes
            accepted_action = me.dropAction();
            last_target_accepted_action = accepted_action;
        } else {
            response.data.data32[0] = 0;
            last_target_accepted_action = Qt::IgnoreAction;
        }
        answerRect = me.answerRect().translated(geometry.topLeft()).intersected(geometry);

        if (answerRect.left() < 0)
            answerRect.setLeft(0);
        if (answerRect.right() > 4096)
            answerRect.setRight(4096);
        if (answerRect.top() < 0)
            answerRect.setTop(0);
        if (answerRect.bottom() > 4096)
            answerRect.setBottom(4096);
        if (answerRect.width() < 0)
            answerRect.setWidth(0);
        if (answerRect.height() < 0)
            answerRect.setHeight(0);

//        response.data.data32[2] = (answerRect.x() << 16) + answerRect.y();
//        response.data.data32[3] = (answerRect.width() << 16) + answerRect.height();
        response.data.data32[4] = toXdndAction(accepted_action);
    }

    // reset
    target_time = XCB_CURRENT_TIME;

    DEBUG() << "sending XdndStatus" << (xdnd_dragsource == connection()->clipboard()->owner()) << xdnd_dragsource
            << response.data.data32[1] << connection()->atomName(response.data.data32[4]);
    if (xdnd_dragsource == connection()->clipboard()->owner())
        handle_xdnd_status(&response, passive);
    else
        Q_XCB_CALL(xcb_send_event(xcb_connection(), false, xdnd_dragsource,
                                  XCB_EVENT_MASK_NO_EVENT, (const char *)&response));
}

namespace
{
    class ClientMessageScanner {
    public:
        ClientMessageScanner(xcb_atom_t a) : atom(a) {}
        xcb_atom_t atom;
        bool checkEvent(xcb_generic_event_t *event) const {
            if (!event)
                return false;
            if ((event->response_type & 0x7f) != XCB_CLIENT_MESSAGE)
                return false;
            return ((xcb_client_message_event_t *)event)->type == atom;
        }
    };
}

void QXcbDrag::handlePosition(QWindow * w, const xcb_client_message_event_t *event, bool passive)
{
    xcb_client_message_event_t *lastEvent = const_cast<xcb_client_message_event_t *>(event);
    xcb_generic_event_t *nextEvent;
    ClientMessageScanner scanner(atom(QXcbAtom::XdndPosition));
    while ((nextEvent = connection()->checkEvent(scanner))) {
        if (lastEvent != event)
            free(lastEvent);
        lastEvent = (xcb_client_message_event_t *)nextEvent;
    }

    handle_xdnd_position(w, lastEvent, passive);
    if (lastEvent != event)
        free(lastEvent);
}

void QXcbDrag::handle_xdnd_status(const xcb_client_message_event_t *event, bool)
{
    DEBUG("xdndHandleStatus");
    // ignore late status messages
    if (event->data.data32[0] && event->data.data32[0] != current_proxy_target)
        return;

    Qt::DropAction newAction = (event->data.data32[1] & 0x1) ? toDropAction(event->data.data32[4]) : Qt::IgnoreAction;

    if ((event->data.data32[1] & 2) == 0) {
        QPoint p((event->data.data32[2] & 0xffff0000) >> 16, event->data.data32[2] & 0x0000ffff);
        QSize s((event->data.data32[3] & 0xffff0000) >> 16, event->data.data32[3] & 0x0000ffff);
        source_sameanswer = QRect(p, s);
    } else {
        source_sameanswer = QRect();
    }
    QDragManager *manager = QDragManager::self();
    manager->willDrop = (event->data.data32[1] & 0x1);
    if (manager->global_accepted_action != newAction) {
        manager->global_accepted_action = newAction;
        manager->emitActionChanged(newAction);
    }
    DEBUG() << "willDrop=" << manager->willDrop << "action=" << newAction;
    manager->updateCursor();
    waiting_for_status = false;
}

void QXcbDrag::handleStatus(const xcb_client_message_event_t *event, bool passive)
{
    if (event->window != connection()->clipboard()->owner())
        return;

    xcb_client_message_event_t *lastEvent = const_cast<xcb_client_message_event_t *>(event);
    xcb_generic_event_t *nextEvent;
    ClientMessageScanner scanner(atom(QXcbAtom::XdndStatus));
    while ((nextEvent = connection()->checkEvent(scanner))) {
        if (lastEvent != event)
            free(lastEvent);
        lastEvent = (xcb_client_message_event_t *)nextEvent;
    }

    handle_xdnd_status(lastEvent, passive);
    if (lastEvent != event)
        free(lastEvent);
    DEBUG("xdndHandleStatus end");
}

void QXcbDrag::handleLeave(QWindow *w, const xcb_client_message_event_t *event, bool /*passive*/)
{
    DEBUG("xdnd leave");
    if (!currentWindow || w != currentWindow.data())
        return; // sanity

    // ###
//    if (checkEmbedded(current_embedding_widget, event)) {
//        current_embedding_widget = 0;
//        currentWindow.clear();
//        return;
//    }

    if (event->data.data32[0] != xdnd_dragsource) {
        // This often happens - leave other-process window quickly
        DEBUG("xdnd drag leave from unexpected source (%x not %x", event->data.data32[0], xdnd_dragsource);
    }

    QDragLeaveEvent e;
    QGuiApplication::sendEvent(currentWindow.data(), &e);

    xdnd_dragsource = 0;
    xdnd_types.clear();
    currentWindow.clear();
}

void QXcbDrag::send_leave()
{
    if (!current_target)
        return;

    QDragManager *manager = QDragManager::self();

    xcb_client_message_event_t leave;
    leave.response_type = XCB_CLIENT_MESSAGE;
    leave.window = current_target;
    leave.format = 32;
    leave.type = atom(QXcbAtom::XdndLeave);
    leave.data.data32[0] = connection()->clipboard()->owner();
    leave.data.data32[1] = 0; // flags
    leave.data.data32[2] = 0; // x, y
    leave.data.data32[3] = 0; // w, h
    leave.data.data32[4] = 0; // just null

    QXcbWindow *w = connection()->platformWindowFromId(current_proxy_target);

    if (w && (w->window()->windowType() == Qt::Desktop) /*&& !w->acceptDrops()*/)
        w = 0;

    if (w)
        handleLeave(w->window(), (const xcb_client_message_event_t *)&leave, false);
    else
        xcb_send_event(xcb_connection(), false,current_proxy_target,
                       XCB_EVENT_MASK_NO_EVENT, (const char *)&leave);

    // reset the drag manager state
    manager->willDrop = false;
    if (manager->global_accepted_action != Qt::IgnoreAction)
        manager->emitActionChanged(Qt::IgnoreAction);
    manager->global_accepted_action = Qt::IgnoreAction;
    manager->updateCursor();
    current_target = 0;
    current_proxy_target = 0;
    source_time = XCB_CURRENT_TIME;
    waiting_for_status = false;
}

void QXcbDrag::handleDrop(QWindow *, const xcb_client_message_event_t *event, bool passive)
{
    DEBUG("xdndHandleDrop");
    if (!currentWindow) {
        xdnd_dragsource = 0;
        return; // sanity
    }

    // ###
//    if (!passive && checkEmbedded(currentWindow, xe)){
//        current_embedding_widget = 0;
//        xdnd_dragsource = 0;
//        currentWindow = 0;
//        return;
//    }
    const uint32_t *l = event->data.data32;

    QDragManager *manager = QDragManager::self();
    DEBUG("xdnd drop");

    if (l[0] != xdnd_dragsource) {
        DEBUG("xdnd drop from unexpected source (%x not %x", l[0], xdnd_dragsource);
        return;
    }

    // update the "user time" from the timestamp in the event.
    if (l[2] != 0)
        target_time = /*X11->userTime =*/ l[2];

    if (!passive) {
        // this could be a same-application drop, just proxied due to
        // some XEMBEDding, so try to find the real QMimeData used
        // based on the timestamp for this drop.
        QMimeData *dropData = 0;
        // ###
//        int at = findXdndDropTransactionByTime(target_time);
//        if (at != -1)
//            dropData = QDragManager::dragPrivate(X11->dndDropTransactions.at(at).object)->data;
        // if we can't find it, then use the data in the drag manager
        if (!dropData)
            dropData = manager->dropData();

        // Drop coming from another app? Update keyboard modifiers.
//        if (!qt_xdnd_dragging) {
//            QApplicationPrivate::modifier_buttons = currentKeyboardModifiers();
//        }

        QDropEvent de(currentPosition, manager->possible_actions, dropData,
                      QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());
        QGuiApplication::sendEvent(currentWindow.data(), &de);
        if (!de.isAccepted()) {
            // Ignore a failed drag
            manager->global_accepted_action = Qt::IgnoreAction;
        } else {
            manager->global_accepted_action = de.dropAction();
        }
        xcb_client_message_event_t finished;
        finished.response_type = XCB_CLIENT_MESSAGE;
        finished.window = xdnd_dragsource;
        finished.format = 32;
        finished.type = atom(QXcbAtom::XdndFinished);
        DNDDEBUG << "xdndHandleDrop"
             << "currentWindow" << currentWindow.data()
             << (currentWindow ? xcb_window(currentWindow.data()) : 0);
        finished.data.data32[0] = currentWindow ? xcb_window(currentWindow.data()) : XCB_NONE;
        finished.data.data32[1] = de.isAccepted() ? 1 : 0; // flags
        finished.data.data32[2] = toXdndAction(manager->global_accepted_action);
        Q_XCB_CALL(xcb_send_event(xcb_connection(), false, xdnd_dragsource,
                       XCB_EVENT_MASK_NO_EVENT, (char *)&finished));
    } else {
        QDragLeaveEvent e;
        QGuiApplication::sendEvent(currentWindow.data(), &e);
    }
    xdnd_dragsource = 0;
    currentWindow.clear();
    waiting_for_status = false;

    // reset
    target_time = XCB_CURRENT_TIME;
}


void QXcbDrag::handleFinished(const xcb_client_message_event_t *event, bool)
{
    DEBUG("xdndHandleFinished");
    if (event->window != connection()->clipboard()->owner())
        return;

    const unsigned long *l = (const unsigned long *)event->data.data32;

    DNDDEBUG << "xdndHandleFinished, l[0]" << l[0]
             << "current_target" << current_target
             << "qt_xdnd_current_proxy_targe" << current_proxy_target;

    if (l[0]) {
        int at = findTransactionByWindow(l[0]);
        if (at != -1) {
            restartDropExpiryTimer();

            Transaction t = transactions.takeAt(at);
//            QDragManager *manager = QDragManager::self();

//            Window target = current_target;
//            Window proxy_target = current_proxy_target;
//            QWidget *embedding_widget = current_embedding_widget;
//            QDrag *currentObject = manager->object;

//            current_target = t.target;
//            current_proxy_target = t.proxy_target;
//            current_embedding_widget = t.embedding_widget;
//            manager->object = t.object;

//            if (!passive)
//                (void) checkEmbedded(currentWindow, xe);

//            current_embedding_widget = 0;
//            current_target = 0;
//            current_proxy_target = 0;

            if (t.object)
                t.object->deleteLater();

//            current_target = target;
//            current_proxy_target = proxy_target;
//            current_embedding_widget = embedding_widget;
//            manager->object = currentObject;
        }
    }
    waiting_for_status = false;
}


void QXcbDrag::timerEvent(QTimerEvent* e)
{
    if (e->timerId() == heartbeat && source_sameanswer.isNull()) {
        QPointF pos = QCursor::pos();
        QMouseEvent me(QEvent::MouseMove, pos, pos, pos, Qt::LeftButton,
                       QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());
        move(&me);
    } else if (e->timerId() == transaction_expiry_timer) {
        for (int i = 0; i < transactions.count(); ++i) {
            const Transaction &t = transactions.at(i);
            if (t.targetWindow) {
                // dnd within the same process, don't delete these
                continue;
            }
            t.object->deleteLater();
            transactions.removeAt(i--);
        }

        killTimer(transaction_expiry_timer);
        transaction_expiry_timer = -1;
    }
}

void QXcbDrag::cancel()
{
    DEBUG("QXcbDrag::cancel");
    endDrag();

    if (current_target)
        send_leave();

    current_target = 0;
}


void QXcbDrag::handleSelectionRequest(const xcb_selection_request_event_t *event)
{
    xcb_selection_notify_event_t notify;
    notify.response_type = XCB_SELECTION_NOTIFY;
    notify.requestor = event->requestor;
    notify.selection = event->selection;
    notify.target = XCB_NONE;
    notify.property = XCB_NONE;
    notify.time = event->time;

    QDragManager *manager = QDragManager::self();
    QDrag *currentObject = manager->object;

    // which transaction do we use? (note: -2 means use current manager->object)
    int at = -1;

    // figure out which data the requestor is really interested in
    if (manager->object && event->time == source_time) {
        // requestor wants the current drag data
        at = -2;
    } else {
        // if someone has requested data in response to XdndDrop, find the corresponding transaction. the
        // spec says to call XConvertSelection() using the timestamp from the XdndDrop
        at = findTransactionByTime(event->time);
        if (at == -1) {
            // no dice, perhaps the client was nice enough to use the same window id in XConvertSelection()
            // that we sent the XdndDrop event to.
            at = findTransactionByWindow(event->requestor);
        }
//        if (at == -1 && event->time == XCB_CURRENT_TIME) {
//            // previous Qt versions always requested the data on a child of the target window
//            // using CurrentTime... but it could be asking for either drop data or the current drag's data
//            Window target = findXdndAwareParent(event->requestor);
//            if (target) {
//                if (current_target && current_target == target)
//                    at = -2;
//                else
//                    at = findXdndDropTransactionByWindow(target);
//            }
//        }
    }
    if (at >= 0) {
        restartDropExpiryTimer();

        // use the drag object from an XdndDrop tansaction
        manager->object = transactions.at(at).object;
    } else if (at != -2) {
        // no transaction found, we'll have to reject the request
        manager->object = 0;
    }
    if (manager->object) {
        xcb_atom_t atomFormat = event->target;
        int dataFormat = 0;
        QByteArray data;
        if (QXcbMime::mimeDataForAtom(connection(), event->target, manager->dragPrivate()->data,
                                     &data, &atomFormat, &dataFormat)) {
            int dataSize = data.size() / (dataFormat / 8);
            xcb_change_property(xcb_connection(), XCB_PROP_MODE_REPLACE, event->requestor, event->property,
                                atomFormat, dataFormat, dataSize, (const void *)data.constData());
            notify.property = event->property;
            notify.target = atomFormat;
        }
    }

    // reset manager->object in case we modified it above
    manager->object = currentObject;

    xcb_send_event(xcb_connection(), false, event->requestor, XCB_EVENT_MASK_NO_EVENT, (const char *)&notify);
}


bool QXcbDrag::dndEnable(QXcbWindow *w, bool on)
{
    DNDDEBUG << "xdndEnable" << w << on;
    if (on) {
        QXcbWindow *xdnd_widget = 0;
        if ((w->window()->windowType() == Qt::Desktop)) {
            if (desktop_proxy) // *WE* already have one.
                return false;

            xcb_grab_server(xcb_connection());

            // As per Xdnd4, use XdndProxy
            xcb_window_t proxy_id = xdndProxy(connection(), w->xcb_window());

            if (!proxy_id) {
                desktop_proxy = new QWindow;
                xdnd_widget = static_cast<QXcbWindow *>(desktop_proxy->handle());
                proxy_id = xdnd_widget->xcb_window();
                xcb_atom_t xdnd_proxy = atom(QXcbAtom::XdndProxy);
                xcb_change_property(xcb_connection(), XCB_PROP_MODE_REPLACE, w->xcb_window(), xdnd_proxy,
                                    XCB_ATOM_WINDOW, 32, 1, &proxy_id);
                xcb_change_property(xcb_connection(), XCB_PROP_MODE_REPLACE, proxy_id, xdnd_proxy,
                                    XCB_ATOM_WINDOW, 32, 1, &proxy_id);
            }

            xcb_ungrab_server(xcb_connection());
        } else {
            xdnd_widget = w;
        }
        if (xdnd_widget) {
            DNDDEBUG << "setting XdndAware for" << xdnd_widget << xdnd_widget->xcb_window();
            xcb_atom_t atm = xdnd_version;
            xcb_change_property(xcb_connection(), XCB_PROP_MODE_REPLACE, xdnd_widget->xcb_window(),
                                atom(QXcbAtom::XdndAware), XCB_ATOM_ATOM, 32, 1, &atm);
            return true;
        } else {
            return false;
        }
    } else {
        if ((w->window()->windowType() == Qt::Desktop)) {
            xcb_delete_property(xcb_connection(), w->xcb_window(), atom(QXcbAtom::XdndProxy));
            delete desktop_proxy;
            desktop_proxy = 0;
        } else {
            DNDDEBUG << "not deleting XDndAware";
        }
        return true;
    }
}




QDropData::QDropData(QXcbDrag *d)
    : QXcbMime(),
      drag(d)
{
}

QDropData::~QDropData()
{
}

QVariant QDropData::retrieveData_sys(const QString &mimetype, QVariant::Type requestedType) const
{
    QByteArray mime = mimetype.toLatin1();
    QVariant data = /*X11->motifdnd_active
                      ? X11->motifdndObtainData(mime)
                      :*/ xdndObtainData(mime, requestedType);
    return data;
}

QVariant QDropData::xdndObtainData(const QByteArray &format, QVariant::Type requestedType) const
{
    QByteArray result;

    QDragManager *manager = QDragManager::self();
    QXcbConnection *c = drag->connection();
    QXcbWindow *xcb_window = c->platformWindowFromId(drag->xdnd_dragsource);
    if (xcb_window && manager->object && xcb_window->window()->windowType() != Qt::Desktop) {
        QDragPrivate *o = manager->dragPrivate();
        if (o->data->hasFormat(QLatin1String(format)))
            result = o->data->data(QLatin1String(format));
        return result;
    }

    QList<xcb_atom_t> atoms = drag->xdnd_types;
    QByteArray encoding;
    xcb_atom_t a = mimeAtomForFormat(c, QLatin1String(format), requestedType, atoms, &encoding);
    if (a == XCB_NONE)
        return result;

    if (c->clipboard()->getSelectionOwner(drag->atom(QXcbAtom::XdndSelection)) == XCB_NONE)
        return result; // should never happen?

    xcb_atom_t xdnd_selection = c->atom(QXcbAtom::XdndSelection);
    result = c->clipboard()->getSelection(xdnd_selection, a, xdnd_selection);

    return mimeConvertToFormat(c, a, result, QLatin1String(format), requestedType, encoding);
}


bool QDropData::hasFormat_sys(const QString &format) const
{
    return formats().contains(format);
}

QStringList QDropData::formats_sys() const
{
    QStringList formats;
//    if (X11->motifdnd_active) {
//        int i = 0;
//        QByteArray fmt;
//        while (!(fmt = X11->motifdndFormat(i)).isEmpty()) {
//            formats.append(QLatin1String(fmt));
//            ++i;
//        }
//    } else {
        for (int i = 0; i < drag->xdnd_types.size(); ++i) {
            QString f = mimeAtomToString(drag->connection(), drag->xdnd_types.at(i));
            if (!formats.contains(f))
                formats.append(f);
        }
//    }
    return formats;
}

QT_END_NAMESPACE
