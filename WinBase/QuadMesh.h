#pragma once
#include "Structures.hpp"
#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

/*
    [ Quad Mesh ]

    
*/

class QuadMesh
{
private:
    // VB, IB
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    UINT indexCount = 6;
    UINT stride = sizeof(QuadVertex);

public:
    void Draw();
};

