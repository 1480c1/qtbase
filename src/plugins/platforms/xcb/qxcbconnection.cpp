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

#include <QtGui/private/qguiapplication_p.h>
#include <QtCore/QDebug>

#include "qxcbconnection.h"
#include "qxcbkeyboard.h"
#include "qxcbscreen.h"
#include "qxcbwindow.h"
#include "qxcbclipboard.h"
#include "qxcbdrag.h"
#include "qxcbwmsupport.h"
#include "qxcbnativeinterface.h"

#include <QtAlgorithms>
#include <QSocketNotifier>
#include <QAbstractEventDispatcher>
#include <QTimer>
#include <QByteArray>

#include <stdio.h>
#include <errno.h>
#include <xcb/xfixes.h>

#ifdef XCB_USE_XLIB
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xlibint.h>
#endif

#ifdef XCB_USE_RENDER
#include <xcb/render.h>
#endif

#ifdef XCB_USE_EGL //dont pull in eglext prototypes
#include <EGL/egl.h>
#endif

#ifdef XCB_USE_DRI2
#include <xcb/dri2.h>
extern "C" {
#include <xf86drm.h>
}
#define MESA_EGL_NO_X11_HEADERS
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

QT_BEGIN_NAMESPACE

#ifdef XCB_USE_XLIB
static int nullErrorHandler(Display *, XErrorEvent *)
{
    return 0;
}
#endif

QXcbConnection::QXcbConnection(QXcbNativeInterface *nativeInterface, const char *displayName)
    : m_connection(0)
    , m_displayName(displayName ? QByteArray(displayName) : qgetenv("DISPLAY"))
    , m_nativeInterface(nativeInterface)
#ifdef XCB_USE_XINPUT2_MAEMO
    , m_xinputData(0)
#endif
#ifdef XCB_USE_DRI2
    , m_dri2_major(0)
    , m_dri2_minor(0)
    , m_dri2_support_probed(false)
    , m_has_support_for_dri2(false)
#endif
    , xfixes_first_event(0)
{
    m_primaryScreen = 0;

#ifdef XCB_USE_XLIB
    Display *dpy = XOpenDisplay(m_displayName.constData());
    if (dpy) {
        m_primaryScreen = DefaultScreen(dpy);
        m_connection = XGetXCBConnection(dpy);
        XSetEventQueueOwner(dpy, XCBOwnsEventQueue);
        XSetErrorHandler(nullErrorHandler);
        m_xlib_display = dpy;
#ifdef XCB_USE_EGL
        EGLDisplay eglDisplay = eglGetDisplay(dpy);
        m_egl_display = eglDisplay;
        EGLint major, minor;
        eglBindAPI(EGL_OPENGL_ES_API);
        m_has_egl = eglInitialize(eglDisplay,&major,&minor);
#endif //XCB_USE_EGL
    }
#else
    m_connection = xcb_connect(m_displayName.constData(), &m_primaryScreen);
#endif //XCB_USE_XLIB

    if (!m_connection)
        qFatal("Could not connect to display %s", m_displayName.constData());

    m_reader = new QXcbEventReader(this);
#ifdef XCB_POLL_FOR_QUEUED_EVENT
    connect(m_reader, SIGNAL(eventPending()), this, SLOT(processXcbEvents()), Qt::QueuedConnection);
    m_reader->start();
#else
    QSocketNotifier *notifier = new QSocketNotifier(xcb_get_file_descriptor(xcb_connection()), QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(processXcbEvents()));

    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    connect(dispatcher, SIGNAL(aboutToBlock()), this, SLOT(processXcbEvents()));
    connect(dispatcher, SIGNAL(awake()), this, SLOT(processXcbEvents()));
#endif

    xcb_prefetch_extension_data (m_connection, &xcb_xfixes_id);

    m_setup = xcb_get_setup(xcb_connection());

    initializeAllAtoms();

    m_time = XCB_CURRENT_TIME;

    xcb_screen_iterator_t it = xcb_setup_roots_iterator(m_setup);

    int screenNumber = 0;
    while (it.rem) {
        m_screens << new QXcbScreen(this, it.data, screenNumber++);
        xcb_screen_next(&it);
    }

    m_connectionEventListener = xcb_generate_id(m_connection);
    xcb_create_window(m_connection, XCB_COPY_FROM_PARENT,
                      m_connectionEventListener, m_screens.at(0)->root(),
                      0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_ONLY,
                      m_screens.at(0)->screen()->root_visual, 0, 0);

    initializeXFixes();
    initializeXRender();
#ifdef XCB_USE_XINPUT2_MAEMO
    initializeXInput2();
#endif

    m_wmSupport.reset(new QXcbWMSupport(this));
    m_keyboard = new QXcbKeyboard(this);
    m_clipboard = new QXcbClipboard(this);
    m_drag = new QXcbDrag(this);

#ifdef XCB_USE_DRI2
    initializeDri2();
#endif
    sync();
}

