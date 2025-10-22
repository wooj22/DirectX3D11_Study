#pragma once
#include <vector>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include "../WinBase/D3D.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

// Vertex
struct Vertex
{
    Vector3 position;   
    Vector3 normal;     
    Vector3 tangent;    
    Vector3 bitangent;  
    Vector2 texcoord;   

    Vertex() = default;

    Vertex(const Vector3& pos, const Vector3& n, const Vector3& t,
        const Vector3& b, const Vector2& uv)
        : position(pos), normal(n), tangent(t),  bitangent(b),  texcoord(uv) 
    {   }
};

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
    void Create();
};

