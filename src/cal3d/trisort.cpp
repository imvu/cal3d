#include <vector>
#include <stddef.h>

#include "trisort.h"

namespace {
    template<typename T>
    T* vec_data(std::vector<T>& v) {
        return v.empty() ? 0 : &v[0];
    }

    template<typename T>
    const T* vec_data(const std::vector<const T>& v) {
        return v.empty() ? 0 : &v[0];
    }

    struct Vec3f {
        float x, y, z;
    };

    Vec3f operator-(const Vec3f& lhs, const Vec3f& rhs) {
        Vec3f rv;
        rv.x = lhs.x - rhs.x;
        rv.y = lhs.y - rhs.y;
        rv.z = lhs.z - rhs.z;
        return rv;
    }

    float length(const Vec3f& v) {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    Vec3f cross(const Vec3f& a, const Vec3f& u) {
        Vec3f rv;
        rv.x = a.y * u.z - a.z * u.y;
        rv.y = a.z * u.x - a.x * u.z;
        rv.z = a.x * u.y - a.y * u.x;
        return rv;
    }

    struct Triangle {
        unsigned short v0, v1, v2;
    };
}

struct TriangleSorter {
    TriangleSorter(unsigned triangleCount, Triangle* triangles, const float* positions, unsigned positionStride)
        : triangleCount(triangleCount)
        , triangles(triangles)
        , positions(positions)
        , positionStride(positionStride)
    {}

    void heuristicCycleBreaker() {
        // The "value" of an edge (x,y) is area(x) * coverage(y,x).
        // Coverage is some function of area(y) and the angle of
        // incidence.  "Across all of x, integrate the percentage of
        // the hemisphere covered by y".  Computing coverage is n^2
        // and not terribly cheap so just sort triangles so smaller
        // ones come first.  Thus, the topological sort will break
        // cycles from larger triangles to smaller ones, reducing
        // overall error.  There is no correct ordering given a cycle,
        // but there is a least bad ordering, and we can get close
        // without much work.

        struct SortByArea {
            SortByArea(TriangleSorter* ts)
                : ts(ts)
            {}

            bool operator()(const Triangle& lhs, const Triangle& rhs) {
                return ts->area(lhs) < ts->area(rhs);
            }

        private:
            TriangleSorter* ts;
        };
        std::sort(triangles, triangles + triangleCount, SortByArea(this));
    }

    void buildDigraph() {
        edges.resize(triangleCount);
        for (unsigned a = 0; a < triangleCount; ++a) {
            for (unsigned b = 0; b < a; ++b) {
                bool a_facing_b = isFacing(a, b);
                bool b_facing_a = isFacing(b, a);
                if (a_facing_b != b_facing_a) {
                    std::vector<unsigned>& edgelist = edges[a_facing_b ? a : b];
                    unsigned edge = a_facing_b ? b : a;
                    edgelist.push_back(edge);
                }
            }
        }
    }

    void topologicalSort() {
        // assert empty?
        fastedge_storage.reserve(triangleCount);
        for (auto i = edges.begin(); i != edges.end(); ++i) {
            FastEdge fe;
            fe.begin = vec_data(*i);
            fe.end = fe.begin + i->size();
            fastedge_storage.push_back(fe);
        }
        fastedges = vec_data(fastedge_storage);

        visited_storage.resize(triangleCount);
        visited = vec_data(visited_storage);

        output_storage.resize(triangleCount);
        output = vec_data(output_storage);

        // again, iterating from smallest triangle to largest
        for (unsigned i = 0; i < triangleCount; ++i) {
            visit(i);
        }
    }

    void writeOutput() {
        assert(output == vec_data(output_storage) + output_storage.size());

        std::vector<Triangle> newTriangles;
        newTriangles.reserve(triangleCount);

        for (auto i = output_storage.rbegin(); i != output_storage.rend(); ++i) {
            newTriangles.push_back(triangles[*i]);
        }

        std::copy(newTriangles.begin(), newTriangles.end(), triangles);
    }

private:
    const Vec3f& getPosition(unsigned short i) {
        return *reinterpret_cast<const Vec3f*>(positions + i * positionStride);
    }

    float area(const Triangle& t) {
        Vec3f v0 = getPosition(t.v0);
        Vec3f v1 = getPosition(t.v1);
        Vec3f v2 = getPosition(t.v2);
        return length(cross(v1 - v0, v2 - v0)) / 2;
    }

    bool inFront(const Vec3f& p, const Triangle& t) {
        // TODO: precalculate planes
        // TODO: vectorize

        const auto& v1 = getPosition(t.v0);
        const auto& v2 = getPosition(t.v1);
        const auto& v3 = getPosition(t.v2);

        float plane_a = v1.y * (v2.z - v3.z) + v2.y * (v3.z - v1.z) + v3.y * (v1.z - v2.z);
        float plane_b = v1.z * (v2.x - v3.x) + v2.z * (v3.x - v1.x) + v3.z * (v1.x - v2.x);
        float plane_c = v1.x * (v2.y - v3.y) + v2.x * (v3.y - v1.y) + v3.x * (v1.y - v2.y);
        float plane_d =  ( v1.x * ( v2.y * v3.z - v3.y * v2.z ) +
                            v2.x * (v3.y * v1.z - v1.y * v3.z) +
                            v3.x * (v1.y * v2.z - v2.y * v1.z) );

        return plane_a * p.x + plane_b * p.y + plane_c * p.z >= plane_d;
    }

    bool isFacing(unsigned a, unsigned b) {
        return inFront(getPosition(triangles[b].v0), triangles[a])
            && inFront(getPosition(triangles[b].v1), triangles[a])
            && inFront(getPosition(triangles[b].v2), triangles[a]);
    }

    // TODO: could make this non-recursive but it should only recurse
    // triangleCount times, which ought to fit in a 1 MB stack...
    void visit(unsigned i) {
        if (visited[i]) {
            return;
        }
        visited[i] = true;
        
        FastEdge fe = fastedges[i];
        for (unsigned* e = fe.begin; e != fe.end; ++e) {
            visit(*e);
        }

        *output++ = i;
    }

    // inputs
    const unsigned triangleCount;
    Triangle* const triangles;
    const float* const positions;
    const unsigned positionStride;

    // dense adjacency list, a -> b
    // a -> b means "b behind a", thus b should be rendered first
    // worst case storage is something like triangleCount^2 * sizeof(unsigned)
    // for 100k triangles, that's 40 GB
    std::vector<std::vector<unsigned> > edges;
    struct FastEdge {
        unsigned* begin;
        unsigned* end;
    };
    std::vector<FastEdge> fastedge_storage;
    FastEdge* fastedges; // fast access
    
    std::vector<char> visited_storage;
    char* visited; // fast access

    // reversed
    std::vector<unsigned> output_storage;
    unsigned* output; // fast access
};

void sortTrianglesBackToFront(
    unsigned triangleCount,
    unsigned short* indices,
    const float* positions,
    unsigned positionStride
) {
    // so we can count on there being at least one entry in all the arrays
    if (!triangleCount) {
        return;
    }

    Triangle* triangles = reinterpret_cast<Triangle*>(indices);

    TriangleSorter ts(triangleCount, triangles, positions, positionStride);
    ts.heuristicCycleBreaker();
    ts.buildDigraph();
    ts.topologicalSort();
    ts.writeOutput();
}
