#include "SkyBox.h"
#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

struct IBLResources
{
    ComPtr<ID3D11ShaderResourceView> irradiance = nullptr;   // diffuse IBL
    ComPtr<ID3D11ShaderResourceView> specularEnv = nullptr;  // prefiltered env
    ComPtr<ID3D11ShaderResourceView> brdfLut = nullptr;      // 2D LUT
};

struct Environment
{
    SkyBox skybox;
    IBLResources ibl;
};