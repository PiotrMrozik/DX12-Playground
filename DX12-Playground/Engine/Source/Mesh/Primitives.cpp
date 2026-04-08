#include <DX12LibPCH.h>

#include <Mesh/Primitives.h>

#include <DirectXMath.h>
#include <cmath>

using namespace DirectX;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void PushVertex(MeshData& md, float px, float py, float pz, float nx, float ny, float nz)
{
    md.vertices.push_back({{px, py, pz}, {nx, ny, nz}});
}

// ---------------------------------------------------------------------------
// Cube — 24 vertices (unique normals per face), 36 indices
// ---------------------------------------------------------------------------

MeshData Primitives::CreateCube(float size)
{
    MeshData md;
    const float h = size * 0.5f;

    // Each face has 4 vertices with an outward-facing normal.
    // Front face (+Z)
    PushVertex(md, -h, -h, h, 0, 0, 1);
    PushVertex(md, h, -h, h, 0, 0, 1);
    PushVertex(md, h, h, h, 0, 0, 1);
    PushVertex(md, -h, h, h, 0, 0, 1);
    // Back face (-Z)
    PushVertex(md, h, -h, -h, 0, 0, -1);
    PushVertex(md, -h, -h, -h, 0, 0, -1);
    PushVertex(md, -h, h, -h, 0, 0, -1);
    PushVertex(md, h, h, -h, 0, 0, -1);
    // Top face (+Y)
    PushVertex(md, -h, h, h, 0, 1, 0);
    PushVertex(md, h, h, h, 0, 1, 0);
    PushVertex(md, h, h, -h, 0, 1, 0);
    PushVertex(md, -h, h, -h, 0, 1, 0);
    // Bottom face (-Y)
    PushVertex(md, -h, -h, -h, 0, -1, 0);
    PushVertex(md, h, -h, -h, 0, -1, 0);
    PushVertex(md, h, -h, h, 0, -1, 0);
    PushVertex(md, -h, -h, h, 0, -1, 0);
    // Right face (+X)
    PushVertex(md, h, -h, h, 1, 0, 0);
    PushVertex(md, h, -h, -h, 1, 0, 0);
    PushVertex(md, h, h, -h, 1, 0, 0);
    PushVertex(md, h, h, h, 1, 0, 0);
    // Left face (-X)
    PushVertex(md, -h, -h, -h, -1, 0, 0);
    PushVertex(md, -h, -h, h, -1, 0, 0);
    PushVertex(md, -h, h, h, -1, 0, 0);
    PushVertex(md, -h, h, -h, -1, 0, 0);

    // Two triangles per face, CCW winding.
    for (uint16_t face = 0; face < 6; ++face)
    {
        uint16_t base = face * 4;
        md.indices.push_back(base + 0);
        md.indices.push_back(base + 1);
        md.indices.push_back(base + 2);
        md.indices.push_back(base + 0);
        md.indices.push_back(base + 2);
        md.indices.push_back(base + 3);
    }

    return md;
}

// ---------------------------------------------------------------------------
// Sphere — UV-sphere with smooth normals
// ---------------------------------------------------------------------------