QXcbConnection::~QXcbConnection()
{
    delete m_clipboard;

    qDeleteAll(m_screens);

#ifdef XCB_USE_XINPUT2_MAEMO
    finalizeXInput2();
#endif

#ifdef XCB_POLL_FOR_QUEUED_EVENT
    sendConnectionEvent(QXcbAtom::_QT_CLOSE_CONNECTION);
    m_reader->wait();
#endif
    delete m_reader;

#ifdef XCB_USE_XLIB
    XCloseDisplay((Display *)m_xlib_display);
#else
    xcb_disconnect(xcb_connection());
#endif

    delete m_keyboard;
}

void QXcbConnection::addWindow(xcb_window_t id, QXcbWindow *window)
{
    m_mapper.insert(id, window);
}

void QXcbConnection::removeWindow(xcb_window_t id)
{
    m_mapper.remove(id);
}

QXcbWindow *QXcbConnection::platformWindowFromId(xcb_window_t id)
{
    return m_mapper.value(id, 0);
}

#define HANDLE_PLATFORM_WINDOW_EVENT(event_t, windowMember, handler) \
{ \
    event_t *e = (event_t *)event; \
    if (QXcbWindow *platformWindow = platformWindowFromId(e->windowMember)) \
        platformWindow->handler(e); \
} \
break;

#define HANDLE_KEYBOARD_EVENT(event_t, handler) \
{ \
    event_t *e = (event_t *)event; \
    if (QXcbWindow *platformWindow = platformWindowFromId(e->event)) \
        m_keyboard->handler(platformWindow, e); \
} \
break;

//#define XCB_EVENT_DEBUG

