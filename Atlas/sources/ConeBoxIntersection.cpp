#include "Atlas/core/ConeBoxIntersection.h"

#include "atlas/core/Bounds.h"

using namespace atlas;

TIQuery::TIQuery()
    :
    mNumCandidateEdges(0)
{
    // An edge is { v0, v1 }, where v0 and v1 are relative to mVertices
    // with v0 < v1.
    mEdges[0] = { 0, 1 };
    mEdges[1] = { 1, 3 };
    mEdges[2] = { 2, 3 };
    mEdges[3] = { 0, 2 };
    mEdges[4] = { 4, 5 };
    mEdges[5] = { 5, 7 };
    mEdges[6] = { 6, 7 };
    mEdges[7] = { 4, 6 };
    mEdges[8] = { 0, 4 };
    mEdges[9] = { 1, 5 };
    mEdges[10] = { 3, 7 };
    mEdges[11] = { 2, 6 };

    // A face is { { v0, v1, v2, v3 }, { e0, e1, e2, e3 } }, where
    // { v0, v1, v2, v3 } are relative to mVertices with
    // v0 = min(v0,v1,v2,v3) and where { e0, e1, e2, e3 } are relative
    // to mEdges.  For example, mFaces[0] has vertices { 0, 4, 6, 2 }.
    // The edge { 0, 4 } is mEdges[8], the edge { 4, 6 } is mEdges[7],
    // the edge { 6, 2 } is mEdges[11] and the edge { 2, 0 } is
    // mEdges[3]; thus, the edge indices are { 8, 7, 11, 3 }.
    mFaces[0] = { { 0, 4, 6, 2 }, {  8,  7, 11,  3 } };
    mFaces[1] = { { 1, 3, 7, 5 }, {  1, 10,  5,  9 } };
    mFaces[2] = { { 0, 1, 5, 4 }, {  0,  9,  4,  8 } };
    mFaces[3] = { { 2, 6, 7, 3 }, { 11,  6, 10,  2 } };
    mFaces[4] = { { 0, 2, 3, 1 }, {  3,  2,  1,  0 } };
    mFaces[5] = { { 4, 5, 7, 6 }, {  4,  5,  6,  7 } };

    // Clear the edges.
    std::array<size_t, 2> ezero = { 0, 0 };
    mCandidateEdges.fill(ezero);
    for (size_t r = 0; r < MAX_VERTICES; ++r)
    {
        mAdjacencyMatrix[r].fill(0);
    }

    mConfiguration[0] = &TIQuery::NNNN_0;
    mConfiguration[1] = &TIQuery::NNNZ_1;
    mConfiguration[2] = &TIQuery::NNNP_2;
    mConfiguration[3] = &TIQuery::NNZN_3;
    mConfiguration[4] = &TIQuery::NNZZ_4;
    mConfiguration[5] = &TIQuery::NNZP_5;
    mConfiguration[6] = &TIQuery::NNPN_6;
    mConfiguration[7] = &TIQuery::NNPZ_7;
    mConfiguration[8] = &TIQuery::NNPP_8;
    mConfiguration[9] = &TIQuery::NZNN_9;
    mConfiguration[10] = &TIQuery::NZNZ_10;
    mConfiguration[11] = &TIQuery::NZNP_11;
    mConfiguration[12] = &TIQuery::NZZN_12;
    mConfiguration[13] = &TIQuery::NZZZ_13;
    mConfiguration[14] = &TIQuery::NZZP_14;
    mConfiguration[15] = &TIQuery::NZPN_15;
    mConfiguration[16] = &TIQuery::NZPZ_16;
    mConfiguration[17] = &TIQuery::NZPP_17;
    mConfiguration[18] = &TIQuery::NPNN_18;
    mConfiguration[19] = &TIQuery::NPNZ_19;
    mConfiguration[20] = &TIQuery::NPNP_20;
    mConfiguration[21] = &TIQuery::NPZN_21;
    mConfiguration[22] = &TIQuery::NPZZ_22;
    mConfiguration[23] = &TIQuery::NPZP_23;
    mConfiguration[24] = &TIQuery::NPPN_24;
    mConfiguration[25] = &TIQuery::NPPZ_25;
    mConfiguration[26] = &TIQuery::NPPP_26;
    mConfiguration[27] = &TIQuery::ZNNN_27;
    mConfiguration[28] = &TIQuery::ZNNZ_28;
    mConfiguration[29] = &TIQuery::ZNNP_29;
    mConfiguration[30] = &TIQuery::ZNZN_30;
    mConfiguration[31] = &TIQuery::ZNZZ_31;
    mConfiguration[32] = &TIQuery::ZNZP_32;
    mConfiguration[33] = &TIQuery::ZNPN_33;
    mConfiguration[34] = &TIQuery::ZNPZ_34;
    mConfiguration[35] = &TIQuery::ZNPP_35;
    mConfiguration[36] = &TIQuery::ZZNN_36;
    mConfiguration[37] = &TIQuery::ZZNZ_37;
    mConfiguration[38] = &TIQuery::ZZNP_38;
    mConfiguration[39] = &TIQuery::ZZZN_39;
    mConfiguration[40] = &TIQuery::ZZZZ_40;
    mConfiguration[41] = &TIQuery::ZZZP_41;
    mConfiguration[42] = &TIQuery::ZZPN_42;
    mConfiguration[43] = &TIQuery::ZZPZ_43;
    mConfiguration[44] = &TIQuery::ZZPP_44;
    mConfiguration[45] = &TIQuery::ZPNN_45;
    mConfiguration[46] = &TIQuery::ZPNZ_46;
    mConfiguration[47] = &TIQuery::ZPNP_47;
    mConfiguration[48] = &TIQuery::ZPZN_48;
    mConfiguration[49] = &TIQuery::ZPZZ_49;
    mConfiguration[50] = &TIQuery::ZPZP_50;
    mConfiguration[51] = &TIQuery::ZPPN_51;
    mConfiguration[52] = &TIQuery::ZPPZ_52;
    mConfiguration[53] = &TIQuery::ZPPP_53;
    mConfiguration[54] = &TIQuery::PNNN_54;
    mConfiguration[55] = &TIQuery::PNNZ_55;
    mConfiguration[56] = &TIQuery::PNNP_56;
    mConfiguration[57] = &TIQuery::PNZN_57;
    mConfiguration[58] = &TIQuery::PNZZ_58;
    mConfiguration[59] = &TIQuery::PNZP_59;
    mConfiguration[60] = &TIQuery::PNPN_60;
    mConfiguration[61] = &TIQuery::PNPZ_61;
    mConfiguration[62] = &TIQuery::PNPP_62;
    mConfiguration[63] = &TIQuery::PZNN_63;
    mConfiguration[64] = &TIQuery::PZNZ_64;
    mConfiguration[65] = &TIQuery::PZNP_65;
    mConfiguration[66] = &TIQuery::PZZN_66;
    mConfiguration[67] = &TIQuery::PZZZ_67;
    mConfiguration[68] = &TIQuery::PZZP_68;
    mConfiguration[69] = &TIQuery::PZPN_69;
    mConfiguration[70] = &TIQuery::PZPZ_70;
    mConfiguration[71] = &TIQuery::PZPP_71;
    mConfiguration[72] = &TIQuery::PPNN_72;
    mConfiguration[73] = &TIQuery::PPNZ_73;
    mConfiguration[74] = &TIQuery::PPNP_74;
    mConfiguration[75] = &TIQuery::PPZN_75;
    mConfiguration[76] = &TIQuery::PPZZ_76;
    mConfiguration[77] = &TIQuery::PPZP_77;
    mConfiguration[78] = &TIQuery::PPPN_78;
    mConfiguration[79] = &TIQuery::PPPZ_79;
    mConfiguration[80] = &TIQuery::PPPP_80;
}

