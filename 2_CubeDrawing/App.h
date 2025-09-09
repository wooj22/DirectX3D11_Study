#pragma once
#include "../WinBase/WinApp.h"
#include "../D3DBase/D3DBase.h"
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <directxtk/simplemath.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
// 

class App : public WinApp
{
public:
	// rendering pipeline
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;

	// vertex info
	UINT vertexBufferStride = 0;				// ���ؽ� �ϳ��� ũ��
	UINT vertexBufferOffset = 0;				// ���ؽ� ������ ������
	UINT indexCount = 0;						// �ε��� ����

	// else
	float backGroundColor[4] = { 0, 0, 0, 1.0f };

public:
	// main process
	virtual bool OnInit() override;
	virtual void OnUninit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;

	// vertex, shader
	bool InitRenderPipeLine();
	void UninitRenderPipeLine();
};

