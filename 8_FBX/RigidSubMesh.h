#pragma once
#include <vector>
#include <string>
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
* [ Rigid Sub Mesh ]
* 
*/

class RigidSubMesh
{
public:
    // data
    string nodeName;
    vector<Vertex> vertices;
    vector<WORD> indices;
    unsigned int materialIndex;

    UINT vertexBufferStride;
    UINT vertexBufferOffset;
    UINT indexCount = 0;

    // renderpipeline
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;

    // transform
    Matrix localMatrix;   
    Matrix worldMatrix;   

public:
    // create buffer
    void Create();

    void UpdateLocalMatrix(const Matrix& animMatrix)
    {
		localMatrix = animMatrix;
    }

	void UpdateWorldMatrix(const Matrix& parentMatrix)
	{
		worldMatrix = localMatrix * parentMatrix;
	}
};

