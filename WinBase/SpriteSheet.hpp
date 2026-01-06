#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

struct SpriteSheet
{
    ComPtr<ID3D11ShaderResourceView> srv = nullptr;
    int cols;
    int rows;
    float fps;
};
