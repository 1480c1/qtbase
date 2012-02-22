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

#ifndef QGLOBAL_H
#define QGLOBAL_H

#include <stddef.h>

#define QT_VERSION_STR   "5.0.0"
/*
   QT_VERSION is (major << 16) + (minor << 8) + patch.
*/
#define QT_VERSION 0x050000
/*
   can be used like #if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
*/
#define QT_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

#define QT_PACKAGEDATE_STR "YYYY-MM-DD"

#define QT_PACKAGE_TAG ""

#if !defined(QT_BUILD_MOC)
#include <QtCore/qconfig.h>
#endif


#include <QtCore/qsystemdetection.h>
#include <QtCore/qcompilerdetection.h>
#include <QtCore/qprocessordetection.h>

#ifdef __cplusplus

#ifndef QT_NO_STL
#include <algorithm>
#endif

#ifndef QT_NAMESPACE /* user namespace */

# define QT_PREPEND_NAMESPACE(name) ::name
# define QT_USE_NAMESPACE
# define QT_BEGIN_NAMESPACE
# define QT_END_NAMESPACE
# define QT_BEGIN_INCLUDE_NAMESPACE
# define QT_END_INCLUDE_NAMESPACE
#ifndef QT_BEGIN_MOC_NAMESPACE
# define QT_BEGIN_MOC_NAMESPACE
#endif
#ifndef QT_END_MOC_NAMESPACE
# define QT_END_MOC_NAMESPACE
#endif
# define QT_FORWARD_DECLARE_CLASS(name) class name;
# define QT_FORWARD_DECLARE_STRUCT(name) struct name;
# define QT_MANGLE_NAMESPACE(name) name

#else /* user namespace */

# define QT_PREPEND_NAMESPACE(name) ::QT_NAMESPACE::name
# define QT_USE_NAMESPACE using namespace ::QT_NAMESPACE;
# define QT_BEGIN_NAMESPACE namespace QT_NAMESPACE {
# define QT_END_NAMESPACE }
# define QT_BEGIN_INCLUDE_NAMESPACE }
# define QT_END_INCLUDE_NAMESPACE namespace QT_NAMESPACE {
#ifndef QT_BEGIN_MOC_NAMESPACE
# define QT_BEGIN_MOC_NAMESPACE QT_USE_NAMESPACE
#endif
#ifndef QT_END_MOC_NAMESPACE
# define QT_END_MOC_NAMESPACE
#endif
# define QT_FORWARD_DECLARE_CLASS(name) \
    QT_BEGIN_NAMESPACE class name; QT_END_NAMESPACE \
    using QT_PREPEND_NAMESPACE(name);

# define QT_FORWARD_DECLARE_STRUCT(name) \
    QT_BEGIN_NAMESPACE struct name; QT_END_NAMESPACE \
    using QT_PREPEND_NAMESPACE(name);

# define QT_MANGLE_NAMESPACE0(x) x
# define QT_MANGLE_NAMESPACE1(a, b) a##_##b
# define QT_MANGLE_NAMESPACE2(a, b) QT_MANGLE_NAMESPACE1(a,b)
# define QT_MANGLE_NAMESPACE(name) QT_MANGLE_NAMESPACE2( \
        QT_MANGLE_NAMESPACE0(name), QT_MANGLE_NAMESPACE0(QT_NAMESPACE))

namespace QT_NAMESPACE {}

# ifndef QT_BOOTSTRAPPED
# ifndef QT_NO_USING_NAMESPACE
   /*
    This expands to a "using QT_NAMESPACE" also in _header files_.
    It is the only way the feature can be used without too much
    pain, but if people _really_ do not want it they can add
    DEFINES += QT_NO_USING_NAMESPACE to their .pro files.
    */
   QT_USE_NAMESPACE
# endif
# endif

#endif /* user namespace */

#else /* __cplusplus */

# define QT_BEGIN_NAMESPACE
# define QT_END_NAMESPACE
# define QT_USE_NAMESPACE
# define QT_BEGIN_INCLUDE_NAMESPACE
# define QT_END_INCLUDE_NAMESPACE

#endif /* __cplusplus */

#define QT_BEGIN_HEADER
#define QT_END_HEADER

#if defined(Q_OS_DARWIN) && !defined(QT_LARGEFILE_SUPPORT)
#  define QT_LARGEFILE_SUPPORT 64
#endif

#ifndef Q_PACKED
#  define Q_PACKED
#  undef Q_NO_PACKED_REFERENCE
#endif

#ifndef Q_LIKELY
#  define Q_LIKELY(x) (x)
#endif
#ifndef Q_UNLIKELY
#  define Q_UNLIKELY(x) (x)
#endif

#ifndef Q_ALLOC_SIZE
#  define Q_ALLOC_SIZE(x)
#endif

#ifndef Q_CONSTRUCTOR_FUNCTION
# define Q_CONSTRUCTOR_FUNCTION0(AFUNC) \
    namespace { \
    static const struct AFUNC ## _ctor_class_ { \
        inline AFUNC ## _ctor_class_() { AFUNC(); } \
    } AFUNC ## _ctor_instance_; \
    }

# define Q_CONSTRUCTOR_FUNCTION(AFUNC) Q_CONSTRUCTOR_FUNCTION0(AFUNC)
#endif

#ifndef Q_DESTRUCTOR_FUNCTION
# define Q_DESTRUCTOR_FUNCTION0(AFUNC) \
    namespace { \
    static const struct AFUNC ## _dtor_class_ { \
        inline AFUNC ## _dtor_class_() { } \
        inline ~ AFUNC ## _dtor_class_() { AFUNC(); } \
    } AFUNC ## _dtor_instance_; \
    }
# define Q_DESTRUCTOR_FUNCTION(AFUNC) Q_DESTRUCTOR_FUNCTION0(AFUNC)
#endif

#ifndef Q_REQUIRED_RESULT
#  if defined(Q_CC_GNU) && !defined(Q_CC_INTEL) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#    define Q_REQUIRED_RESULT __attribute__ ((warn_unused_result))
#  else
#    define Q_REQUIRED_RESULT
#  endif
#endif

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

/*
   Size-dependent types (architechture-dependent byte order)

   Make sure to update QMetaType when changing these typedefs
*/

typedef signed char qint8;         /* 8 bit signed */
typedef unsigned char quint8;      /* 8 bit unsigned */
typedef short qint16;              /* 16 bit signed */
typedef unsigned short quint16;    /* 16 bit unsigned */
typedef int qint32;                /* 32 bit signed */
typedef unsigned int quint32;      /* 32 bit unsigned */
#if defined(Q_OS_WIN) && !defined(Q_CC_GNU)
#  define Q_INT64_C(c) c ## i64    /* signed 64 bit constant */
#  define Q_UINT64_C(c) c ## ui64   /* unsigned 64 bit constant */
typedef __int64 qint64;            /* 64 bit signed */
typedef unsigned __int64 quint64;  /* 64 bit unsigned */
#else
#  define Q_INT64_C(c) static_cast<long long>(c ## LL)     /* signed 64 bit constant */
#  define Q_UINT64_C(c) static_cast<unsigned long long>(c ## ULL) /* unsigned 64 bit constant */
typedef long long qint64;           /* 64 bit signed */
typedef unsigned long long quint64; /* 64 bit unsigned */
#endif

typedef qint64 qlonglong;
typedef quint64 qulonglong;