bool TIQuery::operator()(const Bounds3f &box, const BoundingCone &cone)
{
    bool result;

    // Quick-rejectance test.  Determine whether the box is outside
    // the slab bounded by the minimum and maximum height planes.
    // When outside the slab, the box vertices are not required by the
    // cone-box intersection query, so the vertices are not yet
    // computed.
    Float boxMinHeight(0), boxMaxHeight(0);
    ComputeBoxHeightInterval(box, cone, boxMinHeight, boxMaxHeight);
    // TODO: See the comments at the beginning of this file.
    Float coneMaxHeight = cone.tmax;
    if (boxMaxHeight <= cone.tmin || boxMinHeight >= coneMaxHeight)
    {
        // There is no volumetric overlap of the box and the cone. The
        // box is clipped entirely.
        result = false;
        return result;
    }

    // Quick-acceptance test.  Determine whether the cone axis
    // intersects the box.
    if (ConeAxisIntersectsBox(box, cone))
    {
        result = true;
        return result;
    }

    // Test for box fully inside the slab.  When inside the slab, the
    // box vertices are required by the cone-box intersection query,
    // so they are computed here; they are also required in the
    // remaining cases.  Also when inside the slab, the box edges are
    // the candidates, so they are copied to mCandidateEdges.
    if (BoxFullyInConeSlab(box, boxMinHeight, boxMaxHeight, cone))
    {
        result = CandidatesHavePointInsideCone(cone);
        return result;
    }

    // Clear the candidates array and adjacency matrix.
    ClearCandidates();

    // The box intersects at least one plane.  Compute the box-plane
    // edge-interior intersection points.  Insert the box subedges into
    // the array of candidate edges.
    ComputeCandidatesOnBoxEdges(cone);

    // Insert any relevant box face-interior clipped edges into the array
    // of candidate edges.
    ComputeCandidatesOnBoxFaces();

    result = CandidatesHavePointInsideCone(cone);
    return result;
}

