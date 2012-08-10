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

#ifndef Q_QDOC

#ifndef QOBJECT_H
#error Do not include qobject_impl.h directly
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


namespace QtPrivate {
    /*
        Logic to statically generate the array of qMetaTypeId
        ConnectionTypes<FunctionPointer<Signal>::Arguments>::types() returns an array
        of int that is suitable for the types arguments of the connection functions.

        The array only exist of all the types are declared as a metatype
        (detected using the TypesAreDeclaredMetaType helper struct)
        If one of the type is not declared, the function return 0 and the signal
        cannot be used in queued connection.
    */
#ifndef Q_COMPILER_VARIADIC_TEMPLATES
    template <typename ArgList> struct TypesAreDeclaredMetaType { enum { Value = false }; };
    template <> struct TypesAreDeclaredMetaType<void> { enum { Value = true }; };
    template <typename Arg, typename Tail> struct TypesAreDeclaredMetaType<List<Arg, Tail> > { enum { Value = QMetaTypeId2<Arg>::Defined && TypesAreDeclaredMetaType<Tail>::Value }; };

    template <typename ArgList, bool Declared = TypesAreDeclaredMetaType<ArgList>::Value > struct ConnectionTypes
    { static const int *types() { return 0; } };
    template <> struct ConnectionTypes<void, true>
    { static const int *types() { static const int t[1] = { 0 }; return t; } };
    template <typename Arg1> struct ConnectionTypes<List<Arg1, void>, true>
    { static const int *types() { static const int t[2] = { QtPrivate::QMetaTypeIdHelper<Arg1>::qt_metatype_id(), 0 }; return t; } };
    template <typename Arg1, typename Arg2> struct ConnectionTypes<List<Arg1, List<Arg2, void> >, true>
    { static const int *types() { static const int t[3] = { QtPrivate::QMetaTypeIdHelper<Arg1>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg2>::qt_metatype_id(), 0 }; return t; } };
    template <typename Arg1, typename Arg2, typename Arg3> struct ConnectionTypes<List<Arg1, List<Arg2,  List<Arg3, void> > >, true>
    { static const int *types() { static const int t[4] = { QtPrivate::QMetaTypeIdHelper<Arg1>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg2>::qt_metatype_id(),
                                                            QtPrivate::QMetaTypeIdHelper<Arg3>::qt_metatype_id(), 0 }; return t; } };
    template <typename Arg1, typename Arg2, typename Arg3, typename Arg4> struct ConnectionTypes<List<Arg1, List<Arg2,  List<Arg3, List<Arg4, void> > > >, true>
    { static const int *types() { static const int t[4] = { QtPrivate::QMetaTypeIdHelper<Arg1>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg2>::qt_metatype_id(),
                QtPrivate::QMetaTypeIdHelper<Arg3>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg4>::qt_metatype_id(), 0 }; return t; } };
    template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5> struct ConnectionTypes<List<Arg1, List<Arg2,  List<Arg3, List<Arg4, List<Arg5, void> > > > >, true>
    { static const int *types() { static const int t[4] = { QtPrivate::QMetaTypeIdHelper<Arg1>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg2>::qt_metatype_id(),
                QtPrivate::QMetaTypeIdHelper<Arg3>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg4>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg5>::qt_metatype_id(), 0 }; return t; } };
    template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
    struct ConnectionTypes<List<Arg1, List<Arg2,  List<Arg3, List<Arg4, List<Arg5, List<Arg6, void> > > > > >, true>
    { static const int *types() { static const int t[4] = { QtPrivate::QMetaTypeIdHelper<Arg1>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg2>::qt_metatype_id(),
            QtPrivate::QMetaTypeIdHelper<Arg3>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg4>::qt_metatype_id(), QtPrivate::QMetaTypeIdHelper<Arg5>::qt_metatype_id(),
            QtPrivate::QMetaTypeIdHelper<Arg6>::qt_metatype_id(), 0 }; return t; } };
#else
    template <typename ArgList> struct TypesAreDeclaredMetaType { enum { Value = false }; };
    template <> struct TypesAreDeclaredMetaType<List<>> { enum { Value = true }; };
    template <typename Arg, typename... Tail> struct TypesAreDeclaredMetaType<List<Arg, Tail...> >
    { enum { Value = QMetaTypeId2<Arg>::Defined && TypesAreDeclaredMetaType<List<Tail...>>::Value }; };

