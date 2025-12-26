/*
    [ PBR Lighting Pass Pixel Shader ] 
    * Deferred Rendering *
    
*/

#include <shared.fxh>
#include <PBR_Common.fxh>
#include <PostFxCommon.fxh>

// --- Texture Bind Slot ------------------


// --- Sampler Bind Slot ------------------



float4 main(PS_FullScreen_Input input) : SV_TARGET
{

    return float4(1, 1, 1, 1);
}