void TIQuery::ComputeBoxHeightInterval(Bounds3f const &box, BoundingCone const &cone,
    Float &boxMinHeight, Float &boxMaxHeight)
{
    Vector3<Float> C, e;
    C = (Vec3f)(box.max + box.min) * 0.5f;
    e = (Vec3f)(box.max - box.min) * 0.5f;
    //box.GetCenteredForm(C, e);
    Vector3<Float> const &V = (Vec3f)cone.origin;
    Vector3<Float> const &U = cone.dir;
    Vector3<Float> CmV = C - V;
    Float DdCmV = dot(U, CmV);
    Float radius = e[0] * std::abs(U[0]) + e[1] * std::abs(U[1]) + e[2] * std::abs(U[2]);
    boxMinHeight = DdCmV - radius;
    boxMaxHeight = DdCmV + radius;
}

bool TIQuery::ConeAxisIntersectsBox(Bounds3f const &box, BoundingCone const &cone)
{
    Ray r(cone.origin, cone.dir, cone.tmax);
    Vec3f invDir(1.f / cone.dir.x, 1.f / cone.dir.y, 1.f / cone.dir.z);
    int8_t dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };
    if (box.intersectP(r, invDir, dirIsNeg))
        return true;

    //Segment<3, Real> segment;
    //segment.p[0] = cone.origin + cone.tmin * cone.dir;
    //segment.p[1] = cone.origin + cone.tmax * cone.dir;
    //auto sbResult = TIQuery()(segment, box);
    //if (sbResult.intersect)
    //{
    //    return true;
    //}
    return false;
}