MeshData Primitives::CreateSphere(float radius, uint32_t slices, uint32_t stacks)
{
    MeshData md;

    // Top pole
    PushVertex(md, 0, radius, 0, 0, 1, 0);

    for (uint32_t i = 1; i < stacks; ++i)
    {
        float phi = XM_PI * static_cast<float>(i) / static_cast<float>(stacks);
        float sinPhi = sinf(phi);
        float cosPhi = cosf(phi);

        for (uint32_t j = 0; j <= slices; ++j)
        {
            float theta = XM_2PI * static_cast<float>(j) / static_cast<float>(slices);
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            float nx = sinPhi * cosTheta;
            float ny = cosPhi;
            float nz = sinPhi * sinTheta;

            PushVertex(md, nx * radius, ny * radius, nz * radius, nx, ny, nz);
        }
    }

    // Bottom pole
    PushVertex(md, 0, -radius, 0, 0, -1, 0);

    // --- Indices ---

    // Top cap: pole (vertex 0) to first ring
    for (uint32_t j = 0; j < slices; ++j)
    {
        md.indices.push_back(0);
        md.indices.push_back(static_cast<uint16_t>(1 + j));
        md.indices.push_back(static_cast<uint16_t>(1 + j + 1));
    }

    // Middle stacks
    uint32_t ringVertCount = slices + 1;
    for (uint32_t i = 0; i < stacks - 2; ++i)
    {
        for (uint32_t j = 0; j < slices; ++j)
        {
            uint16_t a = static_cast<uint16_t>(1 + i * ringVertCount + j);
            uint16_t b = static_cast<uint16_t>(1 + i * ringVertCount + j + 1);
            uint16_t c = static_cast<uint16_t>(1 + (i + 1) * ringVertCount + j);
            uint16_t d = static_cast<uint16_t>(1 + (i + 1) * ringVertCount + j + 1);

            md.indices.push_back(a);
            md.indices.push_back(c);
            md.indices.push_back(b);

            md.indices.push_back(b);
            md.indices.push_back(c);
            md.indices.push_back(d);
        }
    }

    // Bottom cap: last ring to bottom pole
    uint16_t bottomPole = static_cast<uint16_t>(md.vertices.size() - 1);
    uint16_t baseIndex = static_cast<uint16_t>(bottomPole - ringVertCount);
    for (uint32_t j = 0; j < slices; ++j)
    {
        md.indices.push_back(bottomPole);
        md.indices.push_back(static_cast<uint16_t>(baseIndex + j + 1));
        md.indices.push_back(static_cast<uint16_t>(baseIndex + j));
    }

    return md;
}

// ---------------------------------------------------------------------------
// Plane — flat quad on the XZ plane, normal pointing +Y
// ---------------------------------------------------------------------------

MeshData Primitives::CreatePlane(float width, float depth)
{
    MeshData md;
    float hw = width * 0.5f;
    float hd = depth * 0.5f;

    PushVertex(md, -hw, 0, hd, 0, 1, 0);
    PushVertex(md, hw, 0, hd, 0, 1, 0);
    PushVertex(md, hw, 0, -hd, 0, 1, 0);
    PushVertex(md, -hw, 0, -hd, 0, 1, 0);

    md.indices = {0, 1, 2, 0, 2, 3};

    return md;
}

// ---------------------------------------------------------------------------
// Cylinder — open-ended cylinder along the Y axis
// ---------------------------------------------------------------------------

