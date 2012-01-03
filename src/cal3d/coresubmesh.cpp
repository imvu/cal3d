//****************************************************************************//
// coresubmesh.cpp                                                            //
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

#include "cal3d/coreskeleton.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coremorphtarget.h"
#include "cal3d/transform.h"

CalCoreSubmesh::CalCoreSubmesh(int vertexCount, bool hasTextureCoordinates, int faceCount)
    : coreMaterialThreadId(0)
    , m_currentVertexId(0)
    , m_vertices(vertexCount)
    , m_hasVertexColors(false)
    , m_isStatic(false)
{
    m_vertexColors.resize(vertexCount);

    if (hasTextureCoordinates) {
        m_textureCoordinates.resize(vertexCount);
    }

    faces.resize(faceCount);
}

CAL3D_DEFINE_SIZE(CalCoreSubmesh::Face);
CAL3D_DEFINE_SIZE(CalCoreSubmesh::Influence);

size_t sizeInBytes(const CalCoreSubmesh::InfluenceSet& is) {
    return sizeof(is) + sizeInBytes(is.influences);
}

size_t CalCoreSubmesh::sizeInBytes() const {
    size_t r = sizeof(*this);
    r += ::sizeInBytes(m_vertices);
    r += ::sizeInBytes(m_vertexColors);
    r += ::sizeInBytes(faces);
    r += ::sizeInBytes(m_staticInfluenceSet);
    r += ::sizeInBytes(m_influences);
    return r;
}

void CalCoreSubmesh::setTextureCoordinate(int vertexId, const TextureCoordinate& textureCoordinate) {
    m_textureCoordinates[vertexId] = textureCoordinate;
}

bool descendingByWeight(const CalCoreSubmesh::Influence& lhs, const CalCoreSubmesh::Influence& rhs) {
    return lhs.weight > rhs.weight;
}

void CalCoreSubmesh::addVertex(const Vertex& vertex, CalColor32 vertexColor, const std::vector<Influence>& inf_) {
    assert(m_currentVertexId < m_vertices.size());

    const int vertexId = m_currentVertexId++;
    if (vertexId == 0) {
        m_isStatic = true;
        m_staticInfluenceSet = inf_;
        m_boundingVolume.min = vertex.position.asCalVector();
        m_boundingVolume.max = vertex.position.asCalVector();
    } else if (m_isStatic) {
        m_isStatic = m_staticInfluenceSet == inf_;
    }

    if (vertexId) {
        m_boundingVolume.min.x = std::min(m_boundingVolume.min.x, vertex.position.x);
        m_boundingVolume.min.y = std::min(m_boundingVolume.min.y, vertex.position.y);
        m_boundingVolume.min.z = std::min(m_boundingVolume.min.z, vertex.position.z);

        m_boundingVolume.max.x = std::max(m_boundingVolume.max.x, vertex.position.x);
        m_boundingVolume.max.y = std::max(m_boundingVolume.max.y, vertex.position.y);
        m_boundingVolume.max.z = std::max(m_boundingVolume.max.z, vertex.position.z);
    }

    m_vertices[vertexId] = vertex;
    m_vertexColors[vertexId] = vertexColor;

    m_hasVertexColors |= (vertexColor != CalMakeColor(CalVector(1.0f, 1.0f, 1.0f)));

    // Each vertex needs at least one influence.
    std::vector<Influence> inf(inf_);
    if (inf.empty()) {
        m_isStatic = false;
        Influence i;
        i.boneId = 0;
        i.weight = 0.0f;
        inf.push_back(i);
    } else {
        std::sort(inf.begin(), inf.end(), descendingByWeight);
    }

    // Mark the last influence as the last one.  :)
    for (size_t i = 0; i + 1 < inf.size(); ++i) {
        inf[i].lastInfluenceForThisVertex = 0;
    }
    inf[inf.size() - 1].lastInfluenceForThisVertex = 1;

    m_influences.insert(m_influences.end(), inf.begin(), inf.end());
}

void CalCoreSubmesh::addMorphTarget(CalCoreMorphTargetPtr pCoreSubMorphTarget) {
    m_morphTargets.push_back(pCoreSubMorphTarget);
}

const CalCoreSubmesh::MorphTargetArray& CalCoreSubmesh::getMorphTargets() const {
    return m_morphTargets;
}

void CalCoreSubmesh::scale(float factor) {
    // needed because we shouldn't modify the w term
    CalVector4 scaleFactor(factor, factor, factor, 1.0f);

    for (size_t vertexId = 0; vertexId < m_vertices.size(); vertexId++) {
        m_vertices[vertexId].position *= scaleFactor;
    }

    m_boundingVolume.min *= factor;
    m_boundingVolume.max *= factor;

    for (MorphTargetArray::iterator i = m_morphTargets.begin(); i != m_morphTargets.end(); ++i) {
        (*i)->scale(factor);
    }
}

void CalCoreSubmesh::fixup(const CalCoreSkeletonPtr& skeleton) {
    for (size_t i = 0; i < m_influences.size(); ++i) {
        Influence& inf = m_influences[i];
        inf.boneId = (inf.boneId < skeleton->boneIdTranslation.size())
            ? skeleton->boneIdTranslation[inf.boneId]
            : 0;
    }

    std::set<Influence> staticInfluenceSet;

    for (std::set<Influence>::iterator i = m_staticInfluenceSet.influences.begin(); i != m_staticInfluenceSet.influences.end(); ++i) {
        Influence inf = *i;
        inf.boneId = (inf.boneId < skeleton->boneIdTranslation.size())
            ? skeleton->boneIdTranslation[inf.boneId]
            : 0;
        staticInfluenceSet.insert(inf);
    }

    std::swap(m_staticInfluenceSet.influences, staticInfluenceSet);
}

bool CalCoreSubmesh::isStatic() const {
    return m_isStatic && m_morphTargets.empty();
}

BoneTransform CalCoreSubmesh::getStaticTransform(const BoneTransform* bones) const {
    BoneTransform rm;

    std::set<Influence>::const_iterator current = m_staticInfluenceSet.influences.begin();
    while (current != m_staticInfluenceSet.influences.end()) {
        const BoneTransform& influence = bones[current->boneId];
        rm.rowx += current->weight * influence.rowx;
        rm.rowy += current->weight * influence.rowy;
        rm.rowz += current->weight * influence.rowz;

        ++current;
    }

    return rm;
}

/*
          f8, f9
            :
  (0,1,0) __:_____(1,1,0)
         /  :    /|
 (0,1,1)/_______/-|------ f2, f3
        |       | |(1,0, 0)         y
        | f0,f1 | /                 |__ x
        |_______|/                 /
  (0,0,1)   :  (1,0,1)            z
            :
         f10, f11


*/

 
CalCoreSubmeshPtr MakeCube( ) {
    CalCoreSubmeshPtr cube(new CalCoreSubmesh(24, true, 12));
    
    const CalColor32 black = 0;

    
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;

    int curVertexId=0;
    CalCoreSubmesh::TextureCoordinate texCoord;
    CalCoreSubmesh::Vertex vertex;
    //triangle face f0, f1 vertices
    //v0
    curVertexId = 0;
    vertex.position = CalPoint4(0, 0, 1);
    vertex.normal = CalVector4(0, 0, 1);
    cube->addVertex(vertex, black, inf);        
    texCoord.u = 0.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId, texCoord);
    //v1
    ++curVertexId;
    vertex.position = CalPoint4(1,1,1);
    cube->addVertex(vertex, black, inf);           
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;    
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v2
    ++curVertexId;
    vertex.position = CalPoint4(0,1,1);
    cube->addVertex(vertex, black, inf);         
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);    
    //v3
    ++curVertexId;
    vertex.position = CalPoint4(1,0,1);
    cube->addVertex(vertex, black, inf);       
    texCoord.u = 1.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    
    CalCoreSubmesh::Face f0(0, 1, 2);
    cube->faces[0] = f0;
    CalCoreSubmesh::Face f1(0, 3, 1);    
    cube->faces[1] = f1;




    //triangle faces f2, f3 vertices
    //v4
    
    ++curVertexId;
    vertex.position = CalPoint4(1, 0, 1);
    vertex.normal = CalVector4(1, 0, 0);
    cube->addVertex(vertex, black, inf);      
    texCoord.u = 0.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v5
    ++curVertexId;
    vertex.position = CalPoint4(1,1,0);
    cube->addVertex(vertex, black, inf);  
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v6
    ++curVertexId;
    vertex.position = CalPoint4(1,1,1);
    cube->addVertex( vertex, black, inf);  
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v7
    ++curVertexId;
    vertex.position = CalPoint4(1,0,0);
    cube->addVertex(vertex, black, inf);  
    texCoord.u = 1.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    
    CalCoreSubmesh::Face f2(4, 5, 6);
    cube->faces[2] = f2;    
    CalCoreSubmesh::Face f3(4, 7, 5);    
    cube->faces[3] = f3;



    //triangle faces f4, f5 vertices    
    //v8
    ++curVertexId;
    vertex.position = CalPoint4(1, 0, 0);
    vertex.normal = CalVector4(0, 0, -1);
    cube->addVertex(vertex, black, inf);  
    texCoord.u = 0.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v9
    ++curVertexId;
    vertex.position = CalPoint4(0,1,0);
    cube->addVertex(vertex, black, inf);  
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v10
    ++curVertexId;
    vertex.position = CalPoint4(1,1,0);
    cube->addVertex( vertex, black, inf);  
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v11
    ++curVertexId;
    vertex.position = CalPoint4(0,0,0);
    cube->addVertex(vertex, black, inf);  
    texCoord.u = 1.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    
    CalCoreSubmesh::Face f4(8, 9, 10);
    cube->faces[4] = f4;    
    CalCoreSubmesh::Face f5(8, 11, 9);    
    cube->faces[5] = f5;  
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  

    
    //triangle faces f6, f7 vertices    
    //v12
    ++curVertexId;
    vertex.position = CalPoint4(0, 0, 0);
    vertex.normal = CalVector4(-1, 0, 0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v13
    ++curVertexId;
    vertex.position = CalPoint4(0,1,1);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v14
    ++curVertexId;
    vertex.position = CalPoint4(0,1,0);
    cube->addVertex( vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v15
    ++curVertexId;
    vertex.position = CalPoint4(0,0,1);
    cube->addVertex(vertex, black,inf);
    texCoord.u = 1.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  

    CalCoreSubmesh::Face f6(12, 13, 14);
    cube->faces[6] = f6;    
    CalCoreSubmesh::Face f7(12, 15, 13);    
    cube->faces[7] = f7;
  
    //triangle faces f8, f9 vertices       
    //v16    
    ++curVertexId;
    vertex.position = CalPoint4(0, 1, 1);
    vertex.normal = CalVector4(0, 1, 0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v17    
    ++curVertexId;
    vertex.position = CalPoint4(1,1,0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v18
    ++curVertexId;
    vertex.position = CalPoint4(0,1,0);
    cube->addVertex( vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v19
    ++curVertexId;
    vertex.position = CalPoint4(1,1,1);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    
    CalCoreSubmesh::Face f8(16, 17, 18);
    cube->faces[8] = f8; 
    CalCoreSubmesh::Face f9(16, 19, 17);    
    cube->faces[9] = f9;
    
    //triangle faces f10, f11 vertices     
    //v20
    ++curVertexId;
    vertex.position = CalPoint4(1, 0, 0);
    vertex.normal = CalVector4(0, -1, 0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v21
    ++curVertexId;
    vertex.position = CalPoint4(0,0,1);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v22
    ++curVertexId;
    vertex.position = CalPoint4(0,0,0);
    cube->addVertex( vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);  
    //v23
    ++curVertexId;
    vertex.position = CalPoint4(1,0,1);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId, texCoord);  
    
    CalCoreSubmesh::Face f10(20, 21, 22);
    cube->faces[10] = f10;    
    CalCoreSubmesh::Face f11(20, 23, 21);    
    cube->faces[11] = f11;
    return cube;
    }