bool TIQuery::BoxFullyInConeSlab(Bounds3f const &box, Float boxMinHeight, Float boxMaxHeight, BoundingCone const &cone)
{
    // Compute the box vertices relative to cone vertex as origin.
    mVertices[0] = { box.min[0], box.min[1], box.min[2] };
    mVertices[1] = { box.max[0], box.min[1], box.min[2] };
    mVertices[2] = { box.min[0], box.max[1], box.min[2] };
    mVertices[3] = { box.max[0], box.max[1], box.min[2] };
    mVertices[4] = { box.min[0], box.min[1], box.max[2] };
    mVertices[5] = { box.max[0], box.min[1], box.max[2] };
    mVertices[6] = { box.min[0], box.max[1], box.max[2] };
    mVertices[7] = { box.max[0], box.max[1], box.max[2] };
    for (int i = 0; i < NUM_BOX_VERTICES; ++i)
    {
        mVertices[i] -= (Vec3f)cone.origin;
    }

    Float coneMaxHeight = cone.tmax;
    if (cone.tmin <= boxMinHeight && boxMaxHeight <= coneMaxHeight)
    {
        // The box is fully inside, so no clipping is necessary.
        std::copy(mEdges.begin(), mEdges.end(), mCandidateEdges.begin());
        mNumCandidateEdges = 12;
        return true;
    }
    return false;
}

bool TIQuery::HasPointInsideCone(Vector3<Float> const &P0, Vector3<Float> const &P1,
    BoundingCone const &cone)
{
    // Define F(X) = Dot(U,X - V)/|X - V|, where U is the unit-length
    // cone axis direction and V is the cone vertex.  The incoming
    // points P0 and P1 are relative to V; that is, the original
    // points are X0 = P0 + V and X1 = P1 + V.  The segment <P0,P1>
    // and cone intersect when a segment point X is inside the cone;
    // that is, when F(X) > cosAngle.  The comparison is converted to
    // an equivalent one that does not involve divisions in order to
    // avoid a division by zero if a vertex or edge contain (0,0,0).
    // The function is G(X) = Dot(U,X-V) - cosAngle*Length(X-V).
    Vector3<Float> const &U = cone.dir;

    // Test whether P0 or P1 is inside the cone.
    Float g = dot(U, P0) - cone.dot * P0.length();
    if (g > (Float)0)
    {
        // X0 = P0 + V is inside the cone.
        return true;
    }

    g = dot(U, P1) - cone.dot * P1.length();
    if (g > (Float)0)
    {
        // X1 = P1 + V is inside the cone.
        return true;
    }

    // Test whether an interior segment point is inside the cone.
    Vector3<Float> E = P1 - P0;
    Vector3<Float> crossP0U = cross(P0, U);
    Vector3<Float> crossP0E = cross(P0, E);
    Float dphi0 = dot(crossP0E, crossP0U);
    if (dphi0 > (Float)0)
    {
        Vector3<Float> crossP1U = cross(P1, U);
        Float dphi1 = dot(crossP0E, crossP1U);
        if (dphi1 < (Float)0)
        {
            Float t = dphi0 / (dphi0 - dphi1);
            Vector3<Float> PMax = P0 + t * E;
            g = dot(U, PMax) - cone.dot * PMax.length();
            if (g > (Float)0)
            {
                // The edge point XMax = Pmax + V is inside the cone.
                return true;
            }
        }
    }

    return false;
}

bool TIQuery::CandidatesHavePointInsideCone(BoundingCone const &cone) const
{
    for (size_t i = 0; i < mNumCandidateEdges; ++i)
    {
        auto const &edge = mCandidateEdges[i];
        Vector3<Float> const &P0 = mVertices[edge[0]];
        Vector3<Float> const &P1 = mVertices[edge[1]];
        if (HasPointInsideCone(P0, P1, cone))
        {
            return true;
        }
    }
    return false;
}

