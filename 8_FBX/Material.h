#pragma once
#include "../WinBase/D3D.h"
#include <string>
using std::wstring;

/* 
* 비트 플래그
  - 하나의 정수(int) 안에 여러 개의 상태(on/off) 를 비트 단위로 저장하는 방법
  - 비트 켜기 : flags |= (1 << n);
  - 비트 켜진지 확인 : if (flags & (1 << n))
 */
enum TextureFlags
{
	TEX_DIFFUSE = 1 << 0,		// 0001
	TEX_NORMAL = 1 << 1,		// 0010
	TEX_SPECULAR = 1 << 2,		// 0100
	TEX_EMISSIVE = 1 << 3		// 1000
};

/*
* [ Material ]
*
*/
class Material
{
public:
	// directory path
	wstring directory = L"../Resource/";

	// texture flag
	UINT textureFlags = 0;

	// file path
	wstring diffuse_filename = L"";
	wstring normal_filename = L"";
	wstring specular_filename = L"";
	wstring emissive_filename = L"";
	
	// texture
	ID3D11ShaderResourceView* diffuseSRV = nullptr;			
	ID3D11ShaderResourceView* normalSRV = nullptr;			
	ID3D11ShaderResourceView* specualrSRV = nullptr;	
	ID3D11ShaderResourceView* emissiveSRV = nullptr;

public:
	void Create();	// create srv
	void SetDirectoryPath(wstring& path);
};