void printXcbEvent(const char *message, xcb_generic_event_t *event)
{
#ifdef XCB_EVENT_DEBUG
#define PRINT_XCB_EVENT(ev) \
    case ev: \
        qDebug("%s: %d - %s - sequence: %d", message, int(ev), #ev, event->sequence); \
        break;

    switch (event->response_type & ~0x80) {
    PRINT_XCB_EVENT(XCB_KEY_PRESS);
    PRINT_XCB_EVENT(XCB_KEY_RELEASE);
    PRINT_XCB_EVENT(XCB_BUTTON_PRESS);
    PRINT_XCB_EVENT(XCB_BUTTON_RELEASE);
    PRINT_XCB_EVENT(XCB_MOTION_NOTIFY);
    PRINT_XCB_EVENT(XCB_ENTER_NOTIFY);
    PRINT_XCB_EVENT(XCB_LEAVE_NOTIFY);
    PRINT_XCB_EVENT(XCB_FOCUS_IN);
    PRINT_XCB_EVENT(XCB_FOCUS_OUT);
    PRINT_XCB_EVENT(XCB_KEYMAP_NOTIFY);
    PRINT_XCB_EVENT(XCB_EXPOSE);
    PRINT_XCB_EVENT(XCB_GRAPHICS_EXPOSURE);
    PRINT_XCB_EVENT(XCB_VISIBILITY_NOTIFY);
    PRINT_XCB_EVENT(XCB_CREATE_NOTIFY);
    PRINT_XCB_EVENT(XCB_DESTROY_NOTIFY);
    PRINT_XCB_EVENT(XCB_UNMAP_NOTIFY);
    PRINT_XCB_EVENT(XCB_MAP_NOTIFY);
    PRINT_XCB_EVENT(XCB_MAP_REQUEST);
    PRINT_XCB_EVENT(XCB_REPARENT_NOTIFY);
    PRINT_XCB_EVENT(XCB_CONFIGURE_NOTIFY);
    PRINT_XCB_EVENT(XCB_CONFIGURE_REQUEST);
    PRINT_XCB_EVENT(XCB_GRAVITY_NOTIFY);
    PRINT_XCB_EVENT(XCB_RESIZE_REQUEST);
    PRINT_XCB_EVENT(XCB_CIRCULATE_NOTIFY);
    PRINT_XCB_EVENT(XCB_CIRCULATE_REQUEST);
    PRINT_XCB_EVENT(XCB_PROPERTY_NOTIFY);
    PRINT_XCB_EVENT(XCB_SELECTION_CLEAR);
    PRINT_XCB_EVENT(XCB_SELECTION_REQUEST);
    PRINT_XCB_EVENT(XCB_SELECTION_NOTIFY);
    PRINT_XCB_EVENT(XCB_COLORMAP_NOTIFY);
    PRINT_XCB_EVENT(XCB_CLIENT_MESSAGE);
    PRINT_XCB_EVENT(XCB_MAPPING_NOTIFY);
    default:
        qDebug("%s: unknown event - response_type: %d - sequence: %d", message, int(event->response_type & ~0x80), int(event->sequence));
    }
#else
    Q_UNUSED(message);
    Q_UNUSED(event);
#endif
}

const char *xcb_errors[] =
{
    "Success",
    "BadRequest",
    "BadValue",
    "BadWindow",
    "BadPixmap",
    "BadAtom",
    "BadCursor",
    "BadFont",
    "BadMatch",
    "BadDrawable",
    "BadAccess",
    "BadAlloc",
    "BadColor",
    "BadGC",
    "BadIDChoice",
    "BadName",
    "BadLength",
    "BadImplementation",
    "Unknown"
};

const char *xcb_protocol_request_codes[] =
{
    "Null",
    "CreateWindow",
    "ChangeWindowAttributes",
    "GetWindowAttributes",
    "DestroyWindow",
    "DestroySubwindows",
    "ChangeSaveSet",
    "ReparentWindow",
    "MapWindow",
    "MapSubwindows",
    "UnmapWindow",
    "UnmapSubwindows",
    "ConfigureWindow",
    "CirculateWindow",
    "GetGeometry",
    "QueryTree",
    "InternAtom",
    "GetAtomName",
    "ChangeProperty",
    "DeleteProperty",
    "GetProperty",
    "ListProperties",
    "SetSelectionOwner",
    "GetSelectionOwner",
    "ConvertSelection",
    "SendEvent",
    "GrabPointer",
    "UngrabPointer",
    "GrabButton",
    "UngrabButton",
    "ChangeActivePointerGrab",
    "GrabKeyboard",
    "UngrabKeyboard",
    "GrabKey",
    "UngrabKey",
    "AllowEvents",
    "GrabServer",
    "UngrabServer",
    "QueryPointer",
    "GetMotionEvents",
    "TranslateCoords",
    "WarpPointer",
    "SetInputFocus",
    "GetInputFocus",
    "QueryKeymap",
    "OpenFont",
    "CloseFont",
    "QueryFont",
    "QueryTextExtents",
    "ListFonts",
    "ListFontsWithInfo",
    "SetFontPath",
    "GetFontPath",
    "CreatePixmap",
    "FreePixmap",
    "CreateGC",
    "ChangeGC",
    "CopyGC",
    "SetDashes",
    "SetClipRectangles",
    "FreeGC",
    "ClearArea",
    "CopyArea",
    "CopyPlane",
    "PolyPoint",
    "PolyLine",
    "PolySegment",
    "PolyRectangle",
    "PolyArc",
    "FillPoly",
    "PolyFillRectangle",
    "PolyFillArc",
    "PutImage",
    "GetImage",
    "PolyText8",
    "PolyText16",
    "ImageText8",
    "ImageText16",
    "CreateColormap",
    "FreeColormap",
    "CopyColormapAndFree",
    "InstallColormap",
    "UninstallColormap",
    "ListInstalledColormaps",
    "AllocColor",
    "AllocNamedColor",
    "AllocColorCells",
    "AllocColorPlanes",
    "FreeColors",
    "StoreColors",
    "StoreNamedColor",
    "QueryColors",
    "LookupColor",
    "CreateCursor",
    "CreateGlyphCursor",
    "FreeCursor",
    "RecolorCursor",
    "QueryBestSize",
    "QueryExtension",
    "ListExtensions",
    "ChangeKeyboardMapping",
    "GetKeyboardMapping",
    "ChangeKeyboardControl",
    "GetKeyboardControl",
    "Bell",
    "ChangePointerControl",
    "GetPointerControl",
    "SetScreenSaver",
    "GetScreenSaver",
    "ChangeHosts",
    "ListHosts",
    "SetAccessControl",
    "SetCloseDownMode",
    "KillClient",
    "RotateProperties",
    "ForceScreenSaver",
    "SetPointerMapping",
    "GetPointerMapping",
    "SetModifierMapping",
    "GetModifierMapping",
    "Unknown"
};

#ifdef Q_XCB_DEBUG
void QXcbConnection::log(const char *file, int line, int sequence)
{
    QMutexLocker locker(&m_callLogMutex);
    CallInfo info;
    info.sequence = sequence;
    info.file = file;
    info.line = line;
    m_callLog << info;
}
#endif

void QXcbConnection::handleXcbError(xcb_generic_error_t *error)
{
    uint clamped_error_code = qMin<uint>(error->error_code, (sizeof(xcb_errors) / sizeof(xcb_errors[0])) - 1);
    uint clamped_major_code = qMin<uint>(error->major_code, (sizeof(xcb_protocol_request_codes) / sizeof(xcb_protocol_request_codes[0])) - 1);

    qDebug("XCB error: %d (%s), sequence: %d, resource id: %d, major code: %d (%s), minor code: %d",
           int(error->error_code), xcb_errors[clamped_error_code],
           int(error->sequence), int(error->resource_id),
           int(error->major_code), xcb_protocol_request_codes[clamped_major_code],
           int(error->minor_code));
#ifdef Q_XCB_DEBUG
    QMutexLocker locker(&m_callLogMutex);
    int i = 0;
    for (; i < m_callLog.size(); ++i) {
        if (m_callLog.at(i).sequence == error->sequence) {
            qDebug("Caused by: %s:%d", qPrintable(m_callLog.at(i).file), m_callLog.at(i).line);
            break;
        } else if (m_callLog.at(i).sequence > error->sequence) {
            qDebug("Caused some time before: %s:%d", qPrintable(m_callLog.at(i).file), m_callLog.at(i).line);
            if (i > 0)
                qDebug("and after: %s:%d", qPrintable(m_callLog.at(i-1).file), m_callLog.at(i-1).line);
            break;
        }
    }
    if (i == m_callLog.size() && !m_callLog.isEmpty())
        qDebug("Caused some time after: %s:%d", qPrintable(m_callLog.first().file), m_callLog.first().line);
#endif
}

void QXcbConnection::handleXcbEvent(xcb_generic_event_t *event)
{
#ifdef Q_XCB_DEBUG
    {
        QMutexLocker locker(&m_callLogMutex);
        int i = 0;
        for (; i < m_callLog.size(); ++i)
            if (m_callLog.at(i).sequence >= event->sequence)
                break;
        m_callLog.remove(0, i);
    }
#endif
    bool handled = false;

    if (QPlatformNativeInterface::EventFilter filter = m_nativeInterface->eventFilterForEventType(QByteArrayLiteral("xcb_generic_event_t")))
        handled = filter(event, 0);

    uint response_type = event->response_type & ~0x80;

    if (!handled) {
        switch (response_type) {
        case XCB_EXPOSE:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_expose_event_t, window, handleExposeEvent);
        case XCB_BUTTON_PRESS:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_button_press_event_t, event, handleButtonPressEvent);
        case XCB_BUTTON_RELEASE:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_button_release_event_t, event, handleButtonReleaseEvent);
        case XCB_MOTION_NOTIFY:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_motion_notify_event_t, event, handleMotionNotifyEvent);
        case XCB_CONFIGURE_NOTIFY:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_configure_notify_event_t, event, handleConfigureNotifyEvent);
        case XCB_MAP_NOTIFY:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_map_notify_event_t, event, handleMapNotifyEvent);
        case XCB_UNMAP_NOTIFY:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_unmap_notify_event_t, event, handleUnmapNotifyEvent);
        case XCB_CLIENT_MESSAGE:
            handleClientMessageEvent((xcb_client_message_event_t *)event);
        case XCB_ENTER_NOTIFY:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_enter_notify_event_t, event, handleEnterNotifyEvent);
        case XCB_LEAVE_NOTIFY:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_leave_notify_event_t, event, handleLeaveNotifyEvent);
        case XCB_FOCUS_IN:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_focus_in_event_t, event, handleFocusInEvent);
        case XCB_FOCUS_OUT:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_focus_out_event_t, event, handleFocusOutEvent);
        case XCB_KEY_PRESS:
            HANDLE_KEYBOARD_EVENT(xcb_key_press_event_t, handleKeyPressEvent);
        case XCB_KEY_RELEASE:
            HANDLE_KEYBOARD_EVENT(xcb_key_release_event_t, handleKeyReleaseEvent);
        case XCB_MAPPING_NOTIFY:
            m_keyboard->handleMappingNotifyEvent((xcb_mapping_notify_event_t *)event);
            break;
        case XCB_SELECTION_REQUEST:
        {
            xcb_selection_request_event_t *sr = (xcb_selection_request_event_t *)event;
            if (sr->selection == atom(QXcbAtom::XdndSelection))
                m_drag->handleSelectionRequest(sr);
            else
                m_clipboard->handleSelectionRequest(sr);
            break;
        }
        case XCB_SELECTION_CLEAR:
            setTime(((xcb_selection_clear_event_t *)event)->time);
            m_clipboard->handleSelectionClearRequest((xcb_selection_clear_event_t *)event);
            handled = true;
            break;
        case XCB_SELECTION_NOTIFY:
            setTime(((xcb_selection_notify_event_t *)event)->time);
            qDebug() << "XCB_SELECTION_NOTIFY";
            handled = false;
            break;
        case XCB_PROPERTY_NOTIFY:
            HANDLE_PLATFORM_WINDOW_EVENT(xcb_property_notify_event_t, window, handlePropertyNotifyEvent);
            break;
    #ifdef XCB_USE_XINPUT2_MAEMO
        case GenericEvent:
            handleGenericEvent((xcb_ge_event_t*)event);
            break;
    #endif
        default:
            handled = false;
            break;
        }
    }

    if (!handled) {
        if (response_type == xfixes_first_event + XCB_XFIXES_SELECTION_NOTIFY) {
            setTime(((xcb_xfixes_selection_notify_event_t *)event)->timestamp);
            m_clipboard->handleXFixesSelectionRequest((xcb_xfixes_selection_notify_event_t *)event);
            handled = true;
        }
    }

