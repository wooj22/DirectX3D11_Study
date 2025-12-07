static const float PI = 3.141592;
static const float Epsilon = 0.00001;

// D(h) (Normal Distribution Function) : Trowbridge-Reitz GGX
float NDFGGXTR(float3 normal, float3 halfVec, float roughness)
{
    float cosLh = dot(normal, halfVec);
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    
    float demon = PI * pow((cosLh * cosLh) * (alphaSq - 1) + 1, 2);
    
    return alphaSq / demon;
}

// F(v, h) Fresnel equation : Fresnel-Schlick approximation 
float3 FresnelSchlick(float3 halfVec, float3 viewVec, float3 F0) // F0 == relfection factor
{
    return F0 + (1.0 - F0) * pow(1.0 - saturate(dot(halfVec, viewVec)), 5.0);
}

// G(l,v,h) 
float GSchlickGGX(float3 norm, float3 viewVec, float k) // K : 블러 감소 인자
{
    float NdotV = dot(norm, viewVec);
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

// G(n, v, l, k) : Smith's Method
float GSmithMethod(float3 norm, float3 viewVec, float3 lightVec, float roughness)
{
    float kDir = (roughness + 1) * (roughness + 1); // for directional lighting
    //float kIBL = roughness* roughness / 2.0;          // IBL lighting
    kDir /= 8.0;
    
    return GSchlickGGX(norm, viewVec, kDir) * GSchlickGGX(norm, lightVec, kDir);
}