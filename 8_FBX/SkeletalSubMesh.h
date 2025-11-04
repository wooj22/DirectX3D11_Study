#pragma once
#include <vector>
#include <string>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include "../WinBase/D3D.h"
#include "Structures.h"
#include "Bone.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;


/*
* [ Skeletal Sub Mesh ]
*
* 
*/

class SkeletalSubMesh
{
public:
    // mesh data
    string                   nodeName;
    vector<BoneWeightVertex> vertices;
    vector<WORD>             indices;
    vector<Bone>             bones;
    unsigned int             materialIndex;
    
    // parent
    int parentIndex;

    UINT vertexBufferStride;
    UINT vertexBufferOffset;
    UINT indexCount = 0;

    // renderpipeline
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;

    // transform
    Matrix bindMatrix = Matrix::Identity;
    Matrix localMatrix = Matrix::Identity;
    Matrix modelMatrix = Matrix::Identity;

public:
    // create buffer
    void CreateBuffer();
};
