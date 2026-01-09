#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

/*
    [ Sprite Sheet ]

     파티클이 사용하는 텍스처 시트 정보
     하나의 텍스처 안에 여러 프레임이 그리드 형태로 배치되어 있으며,
     파티클 애니메이션에 사용된다. (다른데도 쓸 수 있긴 함)

     ** Sprite Sheet Animation을 사용할 경우 **
      cols = 가로 sprite 개수
      rows = 세로 sprite 개수
      frameCount = cols * rows
      fps = 초당 재생할 프레임 수

     ** 단일 Sprite일 경우 **
      cols = 1
      rows = 1
      frameCount = 1
      fps = 0.0f
*/

struct SpriteSheet
{
    ComPtr<ID3D11ShaderResourceView> srv = nullptr;
    int cols = 1;
    int rows = 1;
    float baseSizeScale = 1.0f;

    // filpbook
    int   frameCount = 1;           // 실제 사용할 프레임 수 (<= cols*rows)
    float fps = 0.0f;               // 초당 재생할 프레임 수 (frames per second)
    bool  loop = true;              // filpbook loop

    float GetFilpbookDuration()
    {
        return frameCount / fps;
    }
};
