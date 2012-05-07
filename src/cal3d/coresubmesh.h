//****************************************************************************//
// coresubmesh.h                                                              //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <ostream>
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>
#include "cal3d/aabox.h"
#include "cal3d/color.h"
#include "cal3d/global.h"
#include "cal3d/memory.h"
#include "cal3d/vector.h"
#include "cal3d/vector4.h"
#include "cal3d/bonetransform.h"

CAL3D_PTR(CalCoreMorphTarget);
CAL3D_PTR(CalCoreSkeleton);

enum CalMorphTargetType {
    CalMorphTargetTypeAdditive,
    CalMorphTargetTypeClamped,
    CalMorphTargetTypeAverage,
    CalMorphTargetTypeExclusive
};

class CAL3D_API CalCoreSubmesh {
public:
    // TODO: replace with Vec2f
    struct TextureCoordinate {
        TextureCoordinate() {
        }

        TextureCoordinate(float u, float v)
            : u(u)
            , v(v)
        {}

        float u, v;

        bool operator==(const TextureCoordinate& rhs) const {
            return u == rhs.u && v == rhs.v;
        }
    };

    struct Influence {
        Influence() {
            boneId = static_cast<unsigned>(-1);
            weight = 0.0f;
            lastInfluenceForThisVertex = 0;
        }

        Influence(unsigned b, float w, bool last) {
            boneId = b;
            weight = w;
            lastInfluenceForThisVertex = last ? 1 : 0;
        }

        unsigned boneId;
        float weight;
        unsigned lastInfluenceForThisVertex;

        bool operator==(const Influence& rhs) const {
            return std::make_pair(boneId, quantize(weight)) == std::make_pair(rhs.boneId, quantize(rhs.weight));
        }

        bool operator<(const Influence& rhs) const {
            return std::make_pair(boneId, quantize(weight)) < std::make_pair(rhs.boneId, quantize(rhs.weight));
        }

        // floating point equivalence is tricky
        static int quantize(float f) {
            return static_cast<int>(f * 10000.0);
        }
    };

    struct InfluenceSet {
        InfluenceSet() {
        }

        InfluenceSet(const std::vector<Influence>& vi)
            : influences(vi.begin(), vi.end()) {
        }

        std::set<Influence> influences;

        bool operator==(const InfluenceSet& rhs) const {
            if (influences.size() != rhs.influences.size()) {
                return false;
            }
            std::set<Influence>::const_iterator begin1 = influences.begin();
            std::set<Influence>::const_iterator begin2 = rhs.influences.begin();

            while (begin1 != influences.end()) {
                if (!(*begin1 == *begin2)) {
                    return false;
                }
                ++begin1;
                ++begin2;
            }

            return true;
        }
    };

    CAL3D_ALIGN_HEAD(16)
    struct Vertex {
        CalPoint4 position;
        CalVector4 normal;
    }
    CAL3D_ALIGN_TAIL(16);

    struct Face {
        Face(){}
        Face(CalIndex v0, CalIndex v1, CalIndex v2){vertexId[0]=v0; vertexId[1]= v1; vertexId[2]=v2;}
        CalIndex vertexId[3];

        bool operator==(const Face& rhs) const {
            return std::equal(vertexId, vertexId + 3, rhs.vertexId);
        }
    };

    typedef std::vector<CalCoreMorphTargetPtr> MorphTargetArray;
    typedef std::vector<Face> VectorFace;
    typedef std::vector<TextureCoordinate> VectorTextureCoordinate;
    typedef cal3d::SSEArray<Vertex> VectorVertex;
    typedef std::vector<Influence> InfluenceVector;

    CalCoreSubmesh(int vertexCount, bool hasTextureCoordinates, int faceCount);

    size_t sizeInBytes() const;

    int coreMaterialThreadId;
    std::vector<Face> faces;

    bool hasVertexColors() const {
        return m_hasVertexColors;
    }
    bool hasTextureCoordinates() const {
        return !m_textureCoordinates.empty();
    }
    bool hasTextureCoordinatesOutside0_1() const;
    const VectorTextureCoordinate& getTextureCoordinates() const {
        return m_textureCoordinates;
    }

    const VectorVertex& getVectorVertex() const {
        return m_vertices;
    }

    const std::vector<CalColor32>& getVertexColors() const {
        return m_vertexColors;
    }

    size_t getVertexCount() const {
        return m_vertices.size();
    }

    void addVertex(const Vertex& vertex, CalColor32 vertexColor, const std::vector<Influence>& influences);
    void setTextureCoordinate(int vertexId, const TextureCoordinate& textureCoordinate);

    void addMorphTarget(CalCoreMorphTargetPtr morphTarget);
    const MorphTargetArray& getMorphTargets() const;
    
    void replaceMeshWithMorphTarget(const std::string& morphTargetName);

    void scale(float factor);
    void fixup(const CalCoreSkeletonPtr& skeleton);

    bool isStatic() const;
    BoneTransform getStaticTransform(const BoneTransform* bones) const;

    const InfluenceVector& getInfluences() const {
        return m_influences;
    }

    CalAABox getBoundingVolume() const {
        return m_boundingVolume;
    }

private:
    unsigned m_currentVertexId;

    // The following arrays should always be the same size.
    VectorVertex m_vertices;

    bool m_hasVertexColors;    
    std::vector<CalColor32> m_vertexColors;
 
    VectorTextureCoordinate m_textureCoordinates;

    MorphTargetArray m_morphTargets;

    bool m_isStatic;
    InfluenceSet m_staticInfluenceSet;

    std::vector<Influence> m_influences;
    CalAABox m_boundingVolume;
};
CAL3D_PTR(CalCoreSubmesh);

inline std::ostream& operator<<(std::ostream& os, const CalCoreSubmesh::Influence& influence) {
    return os << "CalCoreSubmesh::Influence(" << influence.boneId << ", " << influence.weight << ", " << influence.lastInfluenceForThisVertex << ")";
}

CAL3D_API CalCoreSubmeshPtr MakeCube();
