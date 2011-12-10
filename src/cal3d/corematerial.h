//****************************************************************************//
// corematerial.h                                                             //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

class CAL3D_API CalCoreMaterial {
public:
    struct Map {
        std::string filename;
        std::string type;

        bool operator==(const Map& rhs) const {
            return filename == rhs.filename && type == rhs.type;
        }
    };

    std::vector<Map> maps;

    bool getTwoSided() const {
        return maps.size() > 1;    // Should come from check box.
    }
};
CAL3D_PTR(CalCoreMaterial);
