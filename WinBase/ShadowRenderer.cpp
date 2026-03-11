#include "ShadowRenderer.h"
#include "StaticModel.h"
#include "RigidModel.h"
#include "SkeletalModel.h"
#include "D3D.h"

void ShadowRenderer::ShadowMapPass(const Matrix& view, const Matrix& projection, 
    const vector<StaticModel*>& static_models,
    const vector<RigidModel*>& rigid_models,
    const vector<SkeletalModel*>& skeletal_models)
{
    // RTV, DSV
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_shadowMap);
    D3D::deviceContext->OMSetRenderTargets(0, nullptr, D3D::shadowDSV.Get());
    D3D::deviceContext->OMSetDepthStencilState(D3D::defualtDSS.Get(), 0);
    D3D::deviceContext->ClearDepthStencilView(D3D::shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // IA
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // PS
    D3D::deviceContext->PSSetShader(D3D::PS_ShadowDepth.Get(), NULL, 0);    // alpha discard

    // Sampler
    D3D::deviceContext->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());

    // CB
    D3D::transformCBData.shadowView = XMMatrixTranspose(view);
    D3D::transformCBData.shadowProjection = XMMatrixTranspose(projection);
    D3D::deviceContext->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0);

    // Static, Rigid Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_RigidVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_ShadowDepth_Rigid.Get(), NULL, 0);
    for (auto& m : static_models) m->Draw();
    for (auto& m : rigid_models) m->Draw();

    // Skeletal Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_ShadowDepth_Skeletal.Get(), NULL, 0);
    for (auto& m : skeletal_models) m->Draw();
}