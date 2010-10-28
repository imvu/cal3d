#include "cal3d/global.h"

void* allocate_aligned_data(size_t size) {
  void* new_data = CAL3D_ALIGNED_MALLOC(size, 64);
  if (!new_data) {
    throw std::bad_alloc();
  }
  return new_data;
}


namespace Cal {
    void* CAL3D_CDECL Object::operator new(size_t count) {
        return ::operator new(count);
    }
    void* CAL3D_CDECL Object::operator new(size_t count, const std::nothrow_t&) throw() {
        return ::operator new(count, std::nothrow);
    }
    void CAL3D_CDECL Object::operator delete(void* object) {
        return ::operator delete(object);
    }
    void CAL3D_CDECL Object::operator delete(void* object, const std::nothrow_t&) throw() {
        return ::operator delete(object, std::nothrow);
    }

    void* CAL3D_CDECL Object::operator new[](size_t count) {
        return ::operator new[](count);
    }
    void* CAL3D_CDECL Object::operator new[](size_t count, const std::nothrow_t&) throw() {
        return ::operator new[](count, std::nothrow);
    }
    void CAL3D_CDECL Object::operator delete[](void* object) {
        return ::operator delete[](object);
    }
    void CAL3D_CDECL Object::operator delete[](void* object, const std::nothrow_t&) throw() {
        return ::operator delete[](object, std::nothrow);
    }
}
