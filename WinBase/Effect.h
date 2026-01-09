#pragma once
#include "Emitter.h"
#include <vector>
#include <directxtk/simplemath.h>
using std::vector;
using namespace DirectX::SimpleMath;



/*
    [ Effect ]

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


// Effect는 Depth write를 하지 않기 때문에
// CPU에서 Back-to-Front 정렬 후 DrawCall
struct FxSortItem
{
    const Effect* fx;
    float key;       // effect - camera dist (forward)
};