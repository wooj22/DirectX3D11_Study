#pragma once
#include "Emitter.h"
#include <vector>
#include <directxtk/simplemath.h>
using std::vector;
using namespace DirectX::SimpleMath;



/*
    [ Effect ]

    이펙트는 시간에 따라 변하는 시각적 연출을 의미을 의미합니다.
    하나의 이펙트는 하나 이상의 Emitter로 구성됩니다.
    Effect 클래스에서는 Effect를 구성하는 Emitter를 udpate하고, 이펙트 전체의 생애주기를 관리합니다.

    파티클 시스템에 관한 자세한 설명은
    Emitter.h를 참고하세용
*/

class Effect
{
public:
    vector<Emitter> emitters;
    bool allFinished = true; 

    Vector3 position = Vector3::Zero;
    bool    enabled = true;
    bool    playing = true;
    bool    looping = true;

public:
    void Play();
    void Stop();
    void Update();
};


/*
    Effect는 Depth Write를 하지 않기 때문에
    CPU에서 카메라 기준 Back-to-Front 정렬 후 렌더링해야함
*/
struct FxSortItem
{
    const Effect* fx;
    float key;    // effect - camera dist (forward)
};