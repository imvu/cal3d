#include "cal3d/global.h"

void* allocate_aligned_data(size_t size) {
  void* new_data = _aligned_malloc(size, 64);
  if (!new_data) {
    throw std::bad_alloc();
  }
  return new_data;
}


namespace Cal {
    void* __cdecl Object::operator new(size_t count) {
        return ::operator new(count);
    }
    void* __cdecl Object::operator new(size_t count, const std::nothrow_t&) throw() {
        return ::operator new(count, std::nothrow);
    }
    void __cdecl Object::operator delete(void* object) {
        return ::operator delete(object);
    }
    void __cdecl Object::operator delete(void* object, const std::nothrow_t&) throw() {
        return ::operator delete(object, std::nothrow);
    }

    void* __cdecl Object::operator new[](size_t count) {
        return ::operator new[](count);
    }
    void* __cdecl Object::operator new[](size_t count, const std::nothrow_t&) throw() {
        return ::operator new[](count, std::nothrow);
    }
    void __cdecl Object::operator delete[](void* object) {
        return ::operator delete[](object);
    }
    void __cdecl Object::operator delete[](void* object, const std::nothrow_t&) throw() {
        return ::operator delete[](object, std::nothrow);
    }
}