void TIQuery::ComputeCandidatesOnBoxEdges(BoundingCone const &cone)
{
    for (size_t i = 0; i < NUM_BOX_VERTICES; ++i)
    {
        Float h = dot(cone.dir, mVertices[i]);
        Float coneMaxHeight = cone.tmax;
        mProjectionMin[i] = cone.tmin - h;
        mProjectionMax[i] = h - coneMaxHeight;
    }

    size_t v0 = VERTEX_MIN_BASE, v1 = VERTEX_MAX_BASE;
    for (size_t i = 0; i < NUM_BOX_EDGES; ++i, ++v0, ++v1)
    {
        auto const &edge = mEdges[i];

        // In the next blocks, the sign comparisons can be expressed
        // instead as "s0 * s1 < 0". The multiplication could lead to
        // floating-point underflow where the product becomes 0, so I
        // avoid that approach.

        // Process the hmin-plane.
        Float p0Min = mProjectionMin[edge[0]];
        Float p1Min = mProjectionMin[edge[1]];
        bool clipMin = (p0Min < (Float)0 && p1Min >(Float)0) || (p0Min > (Float)0 && p1Min < (Float)0);
        if (clipMin)
        {
            mVertices[v0] = (p1Min * mVertices[edge[0]] - p0Min * mVertices[edge[1]]) / (p1Min - p0Min);
        }

        // Process the hmax-plane.
        Float p0Max = mProjectionMax[edge[0]];
        Float p1Max = mProjectionMax[edge[1]];
        bool clipMax = (p0Max < (Float)0 && p1Max >(Float)0) || (p0Max > (Float)0 && p1Max < (Float)0);
        if (clipMax)
        {
            mVertices[v1] = (p1Max * mVertices[edge[0]] - p0Max * mVertices[edge[1]]) / (p1Max - p0Max);
        }

        // Get the candidate edges that are contained by the box edges.
        if (clipMin)
        {
            if (clipMax)
            {
                InsertEdge(v0, v1);
            }
            else
            {
                if (p0Min < (Float)0)
                {
                    InsertEdge(edge[0], v0);
                }
                else  // p1Min < 0
                {
                    InsertEdge(edge[1], v0);
                }
            }
        }
        else if (clipMax)
        {
            if (p0Max < (Float)0)
            {
                InsertEdge(edge[0], v1);
            }
            else  // p1Max < 0
            {
                InsertEdge(edge[1], v1);
            }
        }
        else
        {
            // No clipping has occurred.  If the edge is inside the box,
            // it is a candidate edge.  To be inside the box, the p*min
            // and p*max values must be nonpositive.
            if (p0Min <= (Float)0 && p1Min <= (Float)0 && p0Max <= (Float)0 && p1Max <= (Float)0)
            {
                InsertEdge(edge[0], edge[1]);
            }
        }
    }
}

void TIQuery::ComputeCandidatesOnBoxFaces()
{
    Float p0, p1, p2, p3;
    size_t b0, b1, b2, b3, index;
    for (size_t i = 0; i < NUM_BOX_FACES; ++i)
    {
        auto const &face = mFaces[i];

        // Process the hmin-plane.
        p0 = mProjectionMin[face.v[0]];
        p1 = mProjectionMin[face.v[1]];
        p2 = mProjectionMin[face.v[2]];
        p3 = mProjectionMin[face.v[3]];
        b0 = (p0 < (Float)0 ? 0 : (p0 > (Float)0 ? 2 : 1));
        b1 = (p1 < (Float)0 ? 0 : (p1 > (Float)0 ? 2 : 1));
        b2 = (p2 < (Float)0 ? 0 : (p2 > (Float)0 ? 2 : 1));
        b3 = (p3 < (Float)0 ? 0 : (p3 > (Float)0 ? 2 : 1));
        index = b3 + 3 * (b2 + 3 * (b1 + 3 * b0));
        (this->*mConfiguration[index])(VERTEX_MIN_BASE, face);

        // Process the hmax-plane.
        p0 = mProjectionMax[face.v[0]];
        p1 = mProjectionMax[face.v[1]];
        p2 = mProjectionMax[face.v[2]];
        p3 = mProjectionMax[face.v[3]];
        b0 = (p0 < (Float)0 ? 0 : (p0 > (Float)0 ? 2 : 1));
        b1 = (p1 < (Float)0 ? 0 : (p1 > (Float)0 ? 2 : 1));
        b2 = (p2 < (Float)0 ? 0 : (p2 > (Float)0 ? 2 : 1));
        b3 = (p3 < (Float)0 ? 0 : (p3 > (Float)0 ? 2 : 1));
        index = b3 + 3 * (b2 + 3 * (b1 + 3 * b0));
        (this->*mConfiguration[index])(VERTEX_MAX_BASE, face);
    }
}