MeshData Primitives::CreateCylinder(float radius, float height, uint32_t slices)
{
    MeshData md;
    float halfH = height * 0.5f;

    // --- Side vertices (two rings) ---
    for (uint32_t i = 0; i <= slices; ++i)
    {
        float theta = XM_2PI * static_cast<float>(i) / static_cast<float>(slices);
        float c = cosf(theta);
        float s = sinf(theta);

        // Bottom ring
        PushVertex(md, c * radius, -halfH, s * radius, c, 0, s);
        // Top ring
        PushVertex(md, c * radius, halfH, s * radius, c, 0, s);
    }

    // Side indices
    for (uint32_t i = 0; i < slices; ++i)
    {
        uint16_t bl = static_cast<uint16_t>(i * 2);
        uint16_t tl = static_cast<uint16_t>(i * 2 + 1);
        uint16_t br = static_cast<uint16_t>((i + 1) * 2);
        uint16_t tr = static_cast<uint16_t>((i + 1) * 2 + 1);

        md.indices.push_back(bl);
        md.indices.push_back(tl);
        md.indices.push_back(br);

        md.indices.push_back(tl);
        md.indices.push_back(tr);
        md.indices.push_back(br);
    }

    // --- Top cap ---
    uint16_t topCenter = static_cast<uint16_t>(md.vertices.size());
    PushVertex(md, 0, halfH, 0, 0, 1, 0);
    for (uint32_t i = 0; i <= slices; ++i)
    {
        float theta = XM_2PI * static_cast<float>(i) / static_cast<float>(slices);
        PushVertex(md, cosf(theta) * radius, halfH, sinf(theta) * radius, 0, 1, 0);
    }
    for (uint32_t i = 0; i < slices; ++i)
    {
        md.indices.push_back(topCenter);
        md.indices.push_back(static_cast<uint16_t>(topCenter + 1 + i + 1));
        md.indices.push_back(static_cast<uint16_t>(topCenter + 1 + i));
    }

    // --- Bottom cap ---
    uint16_t botCenter = static_cast<uint16_t>(md.vertices.size());
    PushVertex(md, 0, -halfH, 0, 0, -1, 0);
    for (uint32_t i = 0; i <= slices; ++i)
    {
        float theta = XM_2PI * static_cast<float>(i) / static_cast<float>(slices);
        PushVertex(md, cosf(theta) * radius, -halfH, sinf(theta) * radius, 0, -1, 0);
    }
    for (uint32_t i = 0; i < slices; ++i)
    {
        md.indices.push_back(botCenter);
        md.indices.push_back(static_cast<uint16_t>(botCenter + 1 + i));
        md.indices.push_back(static_cast<uint16_t>(botCenter + 1 + i + 1));
    }

    return md;
}

// ---------------------------------------------------------------------------
// Torus — ring around the Y axis
// ---------------------------------------------------------------------------

MeshData Primitives::CreateTorus(float majorRadius, float minorRadius, uint32_t majorSegments, uint32_t minorSegments)
{
    MeshData md;

    // Generate vertices
    for (uint32_t i = 0; i <= majorSegments; ++i)
    {
        float u = XM_2PI * static_cast<float>(i) / static_cast<float>(majorSegments);
        float cosU = cosf(u);
        float sinU = sinf(u);

        for (uint32_t j = 0; j <= minorSegments; ++j)
        {
            float v = XM_2PI * static_cast<float>(j) / static_cast<float>(minorSegments);
            float cosV = cosf(v);
            float sinV = sinf(v);

            float px = (majorRadius + minorRadius * cosV) * cosU;
            float py = minorRadius * sinV;
            float pz = (majorRadius + minorRadius * cosV) * sinU;

            // Normal points from the tube center toward the surface
            float nx = cosV * cosU;
            float ny = sinV;
            float nz = cosV * sinU;

            PushVertex(md, px, py, pz, nx, ny, nz);
        }
    }

    // Generate indices
    uint32_t ringVerts = minorSegments + 1;
    for (uint32_t i = 0; i < majorSegments; ++i)
    {
        for (uint32_t j = 0; j < minorSegments; ++j)
        {
            uint16_t a = static_cast<uint16_t>(i * ringVerts + j);
            uint16_t b = static_cast<uint16_t>(i * ringVerts + j + 1);
            uint16_t c = static_cast<uint16_t>((i + 1) * ringVerts + j);
            uint16_t d = static_cast<uint16_t>((i + 1) * ringVerts + j + 1);

            md.indices.push_back(a);
            md.indices.push_back(b);
            md.indices.push_back(c);

            md.indices.push_back(b);
            md.indices.push_back(d);
            md.indices.push_back(c);
        }
    }

    return md;
}

// ---------------------------------------------------------------------------
// TorusKnot — tube swept along a (p,q) torus-knot spine
// ---------------------------------------------------------------------------

