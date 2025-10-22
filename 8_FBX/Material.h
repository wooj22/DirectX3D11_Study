#pragma once
#include "../WinBase/D3D.h"
using namespace std;
#include <string>

/*
* [ Material ]
* 
*/
class Material
{
public:
	// directory path
	wstring directory = L"../Resource/";

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

