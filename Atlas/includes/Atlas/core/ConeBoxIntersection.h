#pragma once

#include "atlas/Atlas.h"
#include "atlas/core/Bounds.h"
#include "atlas/core/Payload.h"

namespace atlas
{
    class TIQuery
    {
    public:

        TIQuery();
        bool operator()(const Bounds3f &box, const BoundingCone &cone);

    protected:
        // The constants here are described in the comments below.
        enum
        {
            NUM_BOX_VERTICES = 8,
            NUM_BOX_EDGES = 12,
            NUM_BOX_FACES = 6,
            MAX_VERTICES = 32,
            VERTEX_MIN_BASE = 8,
            VERTEX_MAX_BASE = 20,
            MAX_CANDIDATE_EDGES = 496,
            NUM_CONFIGURATIONS = 81
        };

        // The box topology is that of a cube whose vertices have components
        // in {0,1}.  The cube vertices are indexed by
        //   0: (0,0,0), 1: (1,0,0), 2: (1,1,0), 3: (0,1,0)
        //   4: (0,0,1), 5: (1,0,1), 6: (1,1,1), 7: (0,1,1)

        // The first 8 vertices are the box corners, the next 12 vertices are
        // reserved for hmin-edge points and the final 12 vertices are reserved
        // for the hmax-edge points.  The conservative upper bound of the number
        // of vertices is 8 + 12 + 12 = 32.
        std::array<Vector3<Float>, MAX_VERTICES> mVertices;

        // The box has 12 edges stored in mEdges.  An edge is mEdges[i] =
        // { v0, v1 }, where the indices v0 and v1 are relative to mVertices
        // with v0 < v1.
        std::array<std::array<size_t, 2>, NUM_BOX_EDGES> mEdges;

        // The box has 6 faces stored in mFaces.  A face is mFaces[i] =
        // { { v0, v1, v2, v3 }, { e0, e1, e2, e3 } }, where the face corner
        // vertices are { v0, v1, v2, v3 }.  These indices are relative to
        // mVertices.  The indices { e0, e1, e2, e3 } are relative to mEdges.
        // The index e0 refers to edge { v0, v1 }, the index e1 refers to edge
        // { v1, v2 }, the index e2 refers to edge { v2, v3 } and the index e3
        // refers to edge { v3, v0 }.  The ordering of vertices for the faces
        // is/ counterclockwise when viewed from outside the box.  The choice
        // of initial vertex affects how you implement the graph data
        // structure.  In this implementation, the initial vertex has minimum
        // index for all vertices of that face.  The faces themselves are
        // listed as -x face, +x face, -y face, +y face, -z face and +z face.
        struct Face
        {
            std::array<size_t, 4> v, e;
        };
        std::array<Face, NUM_BOX_FACES> mFaces;

        // Store the signed distances from the minimum and maximum height
        // planes for the cone to the projection of the box vertices onto the
        // cone axis.
        std::array<Float, NUM_BOX_VERTICES> mProjectionMin, mProjectionMax;

        // The mCandidateEdges array stores the edges of the clipped box that
        // are candidates for containing the optimizing point.  The maximum
        // number of candidate edges is 1 + 2 + ... + 31 = 496, which is a
        // conservative bound because not all pairs of vertices form edges on
        // box faces.  The candidate edges are stored as (v0,v1) with v0 < v1.
        // The implementation is designed so that during a single query, the
        // number of candidate edges can only grow.
        size_t mNumCandidateEdges;
        std::array<std::array<size_t, 2>, MAX_CANDIDATE_EDGES> mCandidateEdges;

        // The mAdjancencyMatrix is a simple representation of edges in the
        // graph G = (V,E) that represents the (wireframe) clipped box.  An
        // edge (r,c) does not exist when mAdjancencyMatrix[r][c] = 0.  If an
        // edge (r,c) does exist, it is appended to mCandidateEdges at index k
        // and the adjacency matrix is set to mAdjacencyMatrix[r][c] = 1.
        // This allows for a fast edge-in-graph test and a fast and efficient
        // clear of mCandidateEdges and mAdjacencyMatrix.
        std::array<std::array<size_t, MAX_VERTICES>, MAX_VERTICES> mAdjacencyMatrix;

        typedef void (TIQuery:: *ConfigurationFunction)(size_t, Face const &);
        std::array<ConfigurationFunction, NUM_CONFIGURATIONS> mConfiguration;

        static void ComputeBoxHeightInterval(Bounds3f const &box, BoundingCone const &cone,
            Float &boxMinHeight, Float &boxMaxHeight);

        static bool ConeAxisIntersectsBox(Bounds3f const &box, BoundingCone const &cone);

        bool BoxFullyInConeSlab(Bounds3f const &box, Float boxMinHeight, Float boxMaxHeight, BoundingCone const &cone);

        static bool HasPointInsideCone(Vector3<Float> const &P0, Vector3<Float> const &P1,
            BoundingCone const &cone);

        bool CandidatesHavePointInsideCone(BoundingCone const &cone) const;

