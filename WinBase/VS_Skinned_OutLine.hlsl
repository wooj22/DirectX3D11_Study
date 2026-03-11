#include <shared.fxh>

// Skeletal Mesh OutLine Vertex Shader

PS_Position_INPUT main(VS_Weight_INPUT input)
{
    PS_Position_INPUT output = (PS_Position_INPUT) 0;

    // skinning
    float4x4 offsetPos[4];
    offsetPos[0] = mul(boneOffset[input.boneIndices.x], bonePose[input.boneIndices.x]);
    offsetPos[1] = mul(boneOffset[input.boneIndices.y], bonePose[input.boneIndices.y]);
    offsetPos[2] = mul(boneOffset[input.boneIndices.z], bonePose[input.boneIndices.z]);
    offsetPos[3] = mul(boneOffset[input.boneIndices.w], bonePose[input.boneIndices.w]);
    
    float4x4 weightedOffsetPose;
    weightedOffsetPose = mul(input.boneWeights.x, offsetPos[0]);
    weightedOffsetPose += mul(input.boneWeights.y, offsetPos[1]);
    weightedOffsetPose += mul(input.boneWeights.z, offsetPos[2]);
    weightedOffsetPose += mul(input.boneWeights.w, offsetPos[3]);
    
    // world
    Matrix finalWorld = mul(weightedOffsetPose, world);
    output.pos = mul(float4(input.pos, 1.0f), finalWorld);
    
    // normal 방향으로 팽창
    //float outlineThickness = 0.3f;
    float3 normal = normalize(mul(input.normal, (float3x3) finalWorld));
    output.pos.xyz += normal * outlineThickness;

    output.pos = mul(output.pos, view); 
    output.pos = mul(output.pos, projection);

    return output;
}