//****************************************************************************//
// platform.h                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <memory>
#include <stdlib.h>

#if !defined(_WIN32) || defined(__MINGW32__)
#define stricmp strcasecmp
#endif


//****************************************************************************//
// Dynamic library export setup                                               //
//****************************************************************************//

#if defined(_WIN32) && !defined(__MINGW32__)
// MSVC
#  ifdef CAL3D_EXPORTS
#    define CAL3D_API __declspec(dllexport)
#  else
#    define CAL3D_API __declspec(dllimport)
#  endif

#  define CAL3D_CDECL __cdecl
#  define CAL3D_FORCEINLINE __forceinline
#  define CAL3D_ALIGN_HEAD(X) __declspec(align(X))
#  define CAL3D_ALIGN_TAIL(X)
#  define CAL3D_ALIGNED_MALLOC(c, a) _aligned_malloc(c, a)
#  define CAL3D_ALIGNED_FREE _aligned_free

typedef __int64 cal3d_int64;
typedef unsigned __int64 cal3d_uint64;

#else
// Assume OSX
#  define CAL3D_API
#  define CAL3D_CDECL
#  define CAL3D_FORCEINLINE inline __attribute__((always_inline))
#  define CAL3D_ALIGN_HEAD(X)
#  define CAL3D_ALIGN_TAIL(X) __attribute__((aligned (X)))
#  define CAL3D_ALIGNED_MALLOC(c, a) malloc(c)
#  define CAL3D_ALIGNED_FREE free
#  define _stricmp stricmp

typedef long long cal3d_int64;
typedef unsigned long long cal3d_uint64;

#endif

//****************************************************************************//
// Endianness setup                                                           //
//****************************************************************************//

#if  defined(__i386__) || \
     defined(__ia64__) || \
     defined(WIN32) || \
     defined(__alpha__) || defined(__alpha) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)

#define CAL3D_LITTLE_ENDIAN

#else

#define CAL3D_BIG_ENDIAN

#endif


// Allocations objects that are 16-byte aligned
#if defined(WIN32) && !defined(__MINGW32__)
template<typename T>
struct SSEAllocator {
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;
    typedef size_t size_type;
    typedef T value_type;

    SSEAllocator() {}
    SSEAllocator(const SSEAllocator&) {}
    template<typename U> SSEAllocator(const SSEAllocator<U>&) {}
    ~SSEAllocator() {}

    pointer address(reference r) const { return &r; }
    const_pointer address(const_reference r) const { return &r; }

    pointer allocate(size_type count) {
        return reinterpret_cast<pointer>(_aligned_malloc(count * sizeof(T), 16));
    }

    template<typename U>
    pointer allocate(size_type count, const U* hint) {
        return allocate(count);
    }

    void construct(pointer ptr, const_reference value) {
        void* p = ptr;
        ::new(p) T(value);
    }

    void deallocate(pointer ptr, size_type) {
        _aligned_free(ptr);
    }

    void destroy(pointer ptr) {
        ptr->T::~T();
    }

    size_type max_size() const {
        return std::allocator<T>().max_size();
    }

    template<class U>
    struct rebind {
        typedef SSEAllocator<U> other;
    };

    template<class Other>
    SSEAllocator& operator=(const SSEAllocator<Other>& right) {
        return *this;
    }
};
#else
template <typename T>
class SSEAllocator : public std::allocator<T> {
public:
    SSEAllocator() { }

    template <typename U>
    SSEAllocator(const std::allocator<U>& a) { }
};
#endif
