//****************************************************************************//
// coremesh.h                                                                 //
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

class CalCoreSubmesh;

class CAL3D_API CalCoreMesh : public Cal::Object
{
public:
  typedef std::vector<boost::shared_ptr<CalCoreSubmesh> > CalCoreSubmeshVector;

  size_t size() const;
  int addCoreSubmesh(const boost::shared_ptr<CalCoreSubmesh>& pCoreSubmesh);
  const boost::shared_ptr<CalCoreSubmesh>& getCoreSubmesh(int id) const {
      return m_vectorCoreSubmesh[id];
  }
  size_t getCoreSubmeshCount() const {
      return m_vectorCoreSubmesh.size();
  }
  CalCoreSubmeshVector& getVectorCoreSubmesh();
  int addAsMorphTarget(CalCoreMesh *pCoreMesh, std::string const & morphTargetName);
  void scale(float factor);

  CalCoreSubmeshVector m_vectorCoreSubmesh;
};
