#pragma once

#include "cal3d/global.h"
#include "cal3d/vector.h"


typedef unsigned int CalColor32; // 32-bit integer, compatible with NSPR

inline CalColor32 CalMakeColor(CalVector cv) {
    return
      ( ( unsigned int ) ( cv.z * 0xff ) << 0) +
      ( ( ( unsigned int ) ( cv.y * 0xff ) ) << 8 ) +
      ( ( ( unsigned int ) ( cv.x * 0xff ) ) << 16 ) +
      0xff000000;
}

inline CalVector CalVectorFromColor(CalColor32 color) {
    return CalVector(
        ((color >> 16) & 0xff) / float(0xff),
        ((color >> 8) & 0xff) / float(0xff),
        ((color >> 0) & 0xff) / float(0xff));
}
