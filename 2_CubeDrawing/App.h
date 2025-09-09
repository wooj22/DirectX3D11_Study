#pragma once
#include "../WinBase/WinApp.h"
#include "../D3DBase/D3DBase.h"
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// Cube �޽��� �������ϰ�, OutputMerger�ܰ��� ���� �׽�Ʈ�� Ȯ���ϴ� ������Ʈ�Դϴ�.
class App : public WinApp
{
private:
	// rendering pipeline 
	ID3D11Buffer*		    vertexBuffer = nullptr;	     // ���� data
	ID3D11Buffer*			indexBuffer = nullptr;       // ���� index data
	ID3D11Buffer*	        constantBuffer = nullptr;    // world, view, projection matrix data
	ID3D11InputLayout*		inputLayout = nullptr;
	ID3D11VertexShader*		vertexShader = nullptr;
	ID3D11PixelShader*		pixelShader = nullptr;
	ID3D11DepthStencilView*	depthStencilView = nullptr;  // ����, ���ٽ� �׽�Ʈ (depth 24bit, stencil 8bit)

	// vertex info
	UINT vertexBufferStride = 0;				// ���ؽ� �ϳ��� ũ��
	UINT vertexBufferOffset = 0;				// ���ؽ� ������ ������
	UINT indexCount = 0;						// �ε��� ����

	// matrix
	Matrix cube1_matrix;
	Matrix cube2_matrix;
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