    template <typename ArgList, bool Declared = TypesAreDeclaredMetaType<ArgList>::Value > struct ConnectionTypes
    { static const int *types() { return 0; } };
    template <typename... Args> struct ConnectionTypes<List<Args...>, true>
    { static const int *types() { static const int t[sizeof...(Args) + 1] = { (QtPrivate::QMetaTypeIdHelper<Args>::qt_metatype_id())..., 0 }; return t; } };
#endif

    // internal base class (interface) containing functions required to call a slot managed by a pointer to function.
    struct QSlotObjectBase {
        QAtomicInt ref;
        // don't use virtual functions here; we don't want the
        // compiler to create tons of per-polymorphic-class stuff that
        // we'll never need. We just use one function pointer.
        enum Operation {
            Destroy,
            Call,
            Compare,

            NumOperations
        };
        typedef bool (*ImplFn)(int which, QSlotObjectBase* this_, QObject *receiver, void **args);
        const ImplFn impl;

        explicit QSlotObjectBase(ImplFn fn) : ref(1), impl(fn) {} // ### make constexpr once QAtomicInt's ctor is, too
        inline void destroy()                  { impl(Destroy, this, 0, 0); }
        inline bool compare(void **a)   { return impl(Compare, this, 0, a); }
        inline void call(QObject *r, void **a) { impl(Call,    this, r, a); }
    protected:
        ~QSlotObjectBase() {}
    };
    // implementation of QSlotObjectBase for which the slot is a pointer to member function of a QObject
    // Args and R are the List of arguments and the returntype of the signal to which the slot is connected.
    template<typename Func, typename Args, typename R> class QSlotObject : public QSlotObjectBase
    {
        typedef QtPrivate::FunctionPointer<Func> FuncType;
        Func function;
        static bool impl(int which, QSlotObjectBase *this_, QObject *r, void **a)
        {
            switch (which) {
            case Destroy:
                delete static_cast<QSlotObject*>(this_);
                return true;
            case Call:
                FuncType::template call<Args, R>(static_cast<QSlotObject*>(this_)->function, static_cast<typename FuncType::Object *>(r), a);
                return true;
            case Compare:
                return *reinterpret_cast<Func *>(a) == static_cast<QSlotObject*>(this_)->function;
            case NumOperations: ;
            }
            return false;
        }
    public:
        explicit QSlotObject(Func f) : QSlotObjectBase(&impl), function(f) {}
    };
    // implementation of QSlotObjectBase for which the slot is a static function
    // Args and R are the List of arguments and the returntype of the signal to which the slot is connected.
    template<typename Func, typename Args, typename R> class QStaticSlotObject : public QSlotObjectBase
    {
        typedef QtPrivate::FunctionPointer<Func> FuncType;
        Func function;
        static bool impl(int which, QSlotObjectBase *this_, QObject *r, void **a)
        {
            switch (which) {
            case Destroy:
                delete static_cast<QStaticSlotObject*>(this_);
                return true;
            case Call:
                FuncType::template call<Args, R>(static_cast<QStaticSlotObject*>(this_)->function, r, a);
                return true;
            case Compare:
                return false; // not implemented
            case NumOperations: ;
            }
            return false;
        }
    public:
        explicit QStaticSlotObject(Func f) : QSlotObjectBase(&impl), function(f) {}
    };
    // implementation of QSlotObjectBase for which the slot is a functor (or lambda)
    // N is the number of arguments
    // Args and R are the List of arguments and the returntype of the signal to which the slot is connected.
    template<typename Func, int N, typename Args, typename R> class QFunctorSlotObject : public QSlotObjectBase
    {
        typedef QtPrivate::Functor<Func, N> FuncType;
        Func function;
        static bool impl(int which, QSlotObjectBase *this_, QObject *r, void **a)
        {
            switch (which) {
            case Destroy:
                delete static_cast<QFunctorSlotObject*>(this_);
                return true;
            case Call:
                FuncType::template call<Args, R>(static_cast<QFunctorSlotObject*>(this_)->function, r, a);
                return true;
            case Compare:
                return false; // not implemented
            case NumOperations: ;
            }
            return false;
        }
    public:
        explicit QFunctorSlotObject(const Func &f) : QSlotObjectBase(&impl), function(f) {}
    };

}


QT_END_NAMESPACE

QT_END_HEADER

#endif
