#pragma once

#include "cal3d/global.h"

CAL3D_API void sortTrianglesBackToFront(
    unsigned triangleCount,
    unsigned short* indices,      // indexed trilist, triangleCount*3 entries
    const float* positions, // vec3f
    unsigned positionStride = 3); // in floats, defaults to packed