MeshData Primitives::CreateTorusKnot(int p, int q, float majorRadius, float minorRadius, float tubeRadius,
                                     uint32_t pathSegments, uint32_t tubeSegments)
{
    MeshData md;

    const float fp = static_cast<float>(p);
    const float fq = static_cast<float>(q);

    // Spine lying flat in the XZ plane:
    // P(t) = ((R + r*cos(q*t))*cos(p*t),
    //          r*sin(q*t),
    //         (R + r*cos(q*t))*sin(p*t))
    auto spinePoint = [&](float t) -> XMFLOAT3
    {
        float rm = majorRadius + minorRadius * cosf(fq * t);
        return {rm * cosf(fp * t), minorRadius * sinf(fq * t), rm * sinf(fp * t)};
    };

    // Analytical tangent dP/dt (avoids finite-difference drift at the seam)
    auto spineTangent = [&](float t) -> XMFLOAT3
    {
        float cosQt = cosf(fq * t), sinQt = sinf(fq * t);
        float cosPt = cosf(fp * t), sinPt = sinf(fp * t);
        float rm  = majorRadius + minorRadius * cosQt;
        float drm = -minorRadius * fq * sinQt; // d(rm)/dt
        return {drm * cosPt - fp * rm * sinPt,
                minorRadius * fq * cosQt,
                drm * sinPt + fp * rm * cosPt};
    };

    // Generate exactly pathSegments rings — no duplicate first/last ring.
    // Seam closure is handled by modulo wrapping in the index loop below.
    for (uint32_t i = 0; i < pathSegments; ++i)
    {
        float t = XM_2PI * static_cast<float>(i) / static_cast<float>(pathSegments);

        XMFLOAT3 s0f  = spinePoint(t);
        XMFLOAT3 tanf = spineTangent(t);
        XMVECTOR s0   = XMLoadFloat3(&s0f);
        XMVECTOR T    = XMVector3Normalize(XMLoadFloat3(&tanf));

        // Frame: project the outward radial direction perpendicular to T,
        // then derive normal as cross(B, T).
        XMVECTOR radial = XMVector3Normalize(s0);
        XMVECTOR B      = XMVector3Normalize(XMVector3Cross(T, radial));
        XMVECTOR N      = XMVector3Cross(B, T); // already unit length

        // Generate tubeSegments vertices per ring — no duplicate first/last vertex.
        // Seam closure handled by modulo wrapping below.
        for (uint32_t j = 0; j < tubeSegments; ++j)
        {
            float phi    = XM_2PI * static_cast<float>(j) / static_cast<float>(tubeSegments);
            float cosPhi = cosf(phi);
            float sinPhi = sinf(phi);

            XMVECTOR offset = XMVectorAdd(XMVectorScale(N, tubeRadius * cosPhi),
                                          XMVectorScale(B, tubeRadius * sinPhi));
            XMVECTOR pos    = XMVectorAdd(s0, offset);
            XMVECTOR norm   = XMVector3Normalize(offset);

            XMFLOAT3 pf, nf;
            XMStoreFloat3(&pf, pos);
            XMStoreFloat3(&nf, norm);
            PushVertex(md, pf.x, pf.y, pf.z, nf.x, nf.y, nf.z);
        }
    }

    // Wrap both axes with modulo — no seam from duplicate vertices
    for (uint32_t i = 0; i < pathSegments; ++i)
    {
        uint32_t ni = (i + 1) % pathSegments;
        for (uint32_t j = 0; j < tubeSegments; ++j)
        {
            uint32_t nj = (j + 1) % tubeSegments;
            uint16_t a  = static_cast<uint16_t>(i  * tubeSegments + j);
            uint16_t b  = static_cast<uint16_t>(i  * tubeSegments + nj);
            uint16_t c  = static_cast<uint16_t>(ni * tubeSegments + j);
            uint16_t d  = static_cast<uint16_t>(ni * tubeSegments + nj);

            md.indices.push_back(a);
            md.indices.push_back(b);
            md.indices.push_back(c);

            md.indices.push_back(b);
            md.indices.push_back(d);
            md.indices.push_back(c);
        }
    }

    return md;
}
