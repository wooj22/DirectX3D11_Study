#pragma once
#include "../WinBase/WinApp.h"
#include "../D3DBase/D3DBase.h"
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


// Cube 메쉬를 렌더링하고, OutputMerger단계의 깊이 테스트를 확인하는 프로젝트입니다.
class App : public WinApp
{
private:
	// rendering pipeline 
	ID3D11Buffer*		    vertexBuffer = nullptr;	     // 정점 data
	ID3D11Buffer*			indexBuffer = nullptr;       // 정점 index data
	ID3D11Buffer*	        constantBuffer = nullptr;    // world, view, projection matrix data
	ID3D11InputLayout*		inputLayout = nullptr;
	ID3D11VertexShader*		vertexShader = nullptr;
	ID3D11PixelShader*		pixelShader = nullptr;
	ID3D11DepthStencilView*	depthStencilView = nullptr;  // 깊이, 스텐실 테스트 (depth 24bit, stencil 8bit)

	// vertex info
	UINT vertexBufferStride = 0;				// 버텍스 하나의 크기
	UINT vertexBufferOffset = 0;				// 버텍스 버퍼의 오프셋
	UINT indexCount = 0;						// 인덱스 개수

	// cube
	XMVECTOR cube1_position = { 0,0,0 };
	XMVECTOR cube2_position = { 3,0,0 };
	XMVECTOR cube3_position = { 2,0,0 };

	// camera
	XMVECTOR eye;
	XMVECTOR at;
	XMVECTOR up;
	// TODO
	/*
	e. 카메라의 FOV 각도(degree) 변경
	f. 카메라의 Near,Far 변경
	*/

	// matrix
	Matrix cube1_matrix;
	Matrix cube2_matrix;
	Matrix cube3_matrix;
	Matrix viewMatrix;
	Matrix projectionMatrix;

	// else
	float time = 0.0f;
	float clearColor[4] = { 0, 0, 0, 1.0f };

public:
	// main process
	virtual bool OnInit() override;
	virtual void OnUninit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;

	// rendering pipeline
	bool InitRenderPipeLine();
	void UninitRenderPipeLine();
};

