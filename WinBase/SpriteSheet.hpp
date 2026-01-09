#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// UV Animation을 위한 SpriteSheet
// Effect의 Resource로 사용됩니다.  -> SRV Binding, CB로 넘길 Data
struct SpriteSheet
{
    ComPtr<ID3D11ShaderResourceView> srv = nullptr;
    int cols = 1;
    int rows = 1;

    int frameCount = 1;             // 실제 사용할 프레임 수 (<= cols*rows)
    bool frameAnimation = true;     // filpbook loop

    float fps = 0.0f;
    float baseSizeScale = 1.0f;
};