#ifdef XCB_USE_XLIB
    if (!handled) {
        // Check if a custom XEvent constructor was registered in xlib for this event type, and call it discarding the constructed XEvent if any.
        // XESetWireToEvent might be used by libraries to intercept messages from the X server e.g. the OpenGL lib waiting for DRI2 events.

        Display *xdisplay = (Display *)m_xlib_display;
        XLockDisplay(xdisplay);
        Bool (*proc)(Display*, XEvent*, xEvent*) = XESetWireToEvent(xdisplay, response_type, 0);
        if (proc) {
            XESetWireToEvent(xdisplay, response_type, proc);
            XEvent dummy;
            event->sequence = LastKnownRequestProcessed(m_xlib_display);
            proc(xdisplay, &dummy, (xEvent*)event);
        }
        XUnlockDisplay(xdisplay);
    }
#endif

    if (handled)
        printXcbEvent("Handled XCB event", event);
    else
        printXcbEvent("Unhandled XCB event", event);
}

void QXcbConnection::addPeekFunc(PeekFunc f)
{
    m_peekFuncs.append(f);
}

#ifdef XCB_POLL_FOR_QUEUED_EVENT
void QXcbEventReader::run()
{
    xcb_generic_event_t *event;
    while (m_connection && (event = xcb_wait_for_event(m_connection->xcb_connection()))) {
        m_mutex.lock();
        addEvent(event);
        while (m_connection && (event = xcb_poll_for_queued_event(m_connection->xcb_connection())))
            addEvent(event);
        m_mutex.unlock();
        emit eventPending();
    }

    for (int i = 0; i < m_events.size(); ++i)
        free(m_events.at(i));
}
#endif