#ifndef QT_POINTER_SIZE
#  if defined(Q_OS_WIN64)
#   define QT_POINTER_SIZE 8
#  elif defined(Q_OS_WIN32) || defined(Q_OS_WINCE)
#   define QT_POINTER_SIZE 4
#  endif
#endif

#define Q_INIT_RESOURCE_EXTERN(name) \
    extern int QT_MANGLE_NAMESPACE(qInitResources_ ## name) ();

#define Q_INIT_RESOURCE(name) \
    do { extern int QT_MANGLE_NAMESPACE(qInitResources_ ## name) ();       \
        QT_MANGLE_NAMESPACE(qInitResources_ ## name) (); } while (0)
#define Q_CLEANUP_RESOURCE(name) \
    do { extern int QT_MANGLE_NAMESPACE(qCleanupResources_ ## name) ();    \
        QT_MANGLE_NAMESPACE(qCleanupResources_ ## name) (); } while (0)

#if defined(__cplusplus)

/*
  quintptr and qptrdiff is guaranteed to be the same size as a pointer, i.e.

      sizeof(void *) == sizeof(quintptr)
      && sizeof(void *) == sizeof(qptrdiff)
*/
template <int> struct QIntegerForSize;
template <>    struct QIntegerForSize<1> { typedef quint8  Unsigned; typedef qint8  Signed; };
template <>    struct QIntegerForSize<2> { typedef quint16 Unsigned; typedef qint16 Signed; };
template <>    struct QIntegerForSize<4> { typedef quint32 Unsigned; typedef qint32 Signed; };
template <>    struct QIntegerForSize<8> { typedef quint64 Unsigned; typedef qint64 Signed; };
template <class T> struct QIntegerForSizeof: QIntegerForSize<sizeof(T)> { };
typedef QIntegerForSizeof<void*>::Unsigned quintptr;
typedef QIntegerForSizeof<void*>::Signed qptrdiff;
typedef qptrdiff qintptr;

/*
   Useful type definitions for Qt
*/

QT_BEGIN_INCLUDE_NAMESPACE
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
QT_END_INCLUDE_NAMESPACE

/*
   Constant bool values
*/

#ifndef QT_LINUXBASE /* the LSB defines TRUE and FALSE for us */
#  ifndef TRUE
#   define TRUE true
#   define FALSE false
#  endif
#endif

/*
   Proper for-scoping in MIPSpro CC
*/
#ifndef QT_NO_KEYWORDS
#  if defined(Q_CC_MIPS) || (defined(Q_CC_HPACC) && defined(__ia64))
#    define for if(0){}else for
#  endif
#endif

/*
   Workaround for static const members on MSVC++.
*/

#if defined(Q_CC_MSVC)
#  define QT_STATIC_CONST static
#  define QT_STATIC_CONST_IMPL
#else
#  define QT_STATIC_CONST static const
#  define QT_STATIC_CONST_IMPL const
#endif

/*
   Warnings and errors when using deprecated methods
*/
#if defined(Q_MOC_RUN)
#  define Q_DECL_DEPRECATED Q_DECL_DEPRECATED
#elif (defined(Q_CC_GNU) && !defined(Q_CC_INTEL) && (__GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2))) || defined(Q_CC_RVCT)
#  define Q_DECL_DEPRECATED __attribute__ ((__deprecated__))
#elif defined(Q_CC_MSVC)
#  define Q_DECL_DEPRECATED __declspec(deprecated)
#  if defined (Q_CC_INTEL)
#    define Q_DECL_VARIABLE_DEPRECATED
#  else
#  endif
#else
#  define Q_DECL_DEPRECATED
#endif
#ifndef Q_DECL_VARIABLE_DEPRECATED
#  define Q_DECL_VARIABLE_DEPRECATED Q_DECL_DEPRECATED
#endif
#ifndef Q_DECL_CONSTRUCTOR_DEPRECATED
#  if defined(Q_MOC_RUN)
#    define Q_DECL_CONSTRUCTOR_DEPRECATED Q_DECL_CONSTRUCTOR_DEPRECATED
#  elif defined(Q_NO_DEPRECATED_CONSTRUCTORS)
#    define Q_DECL_CONSTRUCTOR_DEPRECATED
#  else
#    define Q_DECL_CONSTRUCTOR_DEPRECATED Q_DECL_DEPRECATED
#  endif
#endif

#if defined(QT_NO_DEPRECATED)
#  undef QT_DEPRECATED
#  undef QT_DEPRECATED_VARIABLE
#  undef QT_DEPRECATED_CONSTRUCTOR
#elif defined(QT_DEPRECATED_WARNINGS)
#  undef QT_DEPRECATED
#  define QT_DEPRECATED Q_DECL_DEPRECATED
#  undef QT_DEPRECATED_VARIABLE
#  define QT_DEPRECATED_VARIABLE Q_DECL_VARIABLE_DEPRECATED
#  undef QT_DEPRECATED_CONSTRUCTOR
#  define QT_DEPRECATED_CONSTRUCTOR explicit Q_DECL_CONSTRUCTOR_DEPRECATED
#else
#  undef QT_DEPRECATED
#  define QT_DEPRECATED
#  undef QT_DEPRECATED_VARIABLE
#  define QT_DEPRECATED_VARIABLE
#  undef QT_DEPRECATED_CONSTRUCTOR
#  define QT_DEPRECATED_CONSTRUCTOR
#endif

#ifndef QT_DISABLE_DEPRECATED_BEFORE
// ### Qt5: remember to change that to 5 when we reach feature freeze
#define QT_DISABLE_DEPRECATED_BEFORE QT_VERSION_CHECK(4, 9, 0)
#endif

/*
    QT_DEPRECATED_SINCE(major, minor) evaluates as true if the Qt version is greater than
    the deprecation point specified.

    Use it to specify from which version of Qt a function or class has been deprecated

    Example:
        #if QT_DEPRECATED_SINCE(5,1)
            QT_DEPRECATED void deprecatedFunction(); //function deprecated since Qt 5.1
        #endif

*/
#ifdef QT_DEPRECATED
#define QT_DEPRECATED_SINCE(major, minor) (QT_VERSION_CHECK(major, minor, 0) > QT_DISABLE_DEPRECATED_BEFORE)
#else
#define QT_DEPRECATED_SINCE(major, minor) 0
#endif

/* moc compats (signals/slots) */
#ifndef QT_MOC_COMPAT
#  define QT_MOC_COMPAT
#else
#  undef QT_MOC_COMPAT
#  define QT_MOC_COMPAT
#endif

#ifdef QT_ASCII_CAST_WARNINGS
#  define QT_ASCII_CAST_WARN Q_DECL_DEPRECATED
#  if defined(Q_CC_GNU) && __GNUC__ < 4
     /* gcc < 4 doesn't like Q_DECL_DEPRECATED in front of constructors */
#    define QT_ASCII_CAST_WARN_CONSTRUCTOR
#  else
#    define QT_ASCII_CAST_WARN_CONSTRUCTOR Q_DECL_CONSTRUCTOR_DEPRECATED
#  endif
#else
#  define QT_ASCII_CAST_WARN
#  define QT_ASCII_CAST_WARN_CONSTRUCTOR
#endif

#if defined(__i386__) || defined(_WIN32) || defined(_WIN32_WCE)
#  if defined(Q_CC_GNU)
#if !defined(Q_CC_INTEL) && ((100*(__GNUC__ - 0) + 10*(__GNUC_MINOR__ - 0) + __GNUC_PATCHLEVEL__) >= 332)
#    define QT_FASTCALL __attribute__((regparm(3)))
#else
#    define QT_FASTCALL
#endif
#  elif defined(Q_CC_MSVC)
#    define QT_FASTCALL __fastcall
#  else
#     define QT_FASTCALL
#  endif
#else
#  define QT_FASTCALL
#endif

#ifdef Q_COMPILER_NULLPTR
# define Q_NULLPTR         nullptr
#else
# define Q_NULLPTR         0
#endif

#ifdef Q_COMPILER_DEFAULT_DELETE_MEMBERS
# define Q_DECL_EQ_DELETE = delete
#else
# define Q_DECL_EQ_DELETE
#endif

#ifdef Q_COMPILER_CONSTEXPR
# define Q_DECL_CONSTEXPR constexpr
#else
# define Q_DECL_CONSTEXPR
#endif

#ifdef Q_COMPILER_EXPLICIT_OVERRIDES
# define Q_DECL_OVERRIDE override
# define Q_DECL_FINAL final
#else
# define Q_DECL_OVERRIDE
# define Q_DECL_FINAL
#endif

//defines the type for the WNDPROC on windows
//the alignment needs to be forced for sse2 to not crash with mingw
#if defined(Q_OS_WIN)
#  if defined(Q_CC_MINGW)
#    define QT_ENSURE_STACK_ALIGNED_FOR_SSE __attribute__ ((force_align_arg_pointer))
#  else
#    define QT_ENSURE_STACK_ALIGNED_FOR_SSE
#  endif
#  define QT_WIN_CALLBACK CALLBACK QT_ENSURE_STACK_ALIGNED_FOR_SSE 
#endif

typedef int QNoImplicitBoolCast;

// This logic must match the one in qmetatype.h
#if defined(QT_COORD_TYPE)
typedef QT_COORD_TYPE qreal;
#elif defined(QT_NO_FPU) || defined(Q_PROCESSOR_ARM) || defined(Q_OS_WINCE)
typedef float qreal;
#else
typedef double qreal;
#endif

/*
   Utility macros and inline functions
*/

template <typename T>
Q_DECL_CONSTEXPR inline T qAbs(const T &t) { return t >= 0 ? t : -t; }

Q_DECL_CONSTEXPR inline int qRound(double d)
{ return d >= 0.0 ? int(d + 0.5) : int(d - int(d-1) + 0.5) + int(d-1); }
Q_DECL_CONSTEXPR inline int qRound(float d)
{ return d >= 0.0f ? int(d + 0.5f) : int(d - int(d-1) + 0.5f) + int(d-1); }
#ifdef Q_QDOC
/*
    Just for documentation generation
*/
int qRound(qreal d);
#endif

Q_DECL_CONSTEXPR inline qint64 qRound64(double d)
{ return d >= 0.0 ? qint64(d + 0.5) : qint64(d - double(qint64(d-1)) + 0.5) + qint64(d-1); }
Q_DECL_CONSTEXPR inline qint64 qRound64(float d)
{ return d >= 0.0f ? qint64(d + 0.5f) : qint64(d - float(qint64(d-1)) + 0.5f) + qint64(d-1); }
#ifdef Q_QDOC
/*
    Just for documentation generation
*/
qint64 qRound64(qreal d);
#endif

template <typename T>
Q_DECL_CONSTEXPR inline const T &qMin(const T &a, const T &b) { return (a < b) ? a : b; }
template <typename T>
Q_DECL_CONSTEXPR inline const T &qMax(const T &a, const T &b) { return (a < b) ? b : a; }
template <typename T>
Q_DECL_CONSTEXPR inline const T &qBound(const T &min, const T &val, const T &max)
{ return qMax(min, qMin(max, val)); }

/*
   Data stream functions are provided by many classes (defined in qdatastream.h)
*/

class QDataStream;

#if !defined(QT_NO_COP)
#  define QT_NO_COP
#endif

#if defined(Q_OS_VXWORKS)
#  define QT_NO_CRASHHANDLER     // no popen
#  define QT_NO_PROCESS          // no exec*, no fork
#  define QT_NO_LPR
#  define QT_NO_SHAREDMEMORY     // only POSIX, no SysV and in the end...
#  define QT_NO_SYSTEMSEMAPHORE  // not needed at all in a flat address space
#  define QT_NO_QWS_MULTIPROCESS // no processes
#endif

# include <QtCore/qfeatures.h>

#define QT_SUPPORTS(FEATURE) (!defined(QT_NO_##FEATURE))

#if defined(Q_OS_LINUX) && defined(Q_CC_RVCT)
#  define Q_DECL_EXPORT     __attribute__((visibility("default")))
#  define Q_DECL_IMPORT     __attribute__((visibility("default")))
#  define Q_DECL_HIDDEN     __attribute__((visibility("hidden")))
#endif

#ifndef Q_DECL_EXPORT
#  if defined(Q_OS_WIN) || defined(Q_CC_RVCT)
#    define Q_DECL_EXPORT __declspec(dllexport)
#  elif defined(QT_VISIBILITY_AVAILABLE)
#    define Q_DECL_EXPORT __attribute__((visibility("default")))
#    define Q_DECL_HIDDEN __attribute__((visibility("hidden")))
#  endif
#  ifndef Q_DECL_EXPORT
#    define Q_DECL_EXPORT
#  endif
#endif
#ifndef Q_DECL_IMPORT
#  if defined(Q_OS_WIN) || defined(Q_CC_RVCT)
#    define Q_DECL_IMPORT __declspec(dllimport)
#  else
#    define Q_DECL_IMPORT
#  endif
#endif
#ifndef Q_DECL_HIDDEN
#  define Q_DECL_HIDDEN
#endif


/*
   Create Qt DLL if QT_DLL is defined (Windows only)
*/

#if defined(Q_OS_WIN)
#  if defined(QT_NODLL)
#    undef QT_MAKEDLL
#    undef QT_DLL
#  elif defined(QT_MAKEDLL)        /* create a Qt DLL library */
#    if defined(QT_DLL)
#      undef QT_DLL
#    endif
#    if defined(QT_BUILD_CORE_LIB)
#      define Q_CORE_EXPORT Q_DECL_EXPORT
#    else
#      define Q_CORE_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_GUI_LIB)
#      define Q_GUI_EXPORT Q_DECL_EXPORT
#    else
#      define Q_GUI_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_WIDGETS_LIB)
#      define Q_WIDGETS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_WIDGETS_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_PLATFORMSUPPORT_LIB)
#      define Q_PLATFORMSUPPORT_EXPORT Q_DECL_EXPORT
#    else
#      define Q_PLATFORMSUPPORT_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_PRINTSUPPORT_LIB)
#      define Q_PRINTSUPPORT_EXPORT Q_DECL_EXPORT
#    else
#      define Q_PRINTSUPPORT_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_SQL_LIB)
#      define Q_SQL_EXPORT Q_DECL_EXPORT
#    else
#      define Q_SQL_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_NETWORK_LIB)
#      define Q_NETWORK_EXPORT Q_DECL_EXPORT
#    else
#      define Q_NETWORK_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_SVG_LIB)
#      define Q_SVG_EXPORT Q_DECL_EXPORT
#    else
#      define Q_SVG_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_QTQUICK1_LIB)
#      define Q_QTQUICK1_EXPORT Q_DECL_EXPORT
#    else
#      define Q_QTQUICK1_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_DECLARATIVE_LIB)
#      define Q_DECLARATIVE_EXPORT Q_DECL_EXPORT
#    else
#      define Q_DECLARATIVE_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_OPENGL_LIB)
#      define Q_OPENGL_EXPORT Q_DECL_EXPORT
#    else
#      define Q_OPENGL_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_MULTIMEDIA_LIB)
#      define Q_MULTIMEDIA_EXPORT Q_DECL_EXPORT
#    else
#      define Q_MULTIMEDIA_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_OPENVG_LIB)
#      define Q_OPENVG_EXPORT Q_DECL_EXPORT
#    else
#      define Q_OPENVG_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_XML_LIB)
#      define Q_XML_EXPORT Q_DECL_EXPORT
#    else
#      define Q_XML_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_XMLPATTERNS_LIB)
#      define Q_XMLPATTERNS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_XMLPATTERNS_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_SCRIPT_LIB)
#      define Q_SCRIPT_EXPORT Q_DECL_EXPORT
#    else
#      define Q_SCRIPT_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_SCRIPTTOOLS_LIB)
#      define Q_SCRIPTTOOLS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_SCRIPTTOOLS_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_CANVAS_LIB)
#      define Q_CANVAS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_CANVAS_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_COMPAT_LIB)
#      define Q_COMPAT_EXPORT Q_DECL_EXPORT
#    else
#      define Q_COMPAT_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_DBUS_LIB)
#      define Q_DBUS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_DBUS_EXPORT Q_DECL_IMPORT
#    endif
#    define Q_TEMPLATEDLL
#  elif defined(QT_DLL) /* use a Qt DLL library */
#    define Q_CORE_EXPORT Q_DECL_IMPORT
#    define Q_GUI_EXPORT Q_DECL_IMPORT
#    define Q_WIDGETS_EXPORT Q_DECL_IMPORT
#    define Q_PLATFORMSUPPORT_EXPORT Q_DECL_IMPORT
#    define Q_PRINTSUPPORT_EXPORT Q_DECL_IMPORT
#    define Q_SQL_EXPORT Q_DECL_IMPORT
#    define Q_NETWORK_EXPORT Q_DECL_IMPORT
#    define Q_SVG_EXPORT Q_DECL_IMPORT
#    define Q_DECLARATIVE_EXPORT Q_DECL_IMPORT
#    define Q_QTQUICK1_EXPORT Q_DECL_IMPORT
#    define Q_CANVAS_EXPORT Q_DECL_IMPORT
#    define Q_OPENGL_EXPORT Q_DECL_IMPORT
#    define Q_MULTIMEDIA_EXPORT Q_DECL_IMPORT
#    define Q_OPENVG_EXPORT Q_DECL_IMPORT
#    define Q_XML_EXPORT Q_DECL_IMPORT
#    define Q_XMLPATTERNS_EXPORT Q_DECL_IMPORT
#    define Q_SCRIPT_EXPORT Q_DECL_IMPORT
#    define Q_SCRIPTTOOLS_EXPORT Q_DECL_IMPORT
#    define Q_COMPAT_EXPORT Q_DECL_IMPORT
#    define Q_DBUS_EXPORT Q_DECL_IMPORT
#    define Q_TEMPLATEDLL
#  endif
#  define Q_NO_DECLARED_NOT_DEFINED
#else
#  if defined(Q_OS_LINUX) && defined(Q_CC_BOR)
#    define Q_TEMPLATEDLL
#    define Q_NO_DECLARED_NOT_DEFINED
#  endif
#  undef QT_MAKEDLL /* ignore these for other platforms */
#  undef QT_DLL
#endif

#if !defined(Q_CORE_EXPORT)
#  if defined(QT_SHARED)
#    define Q_CORE_EXPORT Q_DECL_EXPORT
#    define Q_GUI_EXPORT Q_DECL_EXPORT
#    define Q_WIDGETS_EXPORT Q_DECL_EXPORT
#    define Q_PLATFORMSUPPORT_EXPORT Q_DECL_EXPORT
#    define Q_PRINTSUPPORT_EXPORT Q_DECL_EXPORT
#    define Q_SQL_EXPORT Q_DECL_EXPORT
#    define Q_NETWORK_EXPORT Q_DECL_EXPORT
#    define Q_SVG_EXPORT Q_DECL_EXPORT
#    define Q_DECLARATIVE_EXPORT Q_DECL_EXPORT
#    define Q_QTQUICK1_EXPORT Q_DECL_EXPORT
#    define Q_OPENGL_EXPORT Q_DECL_EXPORT
#    define Q_MULTIMEDIA_EXPORT Q_DECL_EXPORT
#    define Q_OPENVG_EXPORT Q_DECL_EXPORT
#    define Q_XML_EXPORT Q_DECL_EXPORT
#    define Q_XMLPATTERNS_EXPORT Q_DECL_EXPORT
#    define Q_SCRIPT_EXPORT Q_DECL_EXPORT
#    define Q_SCRIPTTOOLS_EXPORT Q_DECL_EXPORT
#    define Q_COMPAT_EXPORT Q_DECL_EXPORT
#    define Q_DBUS_EXPORT Q_DECL_EXPORT
#  else
#    define Q_CORE_EXPORT
#    define Q_GUI_EXPORT
#    define Q_WIDGETS_EXPORT
#    define Q_PLATFORMSUPPORT_EXPORT
#    define Q_PRINTSUPPORT_EXPORT
#    define Q_SQL_EXPORT
#    define Q_NETWORK_EXPORT
#    define Q_SVG_EXPORT
#    define Q_DECLARATIVE_EXPORT
#    define Q_QTQUICK1_EXPORT
#    define Q_OPENGL_EXPORT
#    define Q_MULTIMEDIA_EXPORT
#    define Q_OPENVG_EXPORT
#    define Q_XML_EXPORT
#    define Q_XMLPATTERNS_EXPORT
#    define Q_SCRIPT_EXPORT
#    define Q_SCRIPTTOOLS_EXPORT
#    define Q_COMPAT_EXPORT
#    define Q_DBUS_EXPORT
#  endif
#endif

// Functions marked as Q_GUI_EXPORT_INLINE were exported and inlined by mistake.
// Compilers like MinGW complain that the import attribute is ignored.
#if defined(Q_CC_MINGW)
#    if defined(QT_BUILD_CORE_LIB)
#      define Q_CORE_EXPORT_INLINE Q_CORE_EXPORT inline
#    else
#      define Q_CORE_EXPORT_INLINE inline
#    endif
#    if defined(QT_BUILD_GUI_LIB)
#      define Q_GUI_EXPORT_INLINE Q_GUI_EXPORT inline
#    else
#      define Q_GUI_EXPORT_INLINE inline
#    endif
#    if defined(QT_BUILD_WIDGETS_LIB)
#      define Q_WIDGETS_EXPORT_INLINE Q_WIDGETS_EXPORT inline
#    else
#      define Q_WIDGETS_EXPORT_INLINE inline
#    endif
#    if defined(QT_BUILD_PLATFORMSUPPORT_LIB)
#      define Q_PLATFORMSUPPORT_EXPORT_INLINE Q_PLATFORMSUPPORT_EXPORT inline
#    else
#      define Q_PLATFORMSUPPORT_EXPORT_INLINE inline
#    endif
#    if defined(QT_BUILD_PRINTSUPPORT_LIB)
#      define Q_PRINTSUPPORT_EXPORT_INLINE Q_PRINTSUPPORT_EXPORT inline
#    else
#      define Q_PRINTSUPPORT_EXPORT_INLINE inline
#    endif
#    if defined(QT_BUILD_COMPAT_LIB)
#      define Q_COMPAT_EXPORT_INLINE Q_COMPAT_EXPORT inline
#    else
#      define Q_COMPAT_EXPORT_INLINE inline
#    endif
#elif defined(Q_CC_RVCT)
// we force RVCT not to export inlines by passing --visibility_inlines_hidden
// so we need to just inline it, rather than exporting and inlining
// note: this affects the contents of the DEF files (ie. these functions do not appear)
#    define Q_CORE_EXPORT_INLINE inline
#    define Q_GUI_EXPORT_INLINE inline
#    define Q_WIDGETS_EXPORT_INLINE inline
#    define Q_PLATFORMSUPPORT_EXPORT_INLINE inline
#    define Q_PRINTSUPPORT_EXPORT_INLINE inline
#    define Q_COMPAT_EXPORT_INLINE inline
#else
#    define Q_CORE_EXPORT_INLINE Q_CORE_EXPORT inline
#    define Q_GUI_EXPORT_INLINE Q_GUI_EXPORT inline
#    define Q_WIDGETS_EXPORT_INLINE Q_WIDGETS_EXPORT inline
#    define Q_PLATFORMSUPPORT_EXPORT_INLINE Q_PLATFORMSUPPORT_EXPORT inline
#    define Q_PRINTSUPPORT_EXPORT_INLINE Q_PRINTSUPPORT_EXPORT inline
#    define Q_COMPAT_EXPORT_INLINE Q_COMPAT_EXPORT inline
#endif

/*
   No, this is not an evil backdoor. QT_BUILD_INTERNAL just exports more symbols
   for Qt's internal unit tests. If you want slower loading times and more
   symbols that can vanish from version to version, feel free to define QT_BUILD_INTERNAL.
*/
#if defined(QT_BUILD_INTERNAL) && defined(Q_OS_WIN) && defined(QT_MAKEDLL)
#    define Q_AUTOTEST_EXPORT Q_DECL_EXPORT
#elif defined(QT_BUILD_INTERNAL) && defined(Q_OS_WIN) && defined(QT_DLL)
#    define Q_AUTOTEST_EXPORT Q_DECL_IMPORT
#elif defined(QT_BUILD_INTERNAL) && !defined(Q_OS_WIN) && defined(QT_SHARED)
#    define Q_AUTOTEST_EXPORT Q_DECL_EXPORT
#else
#    define Q_AUTOTEST_EXPORT
#endif

inline void qt_noop(void) {}

/* These wrap try/catch so we can switch off exceptions later.

   Beware - do not use more than one QT_CATCH per QT_TRY, and do not use
   the exception instance in the catch block.
   If you can't live with those constraints, don't use these macros.
   Use the QT_NO_EXCEPTIONS macro to protect your code instead.
*/

#ifdef QT_BOOTSTRAPPED
#  define QT_NO_EXCEPTIONS
#endif
#if !defined(QT_NO_EXCEPTIONS) && defined(Q_CC_GNU) && !defined (__EXCEPTIONS) && !defined(Q_MOC_RUN)
#  define QT_NO_EXCEPTIONS
#endif

#ifdef QT_NO_EXCEPTIONS
#  define QT_TRY if (true)
#  define QT_CATCH(A) else
#  define QT_THROW(A) qt_noop()
#  define QT_RETHROW qt_noop()
#else
#  define QT_TRY try
#  define QT_CATCH(A) catch (A)
#  define QT_THROW(A) throw A
#  define QT_RETHROW throw
#endif

Q_CORE_EXPORT const char *qVersion();
Q_CORE_EXPORT bool qSharedBuild();

#ifndef Q_OUTOFLINE_TEMPLATE
#  define Q_OUTOFLINE_TEMPLATE
#endif
#ifndef Q_INLINE_TEMPLATE
#  define Q_INLINE_TEMPLATE inline
#endif

/*
   Avoid "unused parameter" warnings
*/

#if defined(Q_CC_INTEL) && !defined(Q_OS_WIN) || defined(Q_CC_RVCT)
template <typename T>
inline void qUnused(T &x) { (void)x; }
#  define Q_UNUSED(x) qUnused(x);
#else
#  define Q_UNUSED(x) (void)x;
#endif

/*
   Debugging and error handling
*/

#if !defined(QT_NO_DEBUG) && !defined(QT_DEBUG)
#  define QT_DEBUG
#endif

#ifndef qPrintable
#  define qPrintable(string) QString(string).toLocal8Bit().constData()
#endif

class QString;
Q_CORE_EXPORT QString qt_error_string(int errorCode = -1);
Q_CORE_EXPORT void qt_assert(const char *assertion, const char *file, int line);

#if !defined(Q_ASSERT)
#  if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
#    define Q_ASSERT(cond) qt_noop()
#  else
#    define Q_ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())
#  endif
#endif

#if defined(QT_NO_DEBUG) && !defined(QT_PAINT_DEBUG)
#define QT_NO_PAINT_DEBUG
#endif

Q_CORE_EXPORT void qt_assert_x(const char *where, const char *what, const char *file, int line);

#if !defined(Q_ASSERT_X)
#  if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
#    define Q_ASSERT_X(cond, where, what) qt_noop()
#  else
#    define Q_ASSERT_X(cond, where, what) ((!(cond)) ? qt_assert_x(where, what,__FILE__,__LINE__) : qt_noop())
#  endif
#endif


#ifdef Q_COMPILER_STATIC_ASSERT
#define Q_STATIC_ASSERT(Condition) static_assert(bool(Condition), #Condition)
#define Q_STATIC_ASSERT_X(Condition, Message) static_assert(bool(Condition), Message)
#else
// Intentionally undefined
template <bool Test> class QStaticAssertFailure;
template <> class QStaticAssertFailure<true> {};

#define Q_STATIC_ASSERT_PRIVATE_JOIN(A, B) Q_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B)
#define Q_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B) A ## B
#define Q_STATIC_ASSERT(Condition) \
    enum {Q_STATIC_ASSERT_PRIVATE_JOIN(q_static_assert_result, __LINE__) = sizeof(QStaticAssertFailure<!!(Condition)>)}
#define Q_STATIC_ASSERT_X(Condition, Message) Q_STATIC_ASSERT(Condition)
#endif

Q_CORE_EXPORT void qt_check_pointer(const char *, int);
Q_CORE_EXPORT void qBadAlloc();

#ifdef QT_NO_EXCEPTIONS
#  if defined(QT_NO_DEBUG)
#    define Q_CHECK_PTR(p) qt_noop()
#  else
#    define Q_CHECK_PTR(p) do {if(!(p))qt_check_pointer(__FILE__,__LINE__);} while (0)
#  endif
#else
#  define Q_CHECK_PTR(p) do { if (!(p)) qBadAlloc(); } while (0)
#endif

template <typename T>
inline T *q_check_ptr(T *p) { Q_CHECK_PTR(p); return p; }

#if (defined(Q_CC_GNU) && !defined(Q_OS_SOLARIS)) || defined(Q_CC_HPACC) || defined(Q_CC_DIAB)
#  define Q_FUNC_INFO __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define Q_FUNC_INFO __FUNCSIG__
#else
#   if defined(Q_OS_SOLARIS) || defined(Q_CC_XLC)
#      define Q_FUNC_INFO __FILE__ "(line number unavailable)"
#   else
        /* These two macros makes it possible to turn the builtin line expander into a
         * string literal. */
#       define QT_STRINGIFY2(x) #x
#       define QT_STRINGIFY(x) QT_STRINGIFY2(x)
#       define Q_FUNC_INFO __FILE__ ":" QT_STRINGIFY(__LINE__)
#   endif
    /* The MIPSpro and RVCT compilers postpones macro expansion,
       and therefore macros must be in scope when being used. */
#   if !defined(Q_CC_MIPS) && !defined(Q_CC_RVCT)
#       undef QT_STRINGIFY2
#       undef QT_STRINGIFY
#   endif
#endif


typedef void (*QFunctionPointer)();

#if !defined(Q_UNIMPLEMENTED)
#  define Q_UNIMPLEMENTED() qWarning("%s:%d: %s: Unimplemented code.", __FILE__, __LINE__, Q_FUNC_INFO)
#endif

#if defined(QT_NO_THREAD)

template <typename T>
class QGlobalStatic
{
public:
    T *pointer;
    inline QGlobalStatic(T *p) : pointer(p) { }
    inline ~QGlobalStatic() { pointer = 0; }
};

#define Q_GLOBAL_STATIC(TYPE, NAME)                                  \
    static TYPE *NAME()                                              \
    {                                                                \
        static TYPE thisVariable;                                    \
        static QGlobalStatic<TYPE > thisGlobalStatic(&thisVariable); \
        return thisGlobalStatic.pointer;                             \
    }

#define Q_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                  \
    static TYPE *NAME()                                              \
    {                                                                \
        static TYPE thisVariable ARGS;                               \
        static QGlobalStatic<TYPE > thisGlobalStatic(&thisVariable); \
        return thisGlobalStatic.pointer;                             \
    }

#define Q_GLOBAL_STATIC_WITH_INITIALIZER(TYPE, NAME, INITIALIZER)    \
    static TYPE *NAME()                                              \
    {                                                                \
        static TYPE thisVariable;                                    \
        static QGlobalStatic<TYPE > thisGlobalStatic(0);             \
        if (!thisGlobalStatic.pointer) {                             \
            TYPE *x = thisGlobalStatic.pointer = &thisVariable;      \
            INITIALIZER;                                             \
        }                                                            \
        return thisGlobalStatic.pointer;                             \
    }

#else

// forward declaration, since qatomic.h needs qglobal.h
template <typename T> class QBasicAtomicPointer;

// POD for Q_GLOBAL_STATIC
template <typename T>
class QGlobalStatic
{
public:
    QBasicAtomicPointer<T> pointer;
    bool destroyed;
};

// Created as a function-local static to delete a QGlobalStatic<T>
template <typename T>
class QGlobalStaticDeleter
{
public:
    QGlobalStatic<T> &globalStatic;
    QGlobalStaticDeleter(QGlobalStatic<T> &_globalStatic)
        : globalStatic(_globalStatic)
    { }

    inline ~QGlobalStaticDeleter()
    {
        delete globalStatic.pointer.load();
        globalStatic.pointer.store(0);
        globalStatic.destroyed = true;
    }
};

#define Q_GLOBAL_STATIC(TYPE, NAME)                                           \
    static TYPE *NAME()                                                       \
    {                                                                         \
        static QGlobalStatic<TYPE > thisGlobalStatic                          \
                            = { Q_BASIC_ATOMIC_INITIALIZER(0), false };       \
        if (!thisGlobalStatic.pointer.load() && !thisGlobalStatic.destroyed) { \
            TYPE *x = new TYPE;                                               \
            if (!thisGlobalStatic.pointer.testAndSetOrdered(0, x))            \
                delete x;                                                     \
            else                                                              \
                static QGlobalStaticDeleter<TYPE > cleanup(thisGlobalStatic); \
        }                                                                     \
        return thisGlobalStatic.pointer.load();                               \
    }

#define Q_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                           \
    static TYPE *NAME()                                                       \
    {                                                                         \
        static QGlobalStatic<TYPE > thisGlobalStatic                          \
                            = { Q_BASIC_ATOMIC_INITIALIZER(0), false };       \
        if (!thisGlobalStatic.pointer.load() && !thisGlobalStatic.destroyed) {       \
            TYPE *x = new TYPE ARGS;                                          \
            if (!thisGlobalStatic.pointer.testAndSetOrdered(0, x))            \
                delete x;                                                     \
            else                                                              \
                static QGlobalStaticDeleter<TYPE > cleanup(thisGlobalStatic); \
        }                                                                     \
        return thisGlobalStatic.pointer.load();                               \
    }

#define Q_GLOBAL_STATIC_WITH_INITIALIZER(TYPE, NAME, INITIALIZER)             \
    static TYPE *NAME()                                                       \
    {                                                                         \
        static QGlobalStatic<TYPE > thisGlobalStatic                          \
                            = { Q_BASIC_ATOMIC_INITIALIZER(0), false };       \
        if (!thisGlobalStatic.pointer.load() && !thisGlobalStatic.destroyed) {       \
            QScopedPointer<TYPE > x(new TYPE);                                \
            INITIALIZER;                                                      \
            if (thisGlobalStatic.pointer.testAndSetOrdered(0, x.data())) {    \
                static QGlobalStaticDeleter<TYPE > cleanup(thisGlobalStatic); \
                x.take();                                                     \
            }                                                                 \
        }                                                                     \
        return thisGlobalStatic.pointer.load();                               \
    }

#endif

Q_DECL_CONSTEXPR static inline bool qFuzzyCompare(double p1, double p2)
{
    return (qAbs(p1 - p2) <= 0.000000000001 * qMin(qAbs(p1), qAbs(p2)));
}

Q_DECL_CONSTEXPR static inline bool qFuzzyCompare(float p1, float p2)
{
    return (qAbs(p1 - p2) <= 0.00001f * qMin(qAbs(p1), qAbs(p2)));
}

/*!
  \internal
*/
Q_DECL_CONSTEXPR static inline bool qFuzzyIsNull(double d)
{
    return qAbs(d) <= 0.000000000001;
}

/*!
  \internal
*/
Q_DECL_CONSTEXPR static inline bool qFuzzyIsNull(float f)
{
    return qAbs(f) <= 0.00001f;
}

/*
   This function tests a double for a null value. It doesn't
   check whether the actual value is 0 or close to 0, but whether
   it is binary 0.
*/
static inline bool qIsNull(double d)
{
    union U {
        double d;
        quint64 u;
    };
    U val;
    val.d = d;
    return val.u == quint64(0);
}

/*
   This function tests a float for a null value. It doesn't
   check whether the actual value is 0 or close to 0, but whether
   it is binary 0.
*/
static inline bool qIsNull(float f)
{
    union U {
        float f;
        quint32 u;
    };
    U val;
    val.f = f;
    return val.u == 0u;
}

/*
   Compilers which follow outdated template instantiation rules
   require a class to have a comparison operator to exist when
   a QList of this type is instantiated. It's not actually
   used in the list, though. Hence the dummy implementation.
   Just in case other code relies on it we better trigger a warning
   mandating a real implementation.
*/

#ifdef Q_FULL_TEMPLATE_INSTANTIATION
#  define Q_DUMMY_COMPARISON_OPERATOR(C) \
    bool operator==(const C&) const { \
        qWarning(#C"::operator==(const "#C"&) was called"); \
        return false; \
    }
#else

#  define Q_DUMMY_COMPARISON_OPERATOR(C)
#endif

template <typename T>
inline void qSwap(T &value1, T &value2)
{
#ifdef QT_NO_STL
    const T t = value1;
    value1 = value2;
    value2 = t;
#else
    using std::swap;
    swap(value1, value2);
#endif
}

/*
   These functions make it possible to use standard C++ functions with
   a similar name from Qt header files (especially template classes).
*/
Q_CORE_EXPORT void *qMalloc(size_t size) Q_ALLOC_SIZE(1);
Q_CORE_EXPORT void qFree(void *ptr);
Q_CORE_EXPORT void *qRealloc(void *ptr, size_t size) Q_ALLOC_SIZE(2);
Q_CORE_EXPORT void *qMallocAligned(size_t size, size_t alignment) Q_ALLOC_SIZE(1);
Q_CORE_EXPORT void *qReallocAligned(void *ptr, size_t size, size_t oldsize, size_t alignment) Q_ALLOC_SIZE(2);
Q_CORE_EXPORT void qFreeAligned(void *ptr);
Q_CORE_EXPORT void *qMemCopy(void *dest, const void *src, size_t n);
Q_CORE_EXPORT void *qMemSet(void *dest, int c, size_t n);


/*
   Avoid some particularly useless warnings from some stupid compilers.
   To get ALL C++ compiler warnings, define QT_CC_WARNINGS or comment out
   the line "#define QT_NO_WARNINGS".
*/
#if !defined(QT_CC_WARNINGS)
#  define QT_NO_WARNINGS
#endif
#if defined(QT_NO_WARNINGS)
#  if defined(Q_CC_MSVC)
#    pragma warning(disable: 4251) /* class 'A' needs to have dll interface for to be used by clients of class 'B'. */
#    pragma warning(disable: 4244) /* 'conversion' conversion from 'type1' to 'type2', possible loss of data */
#    pragma warning(disable: 4275) /* non - DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier' */
#    pragma warning(disable: 4514) /* unreferenced inline/local function has been removed */
#    pragma warning(disable: 4800) /* 'type' : forcing value to bool 'true' or 'false' (performance warning) */
#    pragma warning(disable: 4097) /* typedef-name 'identifier1' used as synonym for class-name 'identifier2' */
#    pragma warning(disable: 4706) /* assignment within conditional expression */
#    pragma warning(disable: 4786) /* truncating debug info after 255 characters */
#    pragma warning(disable: 4660) /* template-class specialization 'identifier' is already instantiated */
#    pragma warning(disable: 4355) /* 'this' : used in base member initializer list */
#    pragma warning(disable: 4231) /* nonstandard extension used : 'extern' before template explicit instantiation */
#    pragma warning(disable: 4710) /* function not inlined */
#    pragma warning(disable: 4530) /* C++ exception handler used, but unwind semantics are not enabled. Specify -GX */
#  elif defined(Q_CC_BOR)
#    pragma option -w-inl
#    pragma option -w-aus
#    pragma warn -inl
#    pragma warn -pia
#    pragma warn -ccc
#    pragma warn -rch
#    pragma warn -sig
#  endif
#endif

class Q_CORE_EXPORT QFlag
{
    int i;
public:
    inline QFlag(int i);
    inline operator int() const { return i; }
};

inline QFlag::QFlag(int ai) : i(ai) {}

class Q_CORE_EXPORT QIncompatibleFlag
{
    int i;
public:
    inline explicit QIncompatibleFlag(int i);
    inline operator int() const { return i; }
};

inline QIncompatibleFlag::QIncompatibleFlag(int ai) : i(ai) {}


#ifndef Q_NO_TYPESAFE_FLAGS

template<typename Enum>
class QFlags
{
    typedef void **Zero;
    int i;
public:
    typedef Enum enum_type;
    Q_DECL_CONSTEXPR inline QFlags(const QFlags &f) : i(f.i) {}
    Q_DECL_CONSTEXPR inline QFlags(Enum f) : i(f) {}
    Q_DECL_CONSTEXPR inline QFlags(Zero = 0) : i(0) {}
    inline QFlags(QFlag f) : i(f) {}

    inline QFlags &operator=(const QFlags &f) { i = f.i; return *this; }
    inline QFlags &operator&=(int mask) { i &= mask; return *this; }
    inline QFlags &operator&=(uint mask) { i &= mask; return *this; }
    inline QFlags &operator|=(QFlags f) { i |= f.i; return *this; }
    inline QFlags &operator|=(Enum f) { i |= f; return *this; }
    inline QFlags &operator^=(QFlags f) { i ^= f.i; return *this; }
    inline QFlags &operator^=(Enum f) { i ^= f; return *this; }

    Q_DECL_CONSTEXPR  inline operator int() const { return i; }

    Q_DECL_CONSTEXPR inline QFlags operator|(QFlags f) const { return QFlags(Enum(i | f.i)); }
    Q_DECL_CONSTEXPR inline QFlags operator|(Enum f) const { return QFlags(Enum(i | f)); }
    Q_DECL_CONSTEXPR inline QFlags operator^(QFlags f) const { return QFlags(Enum(i ^ f.i)); }
    Q_DECL_CONSTEXPR inline QFlags operator^(Enum f) const { return QFlags(Enum(i ^ f)); }
    Q_DECL_CONSTEXPR inline QFlags operator&(int mask) const { return QFlags(Enum(i & mask)); }
    Q_DECL_CONSTEXPR inline QFlags operator&(uint mask) const { return QFlags(Enum(i & mask)); }
    Q_DECL_CONSTEXPR inline QFlags operator&(Enum f) const { return QFlags(Enum(i & f)); }
    Q_DECL_CONSTEXPR inline QFlags operator~() const { return QFlags(Enum(~i)); }

    Q_DECL_CONSTEXPR inline bool operator!() const { return !i; }

    inline bool testFlag(Enum f) const { return (i & f) == f && (f != 0 || i == int(f) ); }
};

#define Q_DECLARE_FLAGS(Flags, Enum)\
typedef QFlags<Enum> Flags;

#define Q_DECLARE_INCOMPATIBLE_FLAGS(Flags) \
inline QIncompatibleFlag operator|(Flags::enum_type f1, int f2) \
{ return QIncompatibleFlag(int(f1) | f2); }

#define Q_DECLARE_OPERATORS_FOR_FLAGS(Flags) \
Q_DECL_CONSTEXPR inline QFlags<Flags::enum_type> operator|(Flags::enum_type f1, Flags::enum_type f2) \
{ return QFlags<Flags::enum_type>(f1) | f2; } \
Q_DECL_CONSTEXPR inline QFlags<Flags::enum_type> operator|(Flags::enum_type f1, QFlags<Flags::enum_type> f2) \
{ return f2 | f1; } Q_DECLARE_INCOMPATIBLE_FLAGS(Flags)


#else /* Q_NO_TYPESAFE_FLAGS */

#define Q_DECLARE_FLAGS(Flags, Enum)\
typedef uint Flags;
#define Q_DECLARE_OPERATORS_FOR_FLAGS(Flags)

#endif /* Q_NO_TYPESAFE_FLAGS */

#if defined(Q_CC_GNU) && !defined(Q_CC_INTEL) && !defined(Q_CC_RVCT)
/* make use of typeof-extension */
template <typename T>
class QForeachContainer {
public:
    inline QForeachContainer(const T& t) : c(t), brk(0), i(c.begin()), e(c.end()) { }
    const T c;
    int brk;
    typename T::const_iterator i, e;
};

#define Q_FOREACH(variable, container)                                \
for (QForeachContainer<__typeof__(container)> _container_(container); \
     !_container_.brk && _container_.i != _container_.e;              \
     __extension__  ({ ++_container_.brk; ++_container_.i; }))                       \
    for (variable = *_container_.i;; __extension__ ({--_container_.brk; break;}))

#else

struct QForeachContainerBase {};

template <typename T>
class QForeachContainer : public QForeachContainerBase {
public:
    inline QForeachContainer(const T& t): c(t), brk(0), i(c.begin()), e(c.end()){}
    const T c;
    mutable int brk;
    mutable typename T::const_iterator i, e;
    inline bool condition() const { return (!brk++ && i != e); }
};

template <typename T> inline T *qForeachPointer(const T &) { return 0; }

template <typename T> inline QForeachContainer<T> qForeachContainerNew(const T& t)
{ return QForeachContainer<T>(t); }

template <typename T>
inline const QForeachContainer<T> *qForeachContainer(const QForeachContainerBase *base, const T *)
{ return static_cast<const QForeachContainer<T> *>(base); }

#if defined(Q_CC_MIPS)
/*
   Proper for-scoping in MIPSpro CC
*/
#  define Q_FOREACH(variable,container)                                                             \
    if(0){}else                                                                                     \
    for (const QForeachContainerBase &_container_ = qForeachContainerNew(container);                \
         qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->condition();       \
         ++qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i)               \
        for (variable = *qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i; \
             qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk;           \
             --qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk)

#elif defined(Q_CC_DIAB)
// VxWorks DIAB generates unresolvable symbols, if container is a function call
#  define Q_FOREACH(variable,container)                                                             \
    if(0){}else                                                                                     \
    for (const QForeachContainerBase &_container_ = qForeachContainerNew(container);                \
         qForeachContainer(&_container_, (__typeof__(container) *) 0)->condition();       \
         ++qForeachContainer(&_container_, (__typeof__(container) *) 0)->i)               \
        for (variable = *qForeachContainer(&_container_, (__typeof__(container) *) 0)->i; \
             qForeachContainer(&_container_, (__typeof__(container) *) 0)->brk;           \
             --qForeachContainer(&_container_, (__typeof__(container) *) 0)->brk)

#else
#  define Q_FOREACH(variable, container) \
    for (const QForeachContainerBase &_container_ = qForeachContainerNew(container); \
         qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->condition();       \
         ++qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i)               \
        for (variable = *qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i; \
             qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk;           \
             --qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk)
#endif // MSVC6 || MIPSpro

#endif

#define Q_FOREVER for(;;)
#ifndef QT_NO_KEYWORDS
#  ifndef foreach
#    define foreach Q_FOREACH
#  endif
#  ifndef forever
#    define forever Q_FOREVER
#  endif
#endif

#if 0
/* tell gcc to use its built-in methods for some common functions */
#if defined(QT_NO_DEBUG) && defined(Q_CC_GNU)
#  define qMemCopy __builtin_memcpy
#  define qMemSet __builtin_memset
#endif
#endif

template <typename T> static inline T *qGetPtrHelper(T *ptr) { return ptr; }
template <typename Wrapper> static inline typename Wrapper::pointer qGetPtrHelper(const Wrapper &p) { return p.data(); }

#define Q_DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(qGetPtrHelper(d_ptr)); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(qGetPtrHelper(d_ptr)); } \
    friend class Class##Private;

#define Q_DECLARE_PRIVATE_D(Dptr, Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(Dptr); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(Dptr); } \
    friend class Class##Private;

#define Q_DECLARE_PUBLIC(Class)                                    \
    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
    friend class Class;

#define Q_D(Class) Class##Private * const d = d_func()
#define Q_Q(Class) Class * const q = q_func()

#define QT_TR_NOOP(x) x
#define QT_TR_NOOP_UTF8(x) x
#define QT_TRANSLATE_NOOP(scope, x) x
#define QT_TRANSLATE_NOOP_UTF8(scope, x) x
#define QT_TRANSLATE_NOOP3(scope, x, comment) {x, comment}
#define QT_TRANSLATE_NOOP3_UTF8(scope, x, comment) {x, comment}

#ifndef QT_NO_TRANSLATION // ### This should enclose the NOOPs above

// Defined in qcoreapplication.cpp
// The better name qTrId() is reserved for an upcoming function which would
// return a much more powerful QStringFormatter instead of a QString.
Q_CORE_EXPORT QString qtTrId(const char *id, int n = -1);

#define QT_TRID_NOOP(id) id

#endif // QT_NO_TRANSLATION

#define QDOC_PROPERTY(text)

/*
   When RTTI is not available, define this macro to force any uses of
   dynamic_cast to cause a compile failure.
*/

#ifdef QT_NO_DYNAMIC_CAST
#  define dynamic_cast QT_PREPEND_NAMESPACE(qt_dynamic_cast_check)

  template<typename T, typename X>
  T qt_dynamic_cast_check(X, T* = 0)
  { return T::dynamic_cast_will_always_fail_because_rtti_is_disabled; }
#endif

/*
   Some classes do not permit copies to be made of an object. These
   classes contains a private copy constructor and assignment
   operator to disable copying (the compiler gives an error message).
*/
#define Q_DISABLE_COPY(Class) \
    Class(const Class &) Q_DECL_EQ_DELETE;\
    Class &operator=(const Class &) Q_DECL_EQ_DELETE;

class QByteArray;
Q_CORE_EXPORT QByteArray qgetenv(const char *varName);
Q_CORE_EXPORT bool qputenv(const char *varName, const QByteArray& value);

inline int qIntCast(double f) { return int(f); }
inline int qIntCast(float f) { return int(f); }

/*
  Reentrant versions of basic rand() functions for random number generation
*/
Q_CORE_EXPORT void qsrand(uint seed);
Q_CORE_EXPORT int qrand();

#define QT_MODULE(x)

#ifdef Q_OS_QNX
// QNX doesn't have SYSV style shared memory. Multiprocess QWS apps,
// shared fonts and QSystemSemaphore + QSharedMemory are not available
#  define QT_NO_QWS_MULTIPROCESS
#  define QT_NO_QWS_SHARE_FONTS
#  define QT_NO_SYSTEMSEMAPHORE
#  define QT_NO_SHAREDMEMORY
#endif

#if defined (__ELF__)
#  if defined (Q_OS_LINUX) || defined (Q_OS_SOLARIS) || defined (Q_OS_FREEBSD) || defined (Q_OS_OPENBSD) || defined (Q_OS_IRIX)
#    define Q_OF_ELF
#  endif
#endif

#if !defined(QT_BOOTSTRAPPED) && defined(QT_REDUCE_RELOCATIONS) && defined(__ELF__) && !defined(__PIC__)
#  error "You must build your code with position independent code if Qt was built with -reduce-relocations. "\
         "Compile your code with -fPIC or -fPIE."
#endif

namespace QtPrivate {
//like std::enable_if
template <bool B, typename T = void> struct QEnableIf;
template <typename T> struct QEnableIf<true, T> { typedef T Type; };
}

QT_END_NAMESPACE
QT_END_HEADER

// qDebug and friends
#include <QtCore/qlogging.h>

#include <QtCore/qsysinfo.h>
#include <QtCore/qtypeinfo.h>

#endif /* __cplusplus */

#endif /* QGLOBAL_H */
