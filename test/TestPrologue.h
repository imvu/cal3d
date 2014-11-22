#pragma once

// The old version of clang currently used on the Mac builder requires some
// operator<<() declarations to precede their use in the UnitTest++
// templates/macros. -- jlee - 2014-11-21
#include <cal3d/streamops.h>

#include <TestFramework/TestFramework.h>
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