void TIQuery::ClearCandidates()
{
    for (size_t i = 0; i < mNumCandidateEdges; ++i)
    {
        auto const &edge = mCandidateEdges[i];
        mAdjacencyMatrix[edge[0]][edge[1]] = 0;
        mAdjacencyMatrix[edge[1]][edge[0]] = 0;
    }
    mNumCandidateEdges = 0;
}

void TIQuery::InsertEdge(size_t v0, size_t v1)
{
    if (mAdjacencyMatrix[v0][v1] == 0)
    {
        mAdjacencyMatrix[v0][v1] = 1;
        mAdjacencyMatrix[v1][v0] = 1;
        mCandidateEdges[mNumCandidateEdges] = { v0, v1 };
        ++mNumCandidateEdges;
    }
}

void TIQuery::NNNN_0(size_t, Face const &)
{
}

void TIQuery::NNNZ_1(size_t, Face const &)
{
}

void TIQuery::NNNP_2(size_t base, Face const &face)
{
    InsertEdge(base + face.e[2], base + face.e[3]);
}

void TIQuery::NNZN_3(size_t, Face const &)
{
}

void TIQuery::NNZZ_4(size_t, Face const &)
{
}

void TIQuery::NNZP_5(size_t base, Face const &face)
{
    InsertEdge(face.v[2], base + face.e[3]);
}

void TIQuery::NNPN_6(size_t base, Face const &face)
{
    InsertEdge(base + face.e[1], base + face.e[2]);
}

void TIQuery::NNPZ_7(size_t base, Face const &face)
{
    InsertEdge(base + face.e[1], face.v[3]);
}

void TIQuery::NNPP_8(size_t base, Face const &face)
{
    InsertEdge(base + face.e[1], base + face.e[3]);
}

void TIQuery::NZNN_9(size_t, Face const &)
{
}

void TIQuery::NZNZ_10(size_t, Face const &)
{
}

void TIQuery::NZNP_11(size_t base, Face const &face)
{
    InsertEdge(base + face.e[2], face.v[3]);
    InsertEdge(base + face.e[3], face.v[3]);
}

void TIQuery::NZZN_12(size_t, Face const &)
{
}

void TIQuery::NZZZ_13(size_t, Face const &)
{
}

void TIQuery::NZZP_14(size_t base, Face const &face)
{
    InsertEdge(face.v[2], face.v[3]);
    InsertEdge(base + face.e[3], face.v[3]);
}

void TIQuery::NZPN_15(size_t base, Face const &face)
{
    InsertEdge(base + face.e[2], face.v[1]);
}

void TIQuery::NZPZ_16(size_t, Face const &face)
{
    InsertEdge(face.v[1], face.v[3]);
}

void TIQuery::NZPP_17(size_t base, Face const &face)
{
    InsertEdge(base + face.e[3], face.v[1]);
}

void TIQuery::NPNN_18(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], base + face.e[1]);
}

void TIQuery::NPNZ_19(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], face.v[1]);
    InsertEdge(base + face.e[1], face.v[1]);
}

void TIQuery::NPNP_20(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], face.v[1]);
    InsertEdge(base + face.e[1], face.v[1]);
    InsertEdge(base + face.e[2], face.v[3]);
    InsertEdge(base + face.e[3], face.v[3]);
}

void TIQuery::NPZN_21(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], face.v[2]);
}

void TIQuery::NPZZ_22(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], face.v[1]);
    InsertEdge(face.v[1], face.v[2]);
}

