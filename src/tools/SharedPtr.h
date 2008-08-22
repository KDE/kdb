/*
 * This file is part of the KDE libraries.
 *
 * Copyright 2005 Frerich Raabe <raabe@kde.org>
 *
 * It is renamed copy of trunk/kdelibs/kdecore/utils/ksharedptr.h
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef PREDICATE_TOOLS_SHAREDPTR_H
#define PREDICATE_TOOLS_SHAREDPTR_H

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QAtomicPointer>

namespace Predicate
{
namespace Utils
{

typedef QSharedData Shared;

/**
 * \class SharedPtr SharedPtr.h <Predicate::Utils::SharedPtr>
 * 
 * Can be used to control the lifetime of an object that has derived
 * QSharedData. As long a someone holds
 * a SharedPtr on some QSharedData object it won't become deleted but
 * is deleted once its reference count is 0.
 * This struct emulates C++ pointers virtually perfectly.
 * So just use it like a simple C++ pointer.
 *
 * The difference with using QSharedDataPointer is that QSharedDataPointer is
 * a building block for implementing a value class with implicit sharing (like QString),
 * whereas SharedPtr provides refcounting to code that uses pointers.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
template< class T >
class SharedPtr
{
public:
    /**
     * Creates a null pointer.
     */
    inline SharedPtr()
        : d(0) { }

    /**
     * Creates a new pointer.
     * @param p the pointer
     */
    inline explicit SharedPtr( T* p )
        : d(p) { if(d) d->ref.ref(); }

    /**
     * Copies a pointer.
     * @param o the pointer to copy
     */
    inline SharedPtr( const SharedPtr& o )
        : d(o.d) { if(d) d->ref.ref(); }

    /**
     * Unreferences the object that this pointer points to. If it was
     * the last reference, the object will be deleted.
     */
    inline ~SharedPtr() { if (d && !d->ref.deref()) delete d; }

    inline SharedPtr<T>& operator= ( const SharedPtr& o ) { attach(o.d); return *this; }
    inline bool operator== ( const SharedPtr& o ) const { return ( d == o.d ); }
    inline bool operator!= ( const SharedPtr& o ) const { return ( d != o.d ); }
    inline bool operator< ( const SharedPtr& o ) const { return ( d < o.d ); }

    inline SharedPtr<T>& operator= ( T* p ) { attach(p); return *this; }
    inline bool operator== ( const T* p ) const { return ( d == p ); }
    inline bool operator!= ( const T* p ) const { return ( d != p ); }

    /**
     * Test if the shared pointer is NOT null.
     * @return true if the shared pointer is NOT null, false otherwise.
     * @see isNull
     */
    inline operator bool() const { return ( d != 0 ); }

    /**
     * @return the pointer
     */
    inline T* data() { return d; }

    /**
     * @return the pointer
     */
    inline const T* data() const { return d; }

    /**
     * @return a const pointer to the shared object.
     */
    inline const T* constData() const { return d; }

    inline const T& operator*() const { Q_ASSERT(d); return *d; }
    inline T& operator*() { Q_ASSERT(d); return *d; }
    inline const T* operator->() const { Q_ASSERT(d); return d; }
    inline T* operator->() { Q_ASSERT(d); return d; }

    /**
     * Attach the given pointer to the current SharedPtr.
     * If the previous shared pointer is not owned by any SharedPtr,
     * it is deleted.
     */
    void attach(T* p);

    /**
     * Clear the pointer, i.e. make it a null pointer.
     */
    void clear();

    /**
     * Returns the number of references.
     * @return the number of references
     */
    inline int count() const { return d ? static_cast<int>(d->ref) : 0; } // for debugging purposes

    /**
     * Test if the shared pointer is null.
     * @return true if the pointer is null, false otherwise.
     * @see opertor (bool)
     */
    inline bool isNull() const { return (d == 0); }

    /**
     * @return Whether this is the only shared pointer pointing to
     * to the pointee, or whether it's shared among multiple
     * shared pointers.
     */
    inline bool isUnique() const { return count() == 1; }

    template <class U> friend class SharedPtr;

    /**
     * Convert SharedPtr<U> to SharedPtr<T>, using a static_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * <code>
     *   SharedPtr<T> tPtr;
     *   SharedPtr<U> uPtr = SharedPtr<U>::staticCast( tPtr );
     * </code>
     */
    template <class U>
    static SharedPtr<T> staticCast( const SharedPtr<U>& o ) {
        return SharedPtr<T>( static_cast<T *>( o.d ) );
    }
    /**
     * Convert SharedPtr<U> to SharedPtr<T>, using a dynamic_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * <code>
     *   SharedPtr<T> tPtr;
     *   SharedPtr<U> uPtr = SharedPtr<U>::dynamicCast( tPtr );
     * </code>
     * Since a dynamic_cast is used, if U derives from T, and tPtr isn't an instance of U, uPtr will be 0.
     */
    template <class U>
    static SharedPtr<T> dynamicCast( const SharedPtr<U>& o ) {
        return SharedPtr<T>( dynamic_cast<T *>( o.d ) );
    }

protected:
    T* d;
};

template <class T>
Q_INLINE_TEMPLATE bool operator== (const T* p, const SharedPtr<T>& o)
{
    return ( p == o.d );
}

template <class T>
Q_INLINE_TEMPLATE bool operator!= (const T* p, const SharedPtr<T>& o)
{
    return ( p != o.d );
}

template <class T>
Q_INLINE_TEMPLATE void SharedPtr<T>::attach(T* p)
{
    if (d != p) {
        if (p) p->ref.ref();
        if (d && !d->ref.deref())
            delete d;
        d = p;
    }
}

template <class T>
Q_INLINE_TEMPLATE void SharedPtr<T>::clear()
{
    attach(static_cast<T*>(0));
}

} //ns
} //ns

#endif

