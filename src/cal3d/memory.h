#pragma once

#include <cstddef>
#include <iterator>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <boost/noncopyable.hpp>
#include "cal3d/platform.h"

namespace cal3d {
    CAL3D_API void* allocate_aligned_data(size_t size);

    // Can't use std::vector w/ __declspec(align(16)) :(
    // http://ompf.org/forum/viewtopic.php?f=11&t=686
    // http://social.msdn.microsoft.com/Forums/en-US/vclanguage/thread/0adabdb5-f732-4db7-a8de-e3e83af0e147/
    template<typename T>
    struct SSEArray : boost::noncopyable {
        typedef T value_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        typedef T* iterator;
        typedef T const* const_iterator;

        SSEArray()
            : _data(0)
            , _size(0)
            , _capacity(0)
        {}

        SSEArray(const SSEArray& other)
            : _data(reinterpret_cast<T*>(allocate_aligned_data(sizeof(T) * other.size())))
            , _size(other.size())
            , _capacity(other.size())
        {
            const T* s = other.data();
            for (T* d = _data; d != _data + _size; ++d, ++s) {
                void* p = d;
                new(p) T(*s);
            }
        }

        SSEArray(size_t initial_size)
            : _data(reinterpret_cast<T*>(allocate_aligned_data(sizeof(T) * initial_size)))
            , _size(initial_size)
            , _capacity(initial_size)
        {
            for (T* d = _data; d != _data + _size; ++d) {
                void* p = d;
                new(p) T;
            }
        }

        template <typename IteratorT>
        SSEArray(const IteratorT& beginIt, const IteratorT& endIt)
            : _data(reinterpret_cast<T*>(allocate_aligned_data(sizeof(T) * std::distance(beginIt, endIt))))
            , _size(std::distance(beginIt, endIt))
            , _capacity(std::distance(beginIt, endIt))
        {
            auto it = beginIt;
            for (T* d = _data; d != _data + _size; ++d, ++it) {
                void* p = d;
                new(p) T(*it);
            }
        }

        ~SSEArray() {
            CAL3D_ALIGNED_FREE(_data);
        }

        // does not preserve array contents
        void destructive_resize(size_t new_size) {
            if (_size != new_size) {
                T* new_data = reinterpret_cast<T*>(allocate_aligned_data(sizeof(T) * new_size));

                if (_data) {
                    CAL3D_ALIGNED_FREE(_data);
                }
                _size = new_size;
                _capacity = new_size;
                _data = new_data;
            }
        }

        T& operator[](size_t idx) {
            return _data[idx];
        }

        const T& operator[](size_t idx) const {
            return _data[idx];
        }

        size_t size() const {
            return _size;
        }

        void push_back(const T& v) {
            if (_size + 1 > _capacity) {
                size_t new_capacity = _capacity ? (_capacity * 2) : 1;
                T* new_data = reinterpret_cast<T*>(allocate_aligned_data(sizeof(T) * new_capacity));
                std::copy(_data, _data + _size, new_data);

                CAL3D_ALIGNED_FREE(_data);
                _capacity = new_capacity;
                _data = new_data;
            }

            new(_data + _size) T(v);
            ++_size;
        }

        iterator begin() { return _data; }
        const_iterator begin() const { return _data; }
        iterator end() { return _data + _size; }
        const_iterator end() const { return _data + _size; }

        T* data() { return _data; }
        const T* data() const { return _data; }

        void swap(SSEArray& that) {
            std::swap(_data, that._data);
            std::swap(_size, that._size);
            std::swap(_capacity, that._capacity);
        }

    private:
        T* _data;
        size_t _size;
        size_t _capacity;
    };

    template<unsigned Alignment>
    struct AlignedMemory {
        void* operator new(size_t size) {
            return CAL3D_ALIGNED_MALLOC(size, Alignment);
        }
        void operator delete(void* p) {
            CAL3D_ALIGNED_FREE(p);
        }
    };

    template<typename T, typename A>
    T* pointerFromVector(std::vector<T, A>& v) {
        if (v.empty()) {
            return 0;
        } else {
            return &v[0];
        }
    }

    template<typename T, typename A>
    const T* pointerFromVector(const std::vector<T, A>& v) {
        if (v.empty()) {
            return 0;
        } else {
            return &v[0];
        }
    }

    template<typename T>
    T* pointerFromVector(SSEArray<T>& v) {
        return v.data();
    }

    template<typename T>
    const T* pointerFromVector(const SSEArray<T>& v) {
        return v.data();
    }
}

#define CAL3D_DEFINE_SIZE(T) \
    inline size_t sizeInBytes(T const&) { \
        return sizeof(T); \
    }

CAL3D_DEFINE_SIZE(unsigned);

template<typename T>
size_t sizeInBytes(const cal3d::SSEArray<T>& v) {
    return sizeof(T) * v.size();
}

template<typename T>
size_t sizeInBytes(const std::vector<T>& v) {
    size_t r = sizeof(T) * (v.capacity() - v.size());
    for (typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); ++i) {
        r += sizeInBytes(*i);
    }
    return r;
}

template<typename T>
size_t sizeInBytes(const std::list<T>& v) {
    size_t r = 8 * v.size(); // pointers each way
    for (typename std::list<T>::const_iterator i = v.begin(); i != v.end(); ++i) {
        r += sizeInBytes(*i);
    }
    return r;
}

template<typename T>
size_t sizeInBytes(const std::set<T>& s) {
    size_t r = 20 * s.size();
    for (typename std::set<T>::const_iterator i = s.begin(); i != s.end(); ++i) {
        r += sizeInBytes(*i);
    }
    return r;
}

template<typename K, typename V>
size_t sizeInBytes(const std::map<K, V>& m) {
    size_t r = 20 * m.size();
    for (typename std::map<K, V>::const_iterator i = m.begin(); i != m.end(); ++i) {
        r += sizeInBytes(i->first) + sizeInBytes(i->second);
    }
    return r;
}