        void ComputeCandidatesOnBoxEdges(BoundingCone const &cone);

        void ComputeCandidatesOnBoxFaces();

        void ClearCandidates();

        void InsertEdge(size_t v0, size_t v1);

        // The 81 possible configurations for a box face.  The N stands for a
        // '-', the Z stands for '0' and the P stands for '+'.  These are
        // listed in the order that maps to the array mConfiguration.  Thus,
        // NNNN maps to mConfiguration[0], NNNZ maps to mConfiguration[1], and
        // so on.
        void NNNN_0(size_t, Face const &);

        void NNNZ_1(size_t, Face const &);

        void NNNP_2(size_t base, Face const &face);

        void NNZN_3(size_t, Face const &);

        void NNZZ_4(size_t, Face const &);

        void NNZP_5(size_t base, Face const &face);

        void NNPN_6(size_t base, Face const &face);

        void NNPZ_7(size_t base, Face const &face);

        void NNPP_8(size_t base, Face const &face);

        void NZNN_9(size_t, Face const &);

        void NZNZ_10(size_t, Face const &);

        void NZNP_11(size_t base, Face const &face);

        void NZZN_12(size_t, Face const &);

        void NZZZ_13(size_t, Face const &);

        void NZZP_14(size_t base, Face const &face);

        void NZPN_15(size_t base, Face const &face);

        void NZPZ_16(size_t, Face const &face);

        void NZPP_17(size_t base, Face const &face);

        void NPNN_18(size_t base, Face const &face);

        void NPNZ_19(size_t base, Face const &face);

        void NPNP_20(size_t base, Face const &face);

        void NPZN_21(size_t base, Face const &face);

        void NPZZ_22(size_t base, Face const &face);

        void NPZP_23(size_t base, Face const &face);

        void NPPN_24(size_t base, Face const &face);

            void NPPZ_25(size_t base, Face const &face);

            void NPPP_26(size_t base, Face const &face);

        void ZNNN_27(size_t, Face const &);

        void ZNNZ_28(size_t, Face const &);

        void ZNNP_29(size_t base, Face const &face);

        void ZNZN_30(size_t, Face const &);

        void ZNZZ_31(size_t, Face const &);

        void ZNZP_32(size_t, Face const &face);

        void ZNPN_33(size_t base, Face const &face);

        void ZNPZ_34(size_t base, Face const &face);

        void ZNPP_35(size_t base, Face const &face);

        void ZZNN_36(size_t, Face const &);

        void ZZNZ_37(size_t, Face const &);

        void ZZNP_38(size_t base, Face const &face);

        void ZZZN_39(size_t, Face const &);

        void ZZZZ_40(size_t, Face const &);

        void ZZZP_41(size_t, Face const &face);

        void ZZPN_42(size_t base, Face const &face);

        void ZZPZ_43(size_t, Face const &face);

        void ZZPP_44(size_t, Face const &);

        void ZPNN_45(size_t base, Face const &face);

        void ZPNZ_46(size_t base, Face const &face);

        void ZPNP_47(size_t base, Face const &face);

        void ZPZN_48(size_t, Face const &face);

        void ZPZZ_49(size_t, Face const &face);

        void ZPZP_50(size_t, Face const &);

        void ZPPN_51(size_t base, Face const &face);

        void ZPPZ_52(size_t, Face const &);

        void ZPPP_53(size_t, Face const &);

        void PNNN_54(size_t base, Face const &face);

        void PNNZ_55(size_t base, Face const &face);

        void PNNP_56(size_t base, Face const &face);

        void PNZN_57(size_t base, Face const &face);

        void PNZZ_58(size_t base, Face const &face);

        void PNZP_59(size_t base, Face const &face);

        void PNPN_60(size_t base, Face const &face);

        void PNPZ_61(size_t base, Face const &face);

        void PNPP_62(size_t base, Face const &face);

        void PZNN_63(size_t base, Face const &face);

        void PZNZ_64(size_t, Face const &face);

        void PZNP_65(size_t base, Face const &face);

        void PZZN_66(size_t base, Face const &face);

        void PZZZ_67(size_t, Face const &);

        void PZZP_68(size_t, Face const &);

        void PZPN_69(size_t base, Face const &face);

        void PZPZ_70(size_t, Face const &);

        void PZPP_71(size_t, Face const &);

        void PPNN_72(size_t base, Face const &face);

        void PPNZ_73(size_t base, Face const &face);

        void PPNP_74(size_t base, Face const &face);

        void PPZN_75(size_t base, Face const &face);

        void PPZZ_76(size_t, Face const &);

        void PPZP_77(size_t, Face const &);

        void PPPN_78(size_t base, Face const &face);

        void PPPZ_79(size_t, Face const &);

        void PPPP_80(size_t, Face const &);
    };

    inline bool intersectP(const Bounds3f &b, const BoundingCone &cone)
    {
        atlas::TIQuery q;
        return (true);
        // return (q(b, cone));
    }
}