void QXcbEventReader::addEvent(xcb_generic_event_t *event)
{
    if ((event->response_type & ~0x80) == XCB_CLIENT_MESSAGE
        && ((xcb_client_message_event_t *)event)->type == m_connection->atom(QXcbAtom::_QT_CLOSE_CONNECTION))
        m_connection = 0;
    m_events << event;
}

QXcbEventArray *QXcbEventReader::lock()
{
    m_mutex.lock();
#ifndef XCB_POLL_FOR_QUEUED_EVENT
    while (xcb_generic_event_t *event = xcb_poll_for_event(m_connection->xcb_connection()))
        m_events << event;
#endif
    return &m_events;
}

void QXcbEventReader::unlock()
{
    m_mutex.unlock();
}

void QXcbConnection::sendConnectionEvent(QXcbAtom::Atom a, uint id)
{
    xcb_client_message_event_t event;
    memset(&event, 0, sizeof(event));

    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.sequence = 0;
    event.window = m_connectionEventListener;
    event.type = atom(a);
    event.data.data32[0] = id;

    Q_XCB_CALL(xcb_send_event(xcb_connection(), false, m_connectionEventListener, XCB_EVENT_MASK_NO_EVENT, (const char *)&event));
    xcb_flush(xcb_connection());
}

void QXcbConnection::processXcbEvents()
{
    QXcbEventArray *eventqueue = m_reader->lock();

    for(int i = 0; i < eventqueue->size(); ++i) {
        xcb_generic_event_t *event = eventqueue->at(i);
        if (!event)
            continue;
        (*eventqueue)[i] = 0;

        uint response_type = event->response_type & ~0x80;

        if (!response_type) {
            handleXcbError((xcb_generic_error_t *)event);
        } else {
            QVector<PeekFunc>::iterator it = m_peekFuncs.begin();
            while (it != m_peekFuncs.end()) {
                // These callbacks return true if the event is what they were
                // waiting for, remove them from the list in that case.
                if ((*it)(event))
                    it = m_peekFuncs.erase(it);
                else
                    ++it;
            }
            m_reader->unlock();
            handleXcbEvent(event);
            m_reader->lock();
        }

        free(event);
    }

    eventqueue->clear();

    m_reader->unlock();

    // Indicate with a null event that the event the callbacks are waiting for
    // is not in the queue currently.
    Q_FOREACH (PeekFunc f, m_peekFuncs)
        f(0);
    m_peekFuncs.clear();

    xcb_flush(xcb_connection());
}

void QXcbConnection::handleClientMessageEvent(const xcb_client_message_event_t *event)
{
    if (event->format != 32)
        return;

    if (event->type == atom(QXcbAtom::XdndStatus)) {
        drag()->handleStatus(event, false);
    } else if (event->type == atom(QXcbAtom::XdndFinished)) {
        drag()->handleFinished(event, false);
    }

    QXcbWindow *window = platformWindowFromId(event->window);
    if (!window)
        return;

    window->handleClientMessageEvent(event);
}

