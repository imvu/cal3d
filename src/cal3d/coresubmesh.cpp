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
#include "cal3d/forsythtriangleorderoptimizer.h"
#include "cal3d/trisort.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>

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

bool CalCoreSubmesh::validateSubmesh() const {
    size_t vertexCount = m_vertices.size(); 
    size_t numFaces = m_faces.size();
    for (size_t f = 0; f < numFaces; ++f) {
        if (m_faces[f].vertexId[0] >= vertexCount || m_faces[f].vertexId[1] >= vertexCount || m_faces[f].vertexId[2] >= vertexCount) {
            return false;
        }
    }
    return true;
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

 
CalCoreSubmeshPtr MakeCubeScale(float scale) {
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
    vertex.position = CalPoint4(0, 0, scale);
    vertex.normal = CalVector4(0, 0, 1);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId, texCoord);
    //v1
    ++curVertexId;
    vertex.position = CalPoint4(scale,scale,scale);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v2
    ++curVertexId;
    vertex.position = CalPoint4(0,scale,scale);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v3
    ++curVertexId;
    vertex.position = CalPoint4(scale,0,scale);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 1.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    
    cube->addFace(CalCoreSubmesh::Face(0, 1, 2));
    cube->addFace(CalCoreSubmesh::Face(0, 3, 1));
    
    //triangle faces f2, f3 vertices
    //v4
    
    ++curVertexId;
    vertex.position = CalPoint4(scale, 0, scale);
    vertex.normal = CalVector4(1, 0, 0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v5
    ++curVertexId;
    vertex.position = CalPoint4(scale,scale,0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v6
    ++curVertexId;
    vertex.position = CalPoint4(scale,scale,scale);
    cube->addVertex( vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v7
    ++curVertexId;
    vertex.position = CalPoint4(scale,0,0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 1.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    
    cube->addFace(CalCoreSubmesh::Face(4, 5, 6));
    cube->addFace(CalCoreSubmesh::Face(4, 7, 5));
    
    //triangle faces f4, f5 vertices
    //v8
    ++curVertexId;
    vertex.position = CalPoint4(scale, 0, 0);
    vertex.normal = CalVector4(0, 0, -1);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v9
    ++curVertexId;
    vertex.position = CalPoint4(0,scale,0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v10
    ++curVertexId;
    vertex.position = CalPoint4(scale,scale,0);
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
    vertex.position = CalPoint4(0,scale,scale);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v14
    ++curVertexId;
    vertex.position = CalPoint4(0,scale,0);
    cube->addVertex( vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v15
    ++curVertexId;
    vertex.position = CalPoint4(0,0,scale);
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
    vertex.position = CalPoint4(0, scale, scale);
    vertex.normal = CalVector4(0, 1, 0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v17
    ++curVertexId;
    vertex.position = CalPoint4(scale,scale,0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v18
    ++curVertexId;
    vertex.position = CalPoint4(0,scale,0);
    cube->addVertex( vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v19
    ++curVertexId;
    vertex.position = CalPoint4(scale,scale,scale);
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
    vertex.position = CalPoint4(scale, 0, 0);
    vertex.normal = CalVector4(0, -1, 0);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v21
    ++curVertexId;
    vertex.position = CalPoint4(0,0,scale);
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
    vertex.position = CalPoint4(scale,0,scale);
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

CalCoreSubmeshPtr MakeCube() {
    return MakeCubeScale(1.0f);
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

struct FaceSortRef {
    float fscore;
    unsigned short face;
};

struct FaceSort {
    unsigned short ix[3];       //  the face
    unsigned short nfront;      //  how many faces I am in front of
    FaceSortRef *infront;       //  refs to faces I am in front of, with scores
    CalVector fnorm;            //  the face normal
    float fdist;                //  the face distance from origin in the direction of the normal
    float score;                //  the current cost of this face (sum of other faces behind)
    float area;
};

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

typedef std::vector<std::vector<CalCoreSubmesh::Influence>> InfluenceVectorVector;

static InfluenceVectorVector extractInfluenceVector(const CalCoreSubmesh::InfluenceVector& iv) {
    InfluenceVectorVector result;
    std::vector<CalCoreSubmesh::Influence> current;
    for (size_t i = 0; i < iv.size(); ++i) {
        auto& inf = iv[i];
        current.push_back(inf);
        if (inf.lastInfluenceForThisVertex) {
            result.push_back(current);
            current.clear();
        }
    }
    return result;
}
static CalCoreSubmesh::InfluenceVector generateInfluenceVector(const InfluenceVectorVector& iv) {
    CalCoreSubmesh::InfluenceVector result;
    for (auto influences = iv.begin(); influences != iv.end(); ++influences) {
        for (size_t i = 0; i + 1 < influences->size(); ++i) {
            auto j = (*influences)[i];
            j.lastInfluenceForThisVertex = false;
            result.push_back(j);
        }
        auto j = influences->back();
        j.lastInfluenceForThisVertex = true;
        result.push_back(j);
    }
    return result;
}

CalCoreSubmeshPtr CalCoreSubmesh::emitSubmesh(VerticesSet & verticesSetThisSplit, VectorFace & trianglesThisSplit, SplitMeshBasedOnBoneLimitType& rc) {
    auto influencesVector = extractInfluenceVector(m_influences);

    typedef std::map<int, int> VertexMap;
    VertexMap vertexMapper;
    int vIdx = 0;
    int numTris;
    for (std::set<int>::iterator it = verticesSetThisSplit.begin(); it != verticesSetThisSplit.end(); ++it) {
        vertexMapper[*it] = vIdx++;
    }
    numTris = trianglesThisSplit.size();
    for (int x = 0; x < numTris; ++x) {
        if (verticesSetThisSplit.find(trianglesThisSplit[x].vertexId[0]) == verticesSetThisSplit.end()
            || verticesSetThisSplit.find(trianglesThisSplit[x].vertexId[1]) == verticesSetThisSplit.end()
            || verticesSetThisSplit.find(trianglesThisSplit[x].vertexId[2]) == verticesSetThisSplit.end()) {
            rc = SplitMeshBoneLimitVtxTrglMismatch;
            CalCoreSubmeshPtr newSubmesh;
            return newSubmesh;
        }
        trianglesThisSplit[x].vertexId[0] = vertexMapper[trianglesThisSplit[x].vertexId[0]];
        trianglesThisSplit[x].vertexId[1] = vertexMapper[trianglesThisSplit[x].vertexId[1]];
        trianglesThisSplit[x].vertexId[2] = vertexMapper[trianglesThisSplit[x].vertexId[2]];
    }

    CalCoreSubmeshPtr newSubmesh(new CalCoreSubmesh(verticesSetThisSplit.size(), m_textureCoordinates.size() > 0, numTris)); 

    vIdx = 0;
    for (auto it = verticesSetThisSplit.begin(); it != verticesSetThisSplit.end(); ++it) {
        int v = *it;
        struct Vertex originalVertex = m_vertices[v];
        newSubmesh->addVertex(originalVertex, m_vertexColors[v], influencesVector[v]);
        newSubmesh->setTextureCoordinate(vIdx, m_textureCoordinates[v]);
        ++vIdx;
    }
    for (int x = 0; x < numTris; ++x) {
        newSubmesh->addFace(trianglesThisSplit[x]);
    }
    newSubmesh->coreMaterialThreadId = coreMaterialThreadId;
    rc = SplitMeshBoneLimitOK;
    return newSubmesh;
}

static void getBoneIndicesFromFace(InfluenceVectorVector& influences, std::set<int>& bSet, CalCoreSubmesh::Face& t) {
    for (int i = 0; i < 3; ++i) {
        auto& iv = influences[t.vertexId[i]];
        for (size_t indx = 0; indx < iv.size(); ++indx) {
            bSet.insert(iv[indx].boneId);
        }
    }
}

SplitMeshBasedOnBoneLimitType CalCoreSubmesh::splitMeshBasedOnBoneLimit(CalCoreSubmeshPtrVector& newSubmeshes, size_t boneLimit) {
    std::set<int> boneIndicesThisMesh_New;
    std::set<int> boneIndicesTemp;
    std::set<int> boneIndicesThisMesh;
    VerticesSet verticesForThisSplit;
    std::vector<Face> trianglesForThisSplit;
    SplitMeshBasedOnBoneLimitType rc;
    CalCoreSubmeshPtr newSubmeshSP;

    auto influencesVector = extractInfluenceVector(m_influences);

    VectorFace::size_type sz = m_faces.size();
    for (unsigned i = 0; i < sz; i++) {
        Face& t = m_faces[i];
        getBoneIndicesFromFace(influencesVector, boneIndicesTemp, t);
        boneIndicesThisMesh_New.insert(boneIndicesTemp.begin(), boneIndicesTemp.end());

        if (boneIndicesThisMesh_New.size() > boneLimit) {
            newSubmeshSP = emitSubmesh(verticesForThisSplit, trianglesForThisSplit, rc);
            if (rc != SplitMeshBoneLimitOK) {
                return rc;
            }
            boneIndicesThisMesh_New.clear();
            boneIndicesThisMesh_New.insert(boneIndicesTemp.begin(), boneIndicesTemp.end());
            verticesForThisSplit.clear();
            trianglesForThisSplit.clear();
            newSubmeshes.push_back(newSubmeshSP);
        }
        verticesForThisSplit.insert(t.vertexId[0]);
        verticesForThisSplit.insert(t.vertexId[1]);
        verticesForThisSplit.insert(t.vertexId[2]);
        trianglesForThisSplit.push_back(t);
        boneIndicesTemp.clear();
    }
    if (!trianglesForThisSplit.empty()) {
        if (verticesForThisSplit.empty()) {
            return SplitMeshBoneLimitEmptyVertices;
        }
        newSubmeshSP = emitSubmesh(verticesForThisSplit, trianglesForThisSplit, rc);
        if (rc != SplitMeshBoneLimitOK) {
            return rc;
        }
        verticesForThisSplit.clear();
        trianglesForThisSplit.clear();
        boneIndicesThisMesh_New.clear();
        newSubmeshes.push_back(newSubmeshSP);
    }

    return SplitMeshBoneLimitOK;
}

void CalCoreSubmesh::optimizeVertexCache() {
    if (m_faces.empty()) {
        return;
    }

    std::vector<Face> newFaces(m_faces.size());

    Forsyth::OptimizeFaces(
        m_faces[0].vertexId,
        3 * m_faces.size(),
        m_vertices.size(),
        newFaces[0].vertexId,
        32);

    m_faces.swap(newFaces);
}

void CalCoreSubmesh::optimizeVertexCacheSubset(
    unsigned int faceStartIndex,
    unsigned int faceCount
) {
    if (m_faces.empty()) {
        return;
    }
    assert(faceStartIndex + faceCount <= m_faces.size());

    std::vector<Face> newFaces(faceCount);

    Forsyth::OptimizeFaces(
        m_faces[faceStartIndex].vertexId,
        3 * faceCount,
        m_vertices.size(),
        newFaces[0].vertexId,
        32);

    // Without these temp variable castings, MSVC 2010 will insert bounds
    // asserts into std::copy() in debug builds, to keep us from abusing our
    // fixed length arrays.  Presumably this will be better in C++11.
    const CalIndex* srcStart = &(newFaces[0].vertexId[0]);
    const CalIndex* srcEnd = srcStart + faceCount * 3;
    CalIndex* destStart = &(m_faces[faceStartIndex].vertexId[0]);
    std::copy(srcStart, srcEnd, destStart);
}

void CalCoreSubmesh::renumberIndices() {
    // just because operator[] is invalid on an empty vector. in C++11
    // we could use .data().
    if (m_faces.empty()) {
        return;
    }

    std::vector<Face> newFaces(m_faces.size());
    size_t indexCount = m_faces.size() * 3;

    std::vector<CalIndex> mapping(m_vertices.size(), -1); // old -> new
    std::vector<bool> populated(m_vertices.size(), false); // used index
    const CalIndex* oldIndices = m_faces[0].vertexId;
    CalIndex* newIndices = newFaces[0].vertexId;
    unsigned int outputVertexCount = 0;

    for (size_t i = 0; i < indexCount; ++i) {
        CalIndex oldIndex = oldIndices[i];
        CalIndex newIndex = mapping[oldIndex];
        if (!populated[oldIndex]) {
            populated[oldIndex] = true;
            mapping[oldIndex] = static_cast<CalIndex> (outputVertexCount);
            newIndex = static_cast<CalIndex> (outputVertexCount);
            outputVertexCount++;
        }
        *newIndices = newIndex;
        newIndices++;
    }

    m_faces.swap(newFaces);

    // now that the new indices are in place, reorder the vertices

    auto oldInfluences = extractInfluenceVector(m_influences);
    assert(m_vertices.size() == m_vertexColors.size());
    assert(m_vertices.size() == oldInfluences.size());

    VectorVertex newVertices(outputVertexCount);
    std::vector<CalColor32> newColors(outputVertexCount);
    InfluenceVectorVector newInfluences(outputVertexCount);
    VectorTextureCoordinate newTexCoords(hasTextureCoordinates() ? outputVertexCount : 0); // may be empty

    for (size_t oldIndex = 0; oldIndex < mapping.size(); ++oldIndex) {
        CalIndex newIndex = mapping[oldIndex];
        if (!populated[oldIndex]) {
            continue;
        }

        newVertices[newIndex] = m_vertices[oldIndex];
        newColors[newIndex] = m_vertexColors[oldIndex];
        newInfluences[newIndex] = oldInfluences[oldIndex];
        if (!newTexCoords.empty()) {
            newTexCoords[newIndex] = m_textureCoordinates[oldIndex];
        }
    }

    MorphTargetArray newMorphTargets;

    for (size_t i = 0; i < m_morphTargets.size(); ++i) {
        const auto& mt = m_morphTargets[i];
        CalCoreMorphTarget::VertexOffsetArray newOffsets;
        for (auto vo = mt->vertexOffsets.begin(); vo != mt->vertexOffsets.end(); ++vo) {
            CalIndex newIndex = mapping[vo->vertexId];
            if (populated[vo->vertexId]) {
                newOffsets.push_back(VertexOffset(newIndex, vo->position, vo->normal));
            }
        }
        newMorphTargets.push_back(CalCoreMorphTargetPtr(new CalCoreMorphTarget(mt->name, newVertices.size(), newOffsets)));
    }

    m_vertices.swap(newVertices);
    m_vertexColors.swap(newColors);
    m_influences = generateInfluenceVector(newInfluences);
    m_textureCoordinates.swap(newTexCoords);
    m_morphTargets.swap(newMorphTargets);

    m_minimumVertexBufferSize = outputVertexCount;
}

void CalCoreSubmesh::normalizeNormals() {
    const float inf = std::numeric_limits<float>::infinity();
    size_t numVertices = m_vertices.size();
    for (size_t i = 0; i < numVertices; ++i) {
        CalVector4& n = m_vertices[i].normal;
        float length = n.length();
        if (length > 0.0001f && length != inf) {
            n *= 1.0f / length;
        } else {
            // We pick positive Y just because it's "up" in Northstar.  The
            // caller really shouldn't care what value gets returned, since
            // they gave us garbage.
            n = CalVector4(0.0f, 1.0f, 0.0f, 0.0f);
        }
    }
}

void CalCoreSubmesh::sortForBlending() {
    if (m_faces.empty()) {
        return;
    }

    sortTrianglesBackToFront(
        m_faces.size(),
        m_faces[0].vertexId,
        &m_vertices[0].position.x,
        sizeof(Vertex) / sizeof(float));
}

CalExportedInfluences CalCoreSubmesh::exportInfluences(unsigned int influenceLimit) {
    CalExportedInfluences outInfluenceData;

    // NOTE: May be a bit more efficient to use a sorted vector, hash map, or
    // array of bools or something.
    std::set<unsigned int> usedBoneIds;

    unsigned int maxInfluenceCount = 0;
    CalWeightsBoneIdsPair pair;
    for (auto it = m_influences.begin(); it != m_influences.end(); ++it) {
        const auto& influence = *it;

        if (pair.weights.size() < influenceLimit) {
            pair.weights.push_back(influence.weight);
            pair.boneIds.push_back(influence.boneId);
            usedBoneIds.insert(influence.boneId);
        }

        if (influence.lastInfluenceForThisVertex) {
            unsigned int influenceCount = pair.weights.size();
            maxInfluenceCount = std::max(maxInfluenceCount, influenceCount);

            for (unsigned int i = pair.weights.size(); i < influenceLimit; ++i) {
                pair.weights.push_back(0);
                pair.boneIds.push_back(0);
            }

            float total = 0;
            for (unsigned int i = 0; i < pair.weights.size(); ++i) {
                total += pair.weights[i];
            }
            if (std::fabs(total - 1.0) >= 0.001 && std::fabs(total) >= 0.001) {
                for (unsigned int i = 0; i < pair.weights.size(); ++i) {
                    pair.weights[i] /= total;
                }
            }

            outInfluenceData.weightsBoneIdsPairs.push_back(pair);
            pair = CalWeightsBoneIdsPair();
        }
    }
    outInfluenceData.maximumInfluenceCount = maxInfluenceCount;

    if (m_vertices.size() != outInfluenceData.weightsBoneIdsPairs.size()) {
        throw std::runtime_error("Influence count must match vertex count");
    }
    outInfluenceData.usedBoneIds = std::vector<unsigned int>(usedBoneIds.begin(), usedBoneIds.end());

    return outInfluenceData;
}

/*
 *  Mesh Polygon Reduction Algorithm from Progressive Mesh code
 *  by Stan Melax (c) 1998
 *  Permission to use any of this code wherever you want is granted..
 *  Although, please do acknowledge authorship if appropriate.
 *
 *  See the header file progmesh.h for a description of this module
 *  Ported to STL & Cal3D by IMVU (c) 2014 - progressive permutation
 *  removed.
 */

/*
 *  For the polygon reduction algorithm we use data structures
 *  that contain a little bit more information than the usual
 *  indexed face set type of data structure.
 *  From a vertex we wish to be able to quickly get the
 *  neighbouring faces and vertices.
 */

CalCoreSubmesh::Triangle::Triangle(std::vector<reduxVertex *> &vertices, std::vector<Triangle *> &triangles, reduxVertex *v0, reduxVertex *v1, reduxVertex *v2)
    : parent_vertices(vertices)
    , parent_triangles(triangles)
{
    vertex[0] = v0;
    vertex[1] = v1;
    vertex[2] = v2;
    ComputeNormal();
    ComputeTB();
    for (int i = 0; i<3; i++) {
        vertex[i]->face.push_back(this);
        for (int j = 0; j<3; j++) if (i != j) {
            VECTOR_ADD_UNIQUE(vertex[i]->neighbor, vertex[j]);
        }
    }
}

CalCoreSubmesh::Triangle::~Triangle(){
    int i;
    VECTOR_REMOVE_VALUE(parent_triangles, this);
    for (i = 0; i<3; i++) {
        if (vertex[i])
        {
            VECTOR_REMOVE_VALUE(vertex[i]->face, this);
        }
    }
    for (i = 0; i<3; i++) {
        int i2 = (i + 1) % 3;
        if (!vertex[i] || !vertex[i2]) continue;
        vertex[i]->RemoveIfNonNeighbor(vertex[i2]);
        vertex[i2]->RemoveIfNonNeighbor(vertex[i]);
    }
}
int CalCoreSubmesh::Triangle::HasVertex(reduxVertex *v) {
    return (v == vertex[0] || v == vertex[1] || v == vertex[2]);
}
void CalCoreSubmesh::Triangle::ComputeNormal(){
    CalVector v0 = vertex[0]->position;
    CalVector v1 = vertex[1]->position;
    CalVector v2 = vertex[2]->position;
    normal = cross((v1 - v0), (v2 - v1));
    normal.normalize();
}
void CalCoreSubmesh::Triangle::ComputeTB(){
    //Calculate Tangent and Bitangent (AKA binormal)

    float dx1 = vertex[1]->position.x - vertex[0]->position.x;
    float dx2 = vertex[2]->position.x - vertex[0]->position.x;
    float dy1 = vertex[1]->position.y - vertex[0]->position.y;
    float dy2 = vertex[2]->position.y - vertex[0]->position.y;
    float dz1 = vertex[1]->position.z - vertex[0]->position.z;
    float dz2 = vertex[2]->position.z - vertex[0]->position.z;
    
    float du1 = vertex[1]->u - vertex[0]->u;
    float du2 = vertex[2]->u - vertex[0]->u;
    float dv1 = vertex[1]->u - vertex[0]->v;
    float dv2 = vertex[2]->v - vertex[0]->v;
        
    float recip = 1.0f / (du1 * dv2 - du2 * dv1);
    T = CalVector((dv2 * dx1 - dv1 * dx2) * recip, (dv2 * dy1 - dv1 * dy2) * recip, (dv2 * dz1 - dv1 * dz2) * recip);
    B = CalVector((du1 * dx2 - du2 * dx1) * recip, (du1 * dy2 - du2 * dy1) * recip, (du1 * dz2 - du2 * dz1) * recip);
    // this means dot product later only accounts for angular changes
    // in texture derivative, that'll do for now
    T.normalize();
    B.normalize();
}
void CalCoreSubmesh::Triangle::ReplaceVertex(reduxVertex *vold, reduxVertex *vnew) {
    assert(vold && vnew);
    assert(vold == vertex[0] || vold == vertex[1] || vold == vertex[2]);
    assert(vnew != vertex[0] && vnew != vertex[1] && vnew != vertex[2]);
    if (vold == vertex[0]){
        vertex[0] = vnew;
    }
    else if (vold == vertex[1]){
        vertex[1] = vnew;
    }
    else {
        assert(vold == vertex[2]);
        vertex[2] = vnew;
    }
    int i;
    VECTOR_REMOVE_VALUE(vold->face, this);
    assert(!VECTOR_CONTAINS(vnew->face, this));
    vnew->face.push_back(this);
    for (i = 0; i<3; i++) {
        vold->RemoveIfNonNeighbor(vertex[i]);
        vertex[i]->RemoveIfNonNeighbor(vold);
    }
    for (i = 0; i<3; i++) {
        assert(VECTOR_CONTAINS(vertex[i]->face, this));
        for (int j = 0; j<3; j++) if (i != j) {
            VECTOR_ADD_UNIQUE(vertex[i]->neighbor, vertex[j]);
        }
    }
    ComputeNormal();
    ComputeTB();
}

CalCoreSubmesh::reduxVertex::reduxVertex(std::vector<reduxVertex *> &vertices, const CalPoint4 &p, int _id)
    : parent_vertices(vertices)
{
    position.x = p.x;
    position.y = p.y;
    position.z = p.z;
    u = v = 0.0f;
    id = _id;
}
CalCoreSubmesh::reduxVertex::reduxVertex(std::vector<reduxVertex *> &vertices, const CalPoint4 &p, float _u, float _v, int _id)
    : parent_vertices(vertices)
{
    position.x = p.x;
    position.y = p.y;
    position.z = p.z;
    u = _u;
    v = _v;
    id = _id;
}

CalCoreSubmesh::reduxVertex::~reduxVertex(){
    assert(face.size() == 0);
    while (neighbor.size()) {
        VECTOR_REMOVE_VALUE(neighbor[0]->neighbor, this);
        VECTOR_REMOVE_VALUE(neighbor, neighbor[0]);
    }
    VECTOR_REMOVE_VALUE(parent_vertices, this);
}

bool CalCoreSubmesh::reduxVertex::isBorder() {
    unsigned int i, j;
    
    for(i=0; i< neighbor.size(); i++) {
        int count = 0;
        for(j=0; j< face.size(); j++) {
            if(face[j]->HasVertex(neighbor[i])) {
                count++;
            }
        }
        assert(count > 0);
        if(count==1) {
            return true;
        }
    }
    return false;
}

void CalCoreSubmesh::reduxVertex::RemoveIfNonNeighbor(reduxVertex *n) {
    // removes n from neighbor list if n isn't a neighbor.
    if (!VECTOR_CONTAINS(neighbor, n)) return;
    for (unsigned int i = 0; i<face.size(); i++) {
        if (face[i]->HasVertex(n)) return;
    }
    VECTOR_REMOVE_VALUE(neighbor, n);
}

float CalCoreSubmesh::ComputeEdgeCollapseCost(reduxVertex *u, reduxVertex *v) {
    // if we collapse edge uv by moving u to v then how 
    // much different will the model change, i.e. how much "error".
    // Texture, vertex normal, and border vertex code was removed
    // to keep this demo as simple as possible.
    // The method of determining cost was designed in order 
    // to exploit small and coplanar regions for
    // effective polygon reduction.
    // Is is possible to add some checks here to see if "folds"
    // would be generated.  i.e. normal of a remaining face gets
    // flipped.  I never seemed to run into this problem and
    // therefore never added code to detect this case.
    unsigned int i;
    float edgelength = (v->position - u->position).length();
    float curvature = 0;

    // find the "sides" triangles that are on the edge uv
    std::vector<Triangle *> sides;
    for (i = 0; i<u->face.size(); i++) {
        if (u->face[i]->HasVertex(v)){
            sides.push_back(u->face[i]);
        }
    }

    // use the triangle facing most away from the sides 
    // to determine our curvature term
    for (i = 0; i<u->face.size(); i++) {
        float mincurv = 1.0f; // curve for face i and closer side to it
        for (unsigned int j = 0; j<sides.size(); j++) {
            // use dot product of face normals.
            float dotprod = dot(u->face[i]->normal, sides[j]->normal);
            float dotprodT = dot(u->face[i]->T, sides[j]->T);
            float dotprodB = dot(u->face[i]->B, sides[j]->B);
            mincurv = std::min(mincurv, (1.0f - dotprodT) / 8.0f + (1.0f - dotprodB) / 8.0f + (1.0f - dotprod) / 2.0f);
        }
        curvature = std::max(curvature, mincurv);
    }
    
    // the more coplanar the lower the curvature term 
    // but adjust for border adjacency
    bool uedge = u->isBorder();
    bool vedge = v->isBorder();
    if(u->face.size() == 1) {
        curvature += 5.0;
        m_isolateds++;
    }
    else if (u->face.size() == 2) {
        if( matchFaces(u->face[0], u->face[1]) ) {
            curvature += 5.0;
            m_isolateds++;
        }
    }
    if(uedge) {
        curvature += 4.5;
        if(vedge) {
            curvature -= 2.75;
        }
    }
    return edgelength * curvature;
}

bool CalCoreSubmesh::matchFaces(Triangle *t0, Triangle *t1)
{
    if( (t0->vertex[0] == t1->vertex[0] || t0->vertex[0] == t1->vertex[1] || t0->vertex[0] == t1->vertex[2]) &&
        (t0->vertex[1] == t1->vertex[0] || t0->vertex[1] == t1->vertex[1] || t0->vertex[1] == t1->vertex[2]) &&
        (t0->vertex[2] == t1->vertex[0] || t0->vertex[2] == t1->vertex[1] || t0->vertex[2] == t1->vertex[2]) ) {
        return true;
    }
    return false;
}

bool CalCoreSubmesh::ComputeEdgeCostAtVertex(reduxVertex *v) {
    // compute the edge collapse cost for all edges that start
    // from vertex v.  Since we are only interested in reducing
    // the object by selecting the min cost edge at each step, we
    // only cache the cost of the least cost edge at this vertex
    // (in member variable collapse) as well as the value of the 
    // cost (in member variable objdist).
    if (v->neighbor.size() == 0) {
        // v doesn't have neighbors so it costs nothing to collapse
        v->collapse = NULL;
        v->objdist = -0.01f;
        return true;
    }
    v->objdist = 1000000;
    v->collapse = NULL;
    if(v->neighbor.size() > 25) {
        return false; // abort
    }
    // search all neighboring edges for "least cost" edge
    for (unsigned int i = 0; i<v->neighbor.size(); i++) {
        float dist;
        dist = ComputeEdgeCollapseCost(v, v->neighbor[i]);
        if (dist<v->objdist) {
            v->collapse = v->neighbor[i];  // candidate for edge collapse
            v->objdist = dist;             // cost of the collapse
        }
    }
    return true;
}

bool CalCoreSubmesh::ComputeAllEdgeCollapseCosts() {
    // For all the edges, compute the difference it would make
    // to the model if it was collapsed.  The least of these
    // per vertex is cached in each vertex object.
    for (unsigned int i = 0; i<vertices.size(); i++) {
        if(! ComputeEdgeCostAtVertex(vertices[i])) {
            return false; // abort
        }
    }
    return true;
}

void CalCoreSubmesh::Collapse(reduxVertex *u, reduxVertex *v){
    // Collapse the edge uv by moving vertex u onto v
    // Actually remove tris on uv, then update tris that
    // have u to have v, and then remove u.
    if (!v) {
        // u is a vertex all by itself so just delete it
        delete u;
        return;
    }
    int i;
    std::vector<reduxVertex *>tmp;
    // make tmp a list of all the neighbors of u
    for (i = 0; i<static_cast<int>(u->neighbor.size()); i++) {
        tmp.push_back(u->neighbor[i]);
    }
    // delete triangles on edge uv:
    for (i = static_cast<int>(u->face.size()) - 1; i >= 0; i--) {
        if (u->face[i]->HasVertex(v)) {
            delete(u->face[i]);
        }
    }
    // update remaining triangles to have v instead of u
    for (i = static_cast<int>(u->face.size()) - 1; i >= 0; i--) {
        u->face[i]->ReplaceVertex(u, v);
    }
    delete u;
    // recompute the edge collapse costs for neighboring vertices
    for (i = 0; i<static_cast<int>(tmp.size()); i++) {
        ComputeEdgeCostAtVertex(tmp[i]);
    }
}

void CalCoreSubmesh::reduxAddVertices(const CalCoreSubmesh::VectorVertex &vert, const CalCoreSubmesh::VectorTextureCoordinate &tex){
    if(tex.size() == vert.size()) {
        for (unsigned int i = 0; i<vert.size(); i++) {
            vertices.push_back(new reduxVertex(vertices, vert[i].position, tex[i].u, tex[i].v, i));
        }
    }
    else
    { 
        for (unsigned int i = 0; i<vert.size(); i++) {
            vertices.push_back(new reduxVertex(vertices, vert[i].position, i));
        }
    }
}

void CalCoreSubmesh::reduxAddFaces(const CalCoreSubmesh::VectorFace &tri){
    for (unsigned int i = 0; i<tri.size(); i++) {
        if(!(tri[i].vertexId[0] == tri[i].vertexId[1] ||
             tri[i].vertexId[1] == tri[i].vertexId[2] ||
             tri[i].vertexId[2] == tri[i].vertexId[0])) {
                    
                triangles.push_back(
                    new Triangle(vertices, triangles,
                        vertices[tri[i].vertexId[0]],
                        vertices[tri[i].vertexId[1]],
                        vertices[tri[i].vertexId[2]]));
        }
    }
}

void CalCoreSubmesh::ApplyFaces(const CalCoreSubmesh::VectorFace &tris){
    int gone = m_faces.size() - tris.size();
    if(gone >0){
        m_faces.clear();
        
        //m_faces.push_back(CalCoreSubmesh::Face(0, 1, 2));
        for (unsigned int i = 0; i<tris.size(); i++) {
            m_faces.push_back(tris[i]);
        }
    }
}

CalCoreSubmesh::reduxVertex *CalCoreSubmesh::MinimumCostEdge(){
    // Find the edge that when collapsed will affect model the least.
    // This funtion actually returns a reduxVertex, the second vertex
    // of the edge (collapse candidate) is stored in the vertex data.
    // Serious optimization opportunity here: this function currently
    // does a sequential search through an unsorted list :-(
    // Our algorithm could be O(n*lg(n)) instead of O(n*n)
    CalCoreSubmesh::reduxVertex *mn = vertices[0];
    for (unsigned int i = 0; i<vertices.size(); i++) {
        if (vertices[i]->objdist < mn->objdist) {
            mn = vertices[i];
        }
    }
    return mn;
}

bool CalCoreSubmesh::simplifySubmesh(unsigned int target_tri_count, unsigned int quality) {
    bool retval = false;
    if(target_tri_count < m_faces.size()) {
        vertices.clear();
        triangles.clear();
        reduxAddVertices(getVectorVertex(), getTextureCoordinates());  // put input data into our data structures
        reduxAddFaces(getFaces());
        float lastError, error;
        m_isolateds = 0;
        
        lastError = error = (100.0f-quality ) * 0.00001f;

        if (ComputeAllEdgeCollapseCosts()) { // cache all edge collapse costs
            if(m_isolateds / static_cast<float>(m_faces.size()) < 0.4f) {
                if(m_isolateds / static_cast<float>(m_faces.size()) > 0.21f) { // boost poly count if isolateds are high
                    target_tri_count += ( m_faces.size() - target_tri_count) /2;
                }
                while (triangles.size() > target_tri_count || vertices.size() > 65535 ||
                      ( lastError < error && triangles.size() > (target_tri_count/2)) ) {
                    // get the next vertex to collapse
                    reduxVertex *mn = MinimumCostEdge();
                    lastError = mn->objdist;
                    Collapse(mn, mn->collapse);
                }

                CalCoreSubmesh::VectorFace tris;
                tris.reserve(triangles.size());
                for (unsigned int i = 0; i<triangles.size(); i++) {
                    tris.push_back( CalCoreSubmesh::Face(   triangles[i]->vertex[0]->id,
                                                            triangles[i]->vertex[1]->id,
                                                            triangles[i]->vertex[2]->id ));
                }
                ApplyFaces(tris);
                retval = true;
            }
        }
        
        // cleanup memory
        //while (triangles.size() > 0) {
            // get the next vertex to collapse
        //    reduxVertex *mn = MinimumCostEdge();
        //    Collapse(mn, mn->collapse);
        //}
        for(unsigned int i = 0; i < triangles.size(); i++)
        {
            delete triangles[i];
        }
        vertices.clear();
        triangles.clear();
    }
    return retval;
}

 
