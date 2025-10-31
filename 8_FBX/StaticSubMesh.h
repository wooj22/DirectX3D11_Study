#pragma once
#include <vector>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include "../WinBase/D3D.h"
#include "Structures.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;


/*
* [ Static Sub Mesh ]
* StaticMesh의 부분 메시로, material과 1:1 대응되는 메시
*/

class StaticSubMesh
{
public:
    // data
	vector<Vertex> vertices;
	vector<WORD> indices;
	unsigned int materialIndex;

    UINT vertexBufferStride;
    UINT vertexBufferOffset;
    UINT indexCount = 0;

    // renderpipeline
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;

public:
    // create buffer
    void CreateBuffer();
};

