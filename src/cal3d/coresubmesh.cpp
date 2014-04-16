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
#include "cal3d/vector4.h"
#include "cal3d/transform.h"

#include <iostream>
#include <cstring>

CalCoreSubmesh::CalCoreSubmesh(int vertexCount, bool hasTextureCoordinates, int faceCount)
    : coreMaterialThreadId(0)
    , m_currentVertexId(0)
    , m_vertices(vertexCount)
    , m_isStatic(false)
    , m_minimumVertexBufferSize(0)
{
    m_vertexColors.resize(vertexCount);

    if (hasTextureCoordinates) {
        m_textureCoordinates.resize(vertexCount);
    }

    m_faces.reserve(faceCount);
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
    r += ::sizeInBytes(m_faces);
    r += ::sizeInBytes(m_staticInfluenceSet);
    r += ::sizeInBytes(m_influences);
    return r;
}

void CalCoreSubmesh::addFace(const CalCoreSubmesh::Face& face) {
    for (int i = 0; i < 3; ++i) {
        m_minimumVertexBufferSize = std::max(m_minimumVertexBufferSize, size_t(1 + face.vertexId[i]));
    }
    m_faces.push_back(face);
}

void CalCoreSubmesh::setTextureCoordinate(int vertexId, const TextureCoordinate& textureCoordinate) {
    m_textureCoordinates[vertexId] = textureCoordinate;
}

bool CalCoreSubmesh::hasTextureCoordinatesOutside0_1() const {
    for (auto i = m_textureCoordinates.begin(); i != m_textureCoordinates.end(); ++i) {
        if (
            i->u < 0 || i->u > 1 ||
            i->v < 0 || i->v > 1
        ) {
            return true;
        }
    }
    return false;
}

