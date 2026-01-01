#include "LightVolumeMesh.h"// ---- LightVolumeMesh methods ----

// ---- 내부 헬퍼 (forward decl) ----
static void CreateVBIB(
    ID3D11Device* device,
    const std::vector<Position_Vertex>& verts,
    const std::vector<uint32_t>& indices,
    LightVolumeMesh& outMesh);

static void BuildUnitSphere(
    int slices, int stacks,
    std::vector<Position_Vertex>& outVerts,
    std::vector<uint32_t>& outIndices);

static void BuildUnitCone(
    int slices, bool capBase,
    std::vector<Position_Vertex>& outVerts,
    std::vector<uint32_t>& outIndices);

// ---- 외부 생성 함수 ----
LightVolumeMesh CreateLightVolumeSphere(ID3D11Device* device, int slices, int stacks)
{
    std::vector<Position_Vertex> verts;
    std::vector<uint32_t> indices;
    BuildUnitSphere(slices, stacks, verts, indices);

    LightVolumeMesh mesh;
    CreateVBIB(device, verts, indices, mesh);
    mesh.type = 0;
    return mesh;
}

LightVolumeMesh CreateLightVolumeCone(ID3D11Device* device, int slices, bool capBase)
{
    std::vector<Position_Vertex> verts;
    std::vector<uint32_t> indices;
    BuildUnitCone(slices, capBase, verts, indices);

    LightVolumeMesh mesh;
    CreateVBIB(device, verts, indices, mesh);
    mesh.type = 1;
    return mesh;
}

// ---- VB/IB 생성 ----
static void CreateVBIB(
    ID3D11Device* device,
    const std::vector<Position_Vertex>& verts,
    const std::vector<uint32_t>& indices,
    LightVolumeMesh& outMesh)
{
    outMesh = {};
    outMesh.indexCount = (UINT)indices.size();

    // VB
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = (UINT)(verts.size() * sizeof(Position_Vertex));
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vinit = {};
    vinit.pSysMem = verts.data();

    HRESULT hr = device->CreateBuffer(&vbd, &vinit, outMesh.vertexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        // 프로젝트 스타일대로 로그/어설트 처리
        // throw std::runtime_error("CreateBuffer VB failed");
    }

    // IB
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = (UINT)(indices.size() * sizeof(uint32_t));
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA iinit = {};
    iinit.pSysMem = indices.data();

    hr = device->CreateBuffer(&ibd, &iinit, outMesh.indexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        // 프로젝트 스타일대로 로그/어설트 처리
    }
}

// ---- Unit Sphere ----
static void BuildUnitSphere(
    int slices, int stacks,
    std::vector<Position_Vertex>& outVerts,
    std::vector<uint32_t>& outIndices)
{
    outVerts.clear();
    outIndices.clear();

    slices = (std::max)(3, slices);
    stacks = (std::max)(2, stacks);

    // Top
    outVerts.push_back({ {0.0f, 1.0f, 0.0f} });

    // Rings (excluding poles)
    for (int stack = 1; stack <= stacks - 1; ++stack)
    {
        float v = (float)stack / (float)stacks;        // 0..1
        float phi = v * 3.1415926535f;                 // 0..PI

        float y = std::cos(phi);
        float r = std::sin(phi);

        for (int slice = 0; slice <= slices; ++slice)
        {
            float u = (float)slice / (float)slices;
            float theta = u * 2.0f * 3.1415926535f;

            float x = r * std::cos(theta);
            float z = r * std::sin(theta);

            outVerts.push_back({ {x, y, z} });
        }
    }

    // Bottom
    outVerts.push_back({ {0.0f, -1.0f, 0.0f} });

    const uint32_t top = 0;
    const uint32_t bottom = (uint32_t)outVerts.size() - 1;
    const uint32_t ringVertCount = (uint32_t)(slices + 1);

    // Top cap
    uint32_t base = 1;
    for (int slice = 0; slice < slices; ++slice)
    {
        outIndices.push_back(top);
        outIndices.push_back(base + slice + 1);
        outIndices.push_back(base + slice);
    }

    // Middle
    for (int stack = 0; stack < stacks - 2; ++stack)
    {
        uint32_t ring0 = 1 + stack * ringVertCount;
        uint32_t ring1 = ring0 + ringVertCount;

        for (int slice = 0; slice < slices; ++slice)
        {
            outIndices.push_back(ring0 + slice);
            outIndices.push_back(ring1 + slice);
            outIndices.push_back(ring1 + slice + 1);

            outIndices.push_back(ring0 + slice);
            outIndices.push_back(ring1 + slice + 1);
            outIndices.push_back(ring0 + slice + 1);
        }
    }

    // Bottom cap
    uint32_t lastRing = 1 + (uint32_t)(stacks - 2) * ringVertCount;
    for (int slice = 0; slice < slices; ++slice)
    {
        outIndices.push_back(bottom);
        outIndices.push_back(lastRing + slice);
        outIndices.push_back(lastRing + slice + 1);
    }
}

// ---- Unit Cone (+Z) ----
static void BuildUnitCone(
    int slices, bool capBase,
    std::vector<Position_Vertex>& outVerts,
    std::vector<uint32_t>& outIndices)
{
    outVerts.clear();
    outIndices.clear();

    slices = (std::max)(3, slices);

    // Apex (0,0,0)
    const uint32_t apex = 0;
    outVerts.push_back({ {0.0f, 0.0f, 0.0f} });

    // Base ring (z=1), duplicate last vertex
    const uint32_t baseStart = (uint32_t)outVerts.size();
    for (int i = 0; i <= slices; ++i)
    {
        float u = (float)i / (float)slices;
        float theta = u * 2.0f * 3.1415926535f;

        float x = std::cos(theta);
        float y = std::sin(theta);

        outVerts.push_back({ {x, y, 1.0f} });
    }

    // Side triangles
    for (int i = 0; i < slices; ++i)
    {
        uint32_t v0 = apex;
        uint32_t v1 = baseStart + i;
        uint32_t v2 = baseStart + i + 1;

        // winding: 필요하면 v1/v2 swap
        outIndices.push_back(v0);
        outIndices.push_back(v1);
        outIndices.push_back(v2);
    }

    if (capBase)
    {
        uint32_t baseCenter = (uint32_t)outVerts.size();
        outVerts.push_back({ {0.0f, 0.0f, 1.0f} });

        for (int i = 0; i < slices; ++i)
        {
            uint32_t v1 = baseStart + i;
            uint32_t v2 = baseStart + i + 1;

            // base cap triangles (fan) - winding 주의
            outIndices.push_back(baseCenter);
            outIndices.push_back(v2);
            outIndices.push_back(v1);
        }
    }
}
