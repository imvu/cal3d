#pragma once

#include "cal3d/vector.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

struct CalAABox {
    CalAABox()
    {}

    CalAABox(const CalVector& _min, const CalVector& _max)
        : min(_min)
        , max(_max)
    {}

    CalVector min;
    CalVector max;
};

inline bool operator==(const CalAABox& lhs, const CalAABox& rhs) {
    return lhs.min == rhs.min && lhs.max == rhs.max;
}
