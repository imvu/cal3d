#include "cal3d/global.h"

void* allocate_aligned_data(size_t size) {
  void* new_data = _aligned_malloc(size, 64);
  if (!new_data) {
    throw std::bad_alloc();
  }
  return new_data;
}
