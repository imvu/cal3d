//****************************************************************************//
// coreskeleton.cpp                                                           //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cal3d/error.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"

int CalCoreSkeleton::addCoreBone(const boost::shared_ptr<CalCoreBone>& pCoreBone) {
  int boneId = coreBones.size();
  coreBones.push_back(pCoreBone);

  if(pCoreBone->parentId == -1)
  {
    rootBoneIds.push_back(boneId);
  }

  mapCoreBoneName(boneId, pCoreBone->name);
  return boneId;
}

void CalCoreSkeleton::calculateState() {
    for (
        std::vector<int>::iterator i = rootBoneIds.begin();
        i != rootBoneIds.end();
        ++i
    ) {
        coreBones[*i]->calculateState(this);
    }
}

CalCoreBone* CalCoreSkeleton::getCoreBone(const std::string& strName) {
   return coreBones[getCoreBoneId(strName)].get();
}

int CalCoreSkeleton::getCoreBoneId(const std::string& strName)
{
  //Check to make sure the mapping exists
  if (m_mapCoreBoneNames.count(strName) <= 0)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return -1;
  }

  return m_mapCoreBoneNames[strName];

}

bool CalCoreSkeleton::mapCoreBoneName(int coreBoneId, const std::string& strName)
{
   //Make sure the ID given is a valid corebone ID number
   if ((coreBoneId < 0) || (coreBoneId >= (int)coreBones.size()))
   {
      return false;
   }

   //Add the mapping or overwrite an existing mapping
   m_mapCoreBoneNames[strName] = coreBoneId;

   return true;
}

void CalCoreSkeleton::scale(float factor) {
  for (
      std::vector<int>::const_iterator i = rootBoneIds.begin();
      i != rootBoneIds.end();
      ++i
  ) {
      coreBones[*i]->scale(factor, this);
  }
}