xcb_generic_event_t *QXcbConnection::checkEvent(int type)
{
    QXcbEventArray *eventqueue = m_reader->lock();

    for (int i = 0; i < eventqueue->size(); ++i) {
        xcb_generic_event_t *event = eventqueue->at(i);
        if (event && event->response_type == type) {
            (*eventqueue)[i] = 0;
            m_reader->unlock();
            return event;
        }
    }

    m_reader->unlock();

    return 0;
}

static const char * xcb_atomnames = {
    // window-manager <-> client protocols
    "WM_PROTOCOLS\0"
    "WM_DELETE_WINDOW\0"
    "WM_TAKE_FOCUS\0"
    "_NET_WM_PING\0"
    "_NET_WM_CONTEXT_HELP\0"
    "_NET_WM_SYNC_REQUEST\0"
    "_NET_WM_SYNC_REQUEST_COUNTER\0"

    // ICCCM window state
    "WM_STATE\0"
    "WM_CHANGE_STATE\0"

    // Session management
    "WM_CLIENT_LEADER\0"
    "WM_WINDOW_ROLE\0"
    "SM_CLIENT_ID\0"

    // Clipboard
    "CLIPBOARD\0"
    "INCR\0"
    "TARGETS\0"
    "MULTIPLE\0"
    "TIMESTAMP\0"
    "SAVE_TARGETS\0"
    "CLIP_TEMPORARY\0"
    "_QT_SELECTION\0"
    "_QT_CLIPBOARD_SENTINEL\0"
    "_QT_SELECTION_SENTINEL\0"
    "CLIPBOARD_MANAGER\0"

    "RESOURCE_MANAGER\0"

    "_XSETROOT_ID\0"

    "_QT_SCROLL_DONE\0"
    "_QT_INPUT_ENCODING\0"

    "_QT_CLOSE_CONNECTION\0"

    "_MOTIF_WM_HINTS\0"

    "DTWM_IS_RUNNING\0"
    "ENLIGHTENMENT_DESKTOP\0"
    "_DT_SAVE_MODE\0"
    "_SGI_DESKS_MANAGER\0"

    // EWMH (aka NETWM)
    "_NET_SUPPORTED\0"
    "_NET_VIRTUAL_ROOTS\0"
    "_NET_WORKAREA\0"

    "_NET_MOVERESIZE_WINDOW\0"
    "_NET_WM_MOVERESIZE\0"

    "_NET_WM_NAME\0"
    "_NET_WM_ICON_NAME\0"
    "_NET_WM_ICON\0"

    "_NET_WM_PID\0"

    "_NET_WM_WINDOW_OPACITY\0"

    "_NET_WM_STATE\0"
    "_NET_WM_STATE_ABOVE\0"
    "_NET_WM_STATE_BELOW\0"
    "_NET_WM_STATE_FULLSCREEN\0"
    "_NET_WM_STATE_MAXIMIZED_HORZ\0"
    "_NET_WM_STATE_MAXIMIZED_VERT\0"
    "_NET_WM_STATE_MODAL\0"
    "_NET_WM_STATE_STAYS_ON_TOP\0"
    "_NET_WM_STATE_DEMANDS_ATTENTION\0"

    "_NET_WM_USER_TIME\0"
    "_NET_WM_USER_TIME_WINDOW\0"
    "_NET_WM_FULL_PLACEMENT\0"

    "_NET_WM_WINDOW_TYPE\0"
    "_NET_WM_WINDOW_TYPE_DESKTOP\0"
    "_NET_WM_WINDOW_TYPE_DOCK\0"
    "_NET_WM_WINDOW_TYPE_TOOLBAR\0"
    "_NET_WM_WINDOW_TYPE_MENU\0"
    "_NET_WM_WINDOW_TYPE_UTILITY\0"
    "_NET_WM_WINDOW_TYPE_SPLASH\0"
    "_NET_WM_WINDOW_TYPE_DIALOG\0"
    "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU\0"
    "_NET_WM_WINDOW_TYPE_POPUP_MENU\0"
    "_NET_WM_WINDOW_TYPE_TOOLTIP\0"
    "_NET_WM_WINDOW_TYPE_NOTIFICATION\0"
    "_NET_WM_WINDOW_TYPE_COMBO\0"
    "_NET_WM_WINDOW_TYPE_DND\0"
    "_NET_WM_WINDOW_TYPE_NORMAL\0"
    "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE\0"

    "_KDE_NET_WM_FRAME_STRUT\0"

    "_NET_STARTUP_INFO\0"
    "_NET_STARTUP_INFO_BEGIN\0"

    "_NET_SUPPORTING_WM_CHECK\0"

    "_NET_WM_CM_S0\0"

    "_NET_SYSTEM_TRAY_VISUAL\0"

    "_NET_ACTIVE_WINDOW\0"

    // Property formats
    "TEXT\0"
    "UTF8_STRING\0"

    // xdnd
    "XdndEnter\0"
    "XdndPosition\0"
    "XdndStatus\0"
    "XdndLeave\0"
    "XdndDrop\0"
    "XdndFinished\0"
    "XdndTypeList\0"
    "XdndActionList\0"

    "XdndSelection\0"

    "XdndAware\0"
    "XdndProxy\0"

    "XdndActionCopy\0"
    "XdndActionLink\0"
    "XdndActionMove\0"
    "XdndActionPrivate\0"

    // Motif DND
    "_MOTIF_DRAG_AND_DROP_MESSAGE\0"
    "_MOTIF_DRAG_INITIATOR_INFO\0"
    "_MOTIF_DRAG_RECEIVER_INFO\0"
    "_MOTIF_DRAG_WINDOW\0"
    "_MOTIF_DRAG_TARGETS\0"

    "XmTRANSFER_SUCCESS\0"
    "XmTRANSFER_FAILURE\0"

    // Xkb
    "_XKB_RULES_NAMES\0"

    // XEMBED
    "_XEMBED\0"
    "_XEMBED_INFO\0"

    // Wacom old. (before version 0.10)
    "Wacom Stylus\0"
    "Wacom Cursor\0"
    "Wacom Eraser\0"

    // Tablet
    "STYLUS\0"
    "ERASER\0"

    // XInput2
    "Button Left\0"
    "Button Middle\0"
    "Button Right\0"
    "Button Wheel Up\0"
    "Button Wheel Down\0"
    "Button Horiz Wheel Left\0"
    "Button Horiz Wheel Right\0"
    "Abs MT Position X\0"
    "Abs MT Position Y\0"
    "Abs MT Touch Major\0"
    "Abs MT Touch Minor\0"
    "Abs MT Pressure\0"
    "Abs MT Tracking ID\0"
    "Max Contacts\0"
#if XCB_USE_MAEMO_WINDOW_PROPERTIES
    "_MEEGOTOUCH_ORIENTATION_ANGLE\0"
#endif
};

