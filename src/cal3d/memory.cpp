#include "cal3d/global.h"
#include "cal3d/memory.h"

void* cal3d::allocate_aligned_data(size_t size) {
    void* new_data = CAL3D_ALIGNED_MALLOC(size, 64);
    if (!new_data) {
        throw std::bad_alloc();
    }
    return new_data;
}