void TIQuery::NPZP_23(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], face.v[1]);
    InsertEdge(face.v[1], face.v[2]);
    InsertEdge(base + face.e[3], face.v[2]);
    InsertEdge(face.v[2], face.v[3]);
}

void TIQuery::NPPN_24(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], base + face.e[2]);
}

void TIQuery::NPPZ_25(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], face.v[3]);
}

void TIQuery::NPPP_26(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], base + face.e[3]);
}

void TIQuery::ZNNN_27(size_t, Face const &)
{
}

void TIQuery::ZNNZ_28(size_t, Face const &)
{
}

void TIQuery::ZNNP_29(size_t base, Face const &face)
{
    InsertEdge(base + face.e[2], face.v[0]);
}

void TIQuery::ZNZN_30(size_t, Face const &)
{
}

void TIQuery::ZNZZ_31(size_t, Face const &)
{
}

void TIQuery::ZNZP_32(size_t, Face const &face)
{
    InsertEdge(face.v[0], face.v[2]);
}

void TIQuery::ZNPN_33(size_t base, Face const &face)
{
    InsertEdge(base + face.e[1], face.v[2]);
    InsertEdge(base + face.e[2], face.v[2]);
}

void TIQuery::ZNPZ_34(size_t base, Face const &face)
{
    InsertEdge(base + face.e[1], face.v[2]);
    InsertEdge(face.v[2], face.v[3]);
}

void TIQuery::ZNPP_35(size_t base, Face const &face)
{
    InsertEdge(face.v[0], base + face.e[1]);
}

void TIQuery::ZZNN_36(size_t, Face const &)
{
}

void TIQuery::ZZNZ_37(size_t, Face const &)
{
}

void TIQuery::ZZNP_38(size_t base, Face const &face)
{
    InsertEdge(face.v[0], face.v[3]);
    InsertEdge(face.v[3], base + face.e[2]);
}

void TIQuery::ZZZN_39(size_t, Face const &)
{
}

void TIQuery::ZZZZ_40(size_t, Face const &)
{
}

void TIQuery::ZZZP_41(size_t, Face const &face)
{
    InsertEdge(face.v[0], face.v[3]);
    InsertEdge(face.v[3], face.v[2]);
}

void TIQuery::ZZPN_42(size_t base, Face const &face)
{
    InsertEdge(face.v[1], face.v[2]);
    InsertEdge(face.v[2], base + face.e[2]);
}

void TIQuery::ZZPZ_43(size_t, Face const &face)
{
    InsertEdge(face.v[1], face.v[2]);
    InsertEdge(face.v[2], face.v[3]);
}

void TIQuery::ZZPP_44(size_t, Face const &)
{
}

void TIQuery::ZPNN_45(size_t base, Face const &face)
{
    InsertEdge(face.v[0], base + face.e[1]);
}

void TIQuery::ZPNZ_46(size_t base, Face const &face)
{
    InsertEdge(face.v[0], face.v[1]);
    InsertEdge(face.v[1], base + face.e[1]);
}

void TIQuery::ZPNP_47(size_t base, Face const &face)
{
    InsertEdge(face.v[0], face.v[1]);
    InsertEdge(face.v[1], base + face.e[1]);
    InsertEdge(base + face.e[2], face.v[3]);
    InsertEdge(face.v[3], face.v[0]);
}

void TIQuery::ZPZN_48(size_t, Face const &face)
{
    InsertEdge(face.v[0], face.v[2]);
}

void TIQuery::ZPZZ_49(size_t, Face const &face)
{
    InsertEdge(face.v[0], face.v[1]);
    InsertEdge(face.v[1], face.v[2]);
}

void TIQuery::ZPZP_50(size_t, Face const &)
{
}

void TIQuery::ZPPN_51(size_t base, Face const &face)
{
    InsertEdge(face.v[0], base + face.e[2]);
}

void TIQuery::ZPPZ_52(size_t, Face const &)
{
}

