#pragma once
#include "D3D.h"
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
    TEX_DIFFUSE = 1 << 0,		// 000001
    TEX_NORMAL = 1 << 1,		// 000010
    TEX_SPECULAR = 1 << 2,		// 000100
    TEX_EMISSIVE = 1 << 3,   	// 001000
    TEX_ROUGHNESS = 1 << 4,     // 010000
    TEX_METALLIC = 1 << 5       // 100000
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
    wstring diffuse_filename = L"";         // 블린퐁(diffuse), PBR(albedo)
    wstring normal_filename = L"";          // 블린퐁, PBR
    wstring specular_filename = L"";        // 블린퐁
    wstring emissive_filename = L"";        // 블린퐁, PBR
    wstring roughness_filename = L"";       // PBR
    wstring metallic_filename = L"";        // PBR

    // texture
    ID3D11ShaderResourceView* diffuseSRV = nullptr;
    ID3D11ShaderResourceView* normalSRV = nullptr;
    ID3D11ShaderResourceView* specualrSRV = nullptr;
    ID3D11ShaderResourceView* emissiveSRV = nullptr;
    ID3D11ShaderResourceView* roughnessSRV = nullptr;
    ID3D11ShaderResourceView* metallicSRV = nullptr;

public:
    void CreateSRV();
    void SetDirectoryPath(wstring& path);
};

