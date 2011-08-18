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

#include <set>
#include <map>
#include <boost/shared_ptr.hpp>
#include "cal3d/aabox.h"
#include "cal3d/color.h"
#include "cal3d/global.h"
#include "cal3d/vector.h"
#include "cal3d/vector4.h"

class CalCoreSubMorphTarget;


enum CalMorphTargetType {
    CalMorphTargetTypeNull = 0,
    CalMorphTargetTypeAdditive,
    CalMorphTargetTypeClamped,
    CalMorphTargetTypeAverage,
    CalMorphTargetTypeExclusive
};

class CAL3D_API CalCoreSubmesh {
public:
    struct TextureCoordinate {
        float u, v;
    };

    struct Influence {
        Influence() {
            boneId = -1;
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
        CalIndex vertexId[3];
    };

    typedef std::vector<boost::shared_ptr<CalCoreSubMorphTarget> > CoreSubMorphTargetVector;
    typedef std::vector<Face> VectorFace;
    typedef std::vector<TextureCoordinate> VectorTextureCoordinate;
    typedef std::vector<VectorTextureCoordinate > VectorVectorTextureCoordinate;
    typedef SSEArray<Vertex> VectorVertex;
    typedef std::vector<Influence> InfluenceVector;

    CalCoreSubmesh(int vertexCount, int textureCoordinateCount, int faceCount);

    size_t sizeInBytes() const;

    int coreMaterialThreadId;

    size_t getFaceCount() const {
        return m_vectorFace.size();
    }
    bool hasNonWhiteVertexColors() const {
        return m_hasNonWhiteVertexColors;
    }
    const std::vector<Face>& getVectorFace() const;
    const std::vector<std::vector<TextureCoordinate> >& getVectorVectorTextureCoordinate() const {
        return m_vectorvectorTextureCoordinate;
    }

    const SSEArray<Vertex>& getVectorVertex() const {
        return m_vertices;
    }

    std::vector<CalColor32>& getVertexColors() {
        return m_vertexColors;
    }
    const std::vector<CalColor32>& getVertexColors() const {
        return m_vertexColors;
    }

    size_t getVertexCount() const {
        return m_vertices.size();
    }
    bool setFace(int faceId, const Face& face);

    void addVertex(const Vertex& vertex, CalColor32 vertexColor, const std::vector<Influence>& influences);
    bool setTextureCoordinate(int vertexId, int textureCoordinateId, const TextureCoordinate& textureCoordinate);

    void setHasNonWhiteVertexColors(bool p) {
        m_hasNonWhiteVertexColors = p;
    }
    size_t addCoreSubMorphTarget(boost::shared_ptr<CalCoreSubMorphTarget> pCoreSubMorphTarget);
    const boost::shared_ptr<CalCoreSubMorphTarget>& getCoreSubMorphTarget(size_t id) {
        assert(id < m_vectorCoreSubMorphTarget.size());
        return m_vectorCoreSubMorphTarget[id];
    }
    size_t getCoreSubMorphTargetCount();
    CoreSubMorphTargetVector& getVectorCoreSubMorphTarget();
    void scale(float factor);

    bool isStatic() const;
    BoneTransform getStaticTransform(const BoneTransform* bones) const;

    const InfluenceVector& getInfluences() const {
        return m_influences;
    }

    CalAABox getBoundingVolume() const {
        return m_boundingVolume;
    }

private:
    int m_currentVertexId;

    // The following arrays should always be the same size.
    SSEArray<Vertex> m_vertices;
    std::vector<CalColor32> m_vertexColors;
 
    std::vector<std::vector<TextureCoordinate> > m_vectorvectorTextureCoordinate;
    std::vector<Face> m_vectorFace;

    CoreSubMorphTargetVector m_vectorCoreSubMorphTarget;
    bool m_hasNonWhiteVertexColors;

    bool m_isStatic;
    InfluenceSet m_staticInfluenceSet;

    std::vector<Influence> m_influences;
    CalAABox m_boundingVolume;
};
typedef boost::shared_ptr<CalCoreSubmesh> CalCoreSubmeshPtr;
