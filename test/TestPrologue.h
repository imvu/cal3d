#pragma once

#include <TestFramework/TestFramework.h>
#include <cal3d/tinyxml.h>
#include <cal3d/vector4.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>

using boost::scoped_ptr;
using boost::shared_ptr;
using boost::lexical_cast;
using boost::scoped_array;

inline bool AreClose(
    const CalPoint4& p1,
    const CalPoint4& p2,
    float tolerance
) {
    return (p1.asCalVector4() - p2.asCalVector4()).length() < tolerance;
}

namespace std { // :(
    inline std::ostream& operator<<(std::ostream& os, const CalPoint4& v) {
        return os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
    }
}