void TIQuery::ZPPP_53(size_t, Face const &)
{
}

void TIQuery::PNNN_54(size_t base, Face const &face)
{
    InsertEdge(base + face.e[3], base + face.e[0]);
}

void TIQuery::PNNZ_55(size_t base, Face const &face)
{
    InsertEdge(face.v[3], base + face.e[0]);
}

void TIQuery::PNNP_56(size_t base, Face const &face)
{
    InsertEdge(base + face.e[2], base + face.e[0]);
}

void TIQuery::PNZN_57(size_t base, Face const &face)
{
    InsertEdge(base + face.e[3], face.v[0]);
    InsertEdge(face.v[0], base + face.e[0]);
}

void TIQuery::PNZZ_58(size_t base, Face const &face)
{
    InsertEdge(face.v[3], face.v[0]);
    InsertEdge(face.v[0], base + face.e[0]);
}

void TIQuery::PNZP_59(size_t base, Face const &face)
{
    InsertEdge(face.v[2], base + face.e[0]);
}

void TIQuery::PNPN_60(size_t base, Face const &face)
{
    InsertEdge(base + face.e[3], face.v[0]);
    InsertEdge(face.v[0], base + face.e[0]);
    InsertEdge(base + face.e[1], face.v[2]);
    InsertEdge(face.v[2], base + face.e[2]);
}

void TIQuery::PNPZ_61(size_t base, Face const &face)
{
    InsertEdge(face.v[3], face.v[0]);
    InsertEdge(face.v[0], base + face.e[0]);
    InsertEdge(base + face.e[1], face.v[2]);
    InsertEdge(face.v[2], face.v[3]);
}

void TIQuery::PNPP_62(size_t base, Face const &face)
{
    InsertEdge(base + face.e[0], base + face.e[1]);
}

void TIQuery::PZNN_63(size_t base, Face const &face)
{
    InsertEdge(base + face.e[3], face.v[1]);
}

void TIQuery::PZNZ_64(size_t, Face const &face)
{
    InsertEdge(face.v[3], face.v[1]);
}

void TIQuery::PZNP_65(size_t base, Face const &face)
{
    InsertEdge(base + face.e[2], face.v[1]);
}

void TIQuery::PZZN_66(size_t base, Face const &face)
{
    InsertEdge(base + face.e[3], face.v[0]);
    InsertEdge(face.v[0], face.v[1]);
}

void TIQuery::PZZZ_67(size_t, Face const &)
{
}

void TIQuery::PZZP_68(size_t, Face const &)
{
}

void TIQuery::PZPN_69(size_t base, Face const &face)
{
    InsertEdge(base + face.e[3], face.v[0]);
    InsertEdge(face.v[0], face.v[1]);
    InsertEdge(face.v[1], face.v[2]);
    InsertEdge(face.v[2], base + face.e[2]);
}

void TIQuery::PZPZ_70(size_t, Face const &)
{
}

void TIQuery::PZPP_71(size_t, Face const &)
{
}

void TIQuery::PPNN_72(size_t base, Face const &face)
{
    InsertEdge(base + face.e[3], base + face.e[1]);
}

void TIQuery::PPNZ_73(size_t base, Face const &face)
{
    InsertEdge(face.v[3], base + face.e[1]);
}

void TIQuery::PPNP_74(size_t base, Face const &face)
{
    InsertEdge(base + face.e[2], base + face.e[1]);
}

void TIQuery::PPZN_75(size_t base, Face const &face)
{
    InsertEdge(base + face.e[2], face.v[2]);
}

void TIQuery::PPZZ_76(size_t, Face const &)
{
}

void TIQuery::PPZP_77(size_t, Face const &)
{
}

void TIQuery::PPPN_78(size_t base, Face const &face)
{
    InsertEdge(base + face.e[3], base + face.e[2]);
}

void TIQuery::PPPZ_79(size_t, Face const &)
{
}

void TIQuery::PPPP_80(size_t, Face const &)
{
}