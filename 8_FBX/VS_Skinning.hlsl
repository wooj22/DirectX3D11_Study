#include <shared.fxh>

PS_INPUT main(VS_Weight_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;

    // skinning

    float4x4 offsetPos[4];
    offsetPos[0] = mul(model, boneOffset[input.boneIndices.x]);
    offsetPos[1] = mul(model, boneOffset[input.boneIndices.y]);
    offsetPos[2] = mul(model, boneOffset[input.boneIndices.z]);
    offsetPos[3] = mul(model, boneOffset[input.boneIndices.w]);
    
    float4x4 weightedOffsetPose = mul(offsetPos[0], input.boneWeights.x);
    weightedOffsetPose += mul(offsetPos[1], input.boneWeights.y);
    weightedOffsetPose += mul(offsetPos[2], input.boneWeights.z);
    weightedOffsetPose += mul(offsetPos[3], input.boneWeights.w);
    
    // world
    //Matrix finalWorld = mul(weightedOffsetPose, world);
    
    // test code
    Matrix finalWorld = mul(model, world);
    
     // clip space
    output.pos = mul(float4(input.pos, 1.0f), finalWorld); // local -> world
    output.worldPos = output.pos.xyz;                 // (world pos ÀúÀå)
    output.pos = mul(output.pos, view);               // world -> view
    output.pos = mul(output.pos, projection);         // view -> clip
    
    // world TBN
    float3 tangent = normalize(mul(input.tangent, (float3x3) finalWorld));
    float3 bitangent = normalize(mul(input.bitangent, (float3x3) finalWorld));
    output.normal = normalize(mul(input.normal, (float3x3) finalWorld));
    output.TBN = float3x3(tangent, bitangent, output.normal);
    
    // uv
    output.texCoord = input.texCoord;

    return output;
}