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
* [ Rigid  Skeletal Sub Mesh ]
* Rigid Mesh에서 머티리얼에 1:1 대응하는 서브 메시 정보.
* StaticMesh와 다르게 LocalSpace기준으로 저장되며, ModelSpace 변환을 위해 matrix정보를 추가로 가지고있다.
* 각 메시에 animation을 저장하기 위해 매핑할 nodeName도 함께 저장한다.
*/

class RigidSubMesh
{
public:
    // data
    string nodeName;
    vector<Vertex> vertices;
    vector<WORD> indices;
    unsigned int materialIndex;
	int parentIndex;

    UINT vertexBufferStride;
    UINT vertexBufferOffset;
    UINT indexCount = 0;

    // renderpipeline
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;

    // transform
    Matrix bindMatrix;
    Matrix localMatrix;   
    Matrix modelMatrix;   

public:
    RigidSubMesh();

    // create buffer
    void CreateBuffer();
};

