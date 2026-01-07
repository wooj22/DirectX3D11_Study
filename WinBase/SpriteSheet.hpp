#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// UV Animation을 위한 SpriteSheet
// Effect의 Resource로 사용됩니다.  -> SRV Binding, CB로 넘길 Data
struct SpriteSheet
{
    ComPtr<ID3D11ShaderResourceView> srv = nullptr;
    int cols;   
    int rows;
    float fps;
};
