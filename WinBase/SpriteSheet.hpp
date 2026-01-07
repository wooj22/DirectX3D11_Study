#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// UV Animation¿ª ¿ß«— SpriteSheet Resource
struct SpriteSheet
{
    ComPtr<ID3D11ShaderResourceView> srv = nullptr;
    int cols;   
    int rows;
    float fps;
};