xcb_atom_t QXcbConnection::atom(QXcbAtom::Atom atom)
{
    return m_allAtoms[atom];
}

void QXcbConnection::initializeAllAtoms() {
    const char *names[QXcbAtom::NAtoms];
    const char *ptr = xcb_atomnames;

    int i = 0;
    while (*ptr) {
        names[i++] = ptr;
        while (*ptr)
            ++ptr;
        ++ptr;
    }

    Q_ASSERT(i == QXcbAtom::NPredefinedAtoms);

    QByteArray settings_atom_name("_QT_SETTINGS_TIMESTAMP_");
    settings_atom_name += m_displayName;
    names[i++] = settings_atom_name;

    xcb_intern_atom_cookie_t cookies[QXcbAtom::NAtoms];

    Q_ASSERT(i == QXcbAtom::NAtoms);
    for (i = 0; i < QXcbAtom::NAtoms; ++i)
        cookies[i] = xcb_intern_atom(xcb_connection(), false, strlen(names[i]), names[i]);

    for (i = 0; i < QXcbAtom::NAtoms; ++i) {
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(xcb_connection(), cookies[i], 0);
        m_allAtoms[i] = reply->atom;
        free(reply);
    }
}

xcb_atom_t QXcbConnection::internAtom(const char *name)
{
    if (!name || *name == 0)
        return XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(xcb_connection(), false, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(xcb_connection(), cookie, 0);
    int atom = reply->atom;
    free(reply);
    return atom;
}

QByteArray QXcbConnection::atomName(xcb_atom_t atom)
{
    if (!atom)
        return QByteArray();

    xcb_generic_error_t *error = 0;
    xcb_get_atom_name_cookie_t cookie = Q_XCB_CALL(xcb_get_atom_name(xcb_connection(), atom));
    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(xcb_connection(), cookie, &error);
    if (error) {
        qWarning() << "QXcbConnection::atomName: bad Atom" << atom;
        free(error);
    }
    if (reply) {
        QByteArray result(xcb_get_atom_name_name(reply), xcb_get_atom_name_name_length(reply));
        free(reply);
        return result;
    }
    return QByteArray();
}

const xcb_format_t *QXcbConnection::formatForDepth(uint8_t depth) const
{
    xcb_format_iterator_t iterator =
        xcb_setup_pixmap_formats_iterator(m_setup);

    while (iterator.rem) {
        xcb_format_t *format = iterator.data;
        if (format->depth == depth)
            return format;
        xcb_format_next(&iterator);
    }

    return 0;
}

void QXcbConnection::sync()
{
    // from xcb_aux_sync
    xcb_get_input_focus_cookie_t cookie = Q_XCB_CALL(xcb_get_input_focus(xcb_connection()));
    free(xcb_get_input_focus_reply(xcb_connection(), cookie, 0));
}

void QXcbConnection::initializeXFixes()
{
    xcb_generic_error_t *error = 0;
    const xcb_query_extension_reply_t *reply = xcb_get_extension_data(m_connection, &xcb_xfixes_id);
    xfixes_first_event = reply->first_event;

    xcb_xfixes_query_version_cookie_t xfixes_query_cookie = xcb_xfixes_query_version(m_connection,
                                                                                     XCB_XFIXES_MAJOR_VERSION,
                                                                                     XCB_XFIXES_MINOR_VERSION);
    xcb_xfixes_query_version_reply_t *xfixes_query = xcb_xfixes_query_version_reply (m_connection,
                                                                                     xfixes_query_cookie, &error);
    if (!xfixes_query || error || xfixes_query->major_version < 2) {
        qWarning("Failed to initialize XFixes");
        free(error);
        xfixes_first_event = 0;
    }
    free(xfixes_query);
}

void QXcbConnection::initializeXRender()
{
#ifdef XCB_USE_RENDER
    xcb_generic_error_t *error = 0;
    xcb_render_query_version_cookie_t xrender_query_cookie = xcb_render_query_version(m_connection,
                                                                                      XCB_RENDER_MAJOR_VERSION,
                                                                                      XCB_RENDER_MINOR_VERSION);
    xcb_render_query_version_reply_t *xrender_query = xcb_render_query_version_reply(m_connection,
                                                                                     xrender_query_cookie, &error);
    if (!xrender_query || error || (xrender_query->major_version == 0 && xrender_query->minor_version < 5)) {
        qWarning("Failed to initialize XRender");
        free(error);
    }
    free(xrender_query);
#endif
}

#if defined(XCB_USE_EGL)
bool QXcbConnection::hasEgl() const
{
    return m_has_egl;
}
#endif // defined(XCB_USE_EGL)

#ifdef XCB_USE_DRI2
void QXcbConnection::initializeDri2()
{
    xcb_dri2_connect_cookie_t connect_cookie = xcb_dri2_connect_unchecked (m_connection,
                                                                           m_screens[0]->root(),
                                                                           XCB_DRI2_DRIVER_TYPE_DRI);

    xcb_dri2_connect_reply_t *connect = xcb_dri2_connect_reply (m_connection,
                                                                connect_cookie, NULL);

    if (! connect || connect->driver_name_length + connect->device_name_length == 0) {
        qDebug() << "Failed to connect to dri2";
        return;
    }

    m_dri2_device_name = QByteArray(xcb_dri2_connect_device_name (connect),
                                                    xcb_dri2_connect_device_name_length (connect));
    delete connect;

    int fd = open(m_dri2_device_name.constData(), O_RDWR);
    if (fd < 0) {
        qDebug() << "InitializeDri2: Could'nt open device << dri2DeviceName";
        m_dri2_device_name = QByteArray();
        return;
    }

    drm_magic_t magic;
    if (drmGetMagic(fd, &magic)) {
        qDebug() << "Failed to get drmMagic";
        return;
    }

    xcb_dri2_authenticate_cookie_t authenticate_cookie = xcb_dri2_authenticate_unchecked(m_connection,
                                                                                         m_screens[0]->root(), magic);
    xcb_dri2_authenticate_reply_t *authenticate = xcb_dri2_authenticate_reply(m_connection,
                                                                              authenticate_cookie, NULL);
    if (authenticate == NULL || !authenticate->authenticated) {
        qWarning("DRI2: failed to authenticate");
        free(authenticate);
        return;
    }

    delete authenticate;

    EGLDisplay display = eglGetDRMDisplayMESA(fd);
    if (!display) {
        qWarning("failed to create display");
        return;
    }

    m_egl_display = display;
    EGLint major,minor;
    if (!eglInitialize(display, &major, &minor)) {
        qWarning("failed to initialize display");
        return;
    }
}

bool QXcbConnection::hasSupportForDri2() const
{
    if (!m_dri2_support_probed) {
        xcb_generic_error_t *error = 0;

        xcb_prefetch_extension_data (m_connection, &xcb_dri2_id);

        xcb_dri2_query_version_cookie_t dri2_query_cookie = xcb_dri2_query_version (m_connection,
                                                                                    XCB_DRI2_MAJOR_VERSION,
                                                                                    XCB_DRI2_MINOR_VERSION);

        xcb_dri2_query_version_reply_t *dri2_query = xcb_dri2_query_version_reply (m_connection,
                                                                                   dri2_query_cookie, &error);
        if (!dri2_query || error) {
            delete error;
            delete dri2_query;
            return false;
        }

        QXcbConnection *that = const_cast<QXcbConnection *>(this);
        that->m_dri2_major = dri2_query->major_version;
        that->m_dri2_minor = dri2_query->minor_version;

        that->m_has_support_for_dri2 = true;
        that->m_dri2_support_probed = true;
    }
    return m_has_support_for_dri2;
}
#endif //XCB_USE_DRI2

QT_END_NAMESPACE
