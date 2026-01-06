#include "GeometryRenderer.h"
#include "StaticModel.h"
#include "RigidModel.h"
#include "SkeletalModel.h"
#include "D3D.h"

void GeometryRenderer::GeometryPass(const Matrix& view, const Matrix& projection,
    const vector<StaticModel*>& static_models,
    const vector<RigidModel*>& rigid_models,
    const vector<SkeletalModel*>& skeletal_models)
{
    // RTV, DSV
    ID3D11RenderTargetView* gbuffers[] =
    {
        D3D::albedoRTV.Get(),
        D3D::normalRTV.Get(),
        D3D::metalRoughRTV.Get(),
        D3D::emissiveRTV.Get()
    };

    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);
    D3D::deviceContext->OMSetRenderTargets(4, gbuffers, D3D::depthStencilView.Get());
    D3D::deviceContext->ClearDepthStencilView(D3D::depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    D3D::deviceContext->OMSetDepthStencilState(D3D::defualtDSS.Get(), 0);

    for (int i = 0; i < 4; i++)
    {
        D3D::deviceContext->ClearRenderTargetView(gbuffers[i], clearColor);
    }

    // IA
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // CB
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::deviceContext->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0);

    // Static, Rigid Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_BaseLit_Static.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::PS_Gbuffer.Get(), NULL, 0);
    for (auto& m : static_models) m->Draw();
    for (auto& m : rigid_models) m->Draw();

    // Skeletal Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_BaseLit_Skinned.Get(), NULL, 0);
    for (auto& m : skeletal_models) m->Draw();

    // RTV - SRV hazard ¹æÁö
    D3D::deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}