static bool descendingByWeight(const CalCoreSubmesh::Influence& lhs, const CalCoreSubmesh::Influence& rhs) {
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

void CalCoreSubmesh::addMorphTarget(const CalCoreMorphTargetPtr& morphTarget) {
    if (morphTarget->vertexOffsets.size() > 0) {
        m_morphTargets.push_back(morphTarget);
    }
}

void CalCoreSubmesh::replaceMeshWithMorphTarget(const std::string& morphTargetName) {
    for (auto i = m_morphTargets.begin(); i != m_morphTargets.end(); ++i) {
        if ((*i)->name == morphTargetName) {
            const auto& offsets = (*i)->vertexOffsets;
            for (auto o = offsets.begin(); o != offsets.end(); ++o) {
                m_vertices[o->vertexId].position += o->position;
                m_vertices[o->vertexId].normal += o->normal;
            }
        }
    }
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

 
CalCoreSubmeshPtr MakeCube() {
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
    
    cube->addFace(CalCoreSubmesh::Face(0, 1, 2));
    cube->addFace(CalCoreSubmesh::Face(0, 3, 1));




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
    
    cube->addFace(CalCoreSubmesh::Face(4, 5, 6));
    cube->addFace(CalCoreSubmesh::Face(4, 7, 5));



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
    cube->addFace(f4);
    CalCoreSubmesh::Face f5(8, 11, 9);    
    cube->addFace(f5);
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
    cube->addFace(f6);
    CalCoreSubmesh::Face f7(12, 15, 13);    
    cube->addFace(f7);
  
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
    cube->addFace(f8); 
    CalCoreSubmesh::Face f9(16, 19, 17);    
    cube->addFace(f9);
    
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
    cube->addFace(f10);
    CalCoreSubmesh::Face f11(20, 23, 21);    
    cube->addFace(f11);
    return cube;
}

#define PRINT_MOD 200
#define PRINT_STATUS 0

void CalCoreSubmesh::addVertices(CalCoreSubmesh& submeshTo, unsigned submeshToVertexOffset, float normalMul) {
    size_t c = 0;
    size_t i = 0;
    for (size_t v = 0; v < getVertexCount(); v++) {
        const Vertex& oldVert = m_vertices[v];
        Vertex newVert;
        newVert.position = oldVert.position;
        newVert.normal = oldVert.normal;
        newVert.normal *= normalMul;

        InfluenceVector newInfs;
        while (!m_influences[i].lastInfluenceForThisVertex) {
            Influence inf(m_influences[i].boneId, m_influences[i].weight, false);
            newInfs.push_back(inf);
            i++;
        }

        Influence infLast(m_influences[i].boneId, m_influences[i].weight, true);
        newInfs.push_back(infLast);
        i++;

        submeshTo.addVertex(newVert, getVertexColors()[c], newInfs);
        if (getTextureCoordinates().size() > 0) {
            submeshTo.setTextureCoordinate(submeshToVertexOffset + c, getTextureCoordinates()[c]);
        }
        c++;
    }
}

void CalCoreSubmesh::duplicateTriangles() {
    size_t faceCount = m_faces.size();

    std::vector<Face> newFaces;
    newFaces.reserve(2 * faceCount);

    for (size_t i = 0; i < faceCount; ++i) {
        const auto& face = m_faces[i];
        newFaces.push_back(face);
        newFaces.push_back(Face(face.vertexId[0], face.vertexId[2], face.vertexId[1]));
    }

    std::swap(m_faces, newFaces);
}

// http://www.mindcontrol.org/~hplus/graphics/sort-alpha.html
void CalCoreSubmesh::sortTris(CalCoreSubmesh& submeshTo) {
    size_t numVertices = getVertexCount();
    size_t numFaces = m_faces.size();

#if PRINT_STATUS > 0
    std::cout << "\n\nCalCoreSubmesh::sortTris\n" <<
            "Input submesh numVertices: " <<  numVertices <<
            ", numFaces: " << numFaces << "\n" <<
            "Output submesh numVertices: " <<  submeshTo.getVertexCount() <<
            ", numFaces: " << submeshTo.getFaces().size() << "\n";
#endif

    addVertices(submeshTo, 0, 1.f);
    addVertices(submeshTo, numVertices, -1.f);
    for (size_t mt = 0; mt < getMorphTargets().size(); ++mt) {
        const CalCoreMorphTargetPtr& mtPtr = getMorphTargets()[mt];
        CalCoreMorphTarget::VertexOffsetArray voDup;
        for (CalCoreMorphTarget::VertexOffsetArray::const_iterator voi = mtPtr->vertexOffsets.begin(); voi != mtPtr->vertexOffsets.end(); ++voi) {
            voDup.push_back(VertexOffset(voi->vertexId, voi->position, voi->normal));
        }
        for (CalCoreMorphTarget::VertexOffsetArray::const_iterator voi = mtPtr->vertexOffsets.begin(); voi != mtPtr->vertexOffsets.end(); ++voi) {
            voDup.push_back(VertexOffset(voi->vertexId + numVertices, voi->position, voi->normal));
        }
        CalCoreMorphTargetPtr mtPtrDup(new CalCoreMorphTarget(mtPtr->name, numVertices * 2, voDup));
        submeshTo.addMorphTarget(mtPtrDup);
    }
    submeshTo.coreMaterialThreadId = coreMaterialThreadId;

    FaceSort *faces = new FaceSort[numFaces * 2];
    memset(faces, 0, sizeof(FaceSort)*(numFaces * 2));
    FaceSortRef *frefs = new FaceSortRef[numFaces * 2 * numFaces * 2];
    memset(frefs, 0, sizeof(FaceSortRef) * numFaces * 2 * numFaces * 2);
    size_t frtop = 0;

    for (size_t f = 0; f < numFaces; ++f) {
        faces[f].ix[0] = m_faces[f].vertexId[0];
        faces[f].ix[1] = m_faces[f].vertexId[1];
        faces[f].ix[2] = m_faces[f].vertexId[2];

        faces[f].nfront = 0;
        faces[f].infront = 0;

        CalVector a = m_vertices[m_faces[f].vertexId[0]].position.asCalVector();
        CalVector b = m_vertices[m_faces[f].vertexId[1]].position.asCalVector();
        CalVector c = m_vertices[m_faces[f].vertexId[2]].position.asCalVector();
        b = b - a;
        c = c - a;
        a = cross(b, c);

        faces[f].area = a.length() * 0.5f;
        a.normalize();
        faces[f].fnorm = a;

        a = m_vertices[m_faces[f].vertexId[0]].position.asCalVector();
        faces[f].fdist = dot(a, faces[f].fnorm);
        //  delay score calculation
        faces[f].score = 0;
    }

    memcpy(faces + numFaces, faces, sizeof(FaceSort) * numFaces);
    for (size_t f = numFaces; f < numFaces * 2; ++f) {
        unsigned short s = faces[f].ix[0];
        faces[f].ix[0] = (unsigned short)(faces[f].ix[1] + numVertices);
        faces[f].ix[1] = (unsigned short)(s + numVertices);
        faces[f].ix[2] = (unsigned short)(faces[f].ix[2] + numVertices);
        faces[f].nfront = 0;
        faces[f].infront = 0;
        faces[f].fnorm *= -1;
        faces[f].fdist *= -1;
        faces[f].score = 0;
    }

    unsigned totalSearch = 0;
    for (size_t g = 0; g < numFaces * 2; ++g) {
        assert(faces[g].ix[0] != faces[g].ix[1]);
        faces[g].infront = frefs + frtop;

        for (size_t f = 0; f < numFaces * 2; ++f) {
            if (f == g) {
              continue;
            }

            int vfront = 0, gfront = 0;
            for (int i = 0; i < 3; ++i) {
                CalVector v = submeshTo.getVectorVertex()[faces[g].ix[i]].position.asCalVector();
                float d = dot(v, faces[f].fnorm) - faces[f].fdist;
                if (d < -1e-4) {
                  vfront++;
                }

                v = submeshTo.getVectorVertex()[faces[f].ix[i]].position.asCalVector();
                d = dot(v, faces[g].fnorm) - faces[g].fdist;
                if (d < 1e-4) {
                  gfront++;
                }

                totalSearch++;
            }

            if (vfront && (gfront < 3)) {
              FaceSortRef &fr = faces[g].infront[faces[g].nfront];
              fr.face = (unsigned short)f;
              fr.fscore = (float)(std::max(faces[f].area, faces[g].area) * (1 + dot(faces[f].fnorm, faces[g].fnorm)) * 0.5 * vfront / 3);
              faces[g].nfront++;
              faces[f].score += fr.fscore;
              ++frtop;
            }
        }


#if PRINT_STATUS > 0
      if (!(g % PRINT_MOD)) {
          std::cout << "Face " << g << ", num searches so far: " << totalSearch << " (" << (totalSearch / 1000000) << "M)" << "\n";
      }
#endif

    }

    Face newFace;
    for (size_t i = 0; i < numFaces * 2; ++i) {
        float lowest = faces[i].score;
        size_t lowestIx = i;
        for (size_t f = 0; f < numFaces * 2; ++f) {
            if (faces[f].score < lowest) {
                lowest = faces[f].score;
                lowestIx = f;
            }
        }
        assert(lowest < 1e20f);
        assert(faces[lowestIx].ix[0] != faces[lowestIx].ix[1]);
        assert(faces[lowestIx].score == lowest);

        newFace.vertexId[0] = faces[lowestIx].ix[0];
        newFace.vertexId[1] = faces[lowestIx].ix[1];
        newFace.vertexId[2] = faces[lowestIx].ix[2];
        submeshTo.addFace(newFace);

        faces[lowestIx].score = 1e20f;
        for (size_t q = 0; q < faces[lowestIx].nfront; ++q) {
          faces[faces[lowestIx].infront[q].face].score -= faces[lowestIx].infront[q].fscore;
        }

#if PRINT_STATUS > 0
        if (!(i % PRINT_MOD) && (i > 0)) {
            std::cout << "Generated " << i << " faces.\n";
        }
#endif

    }
}

