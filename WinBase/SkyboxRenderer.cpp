#include "SkyboxRenderer.h"
#include "SkyBox.h"
#include "D3D.h"
#include "Structures.hpp"

// [ Skybox Render ]
// Deferred 렌더링에서 스카이박스는 Lighting Pass 이후에 렌더링되며
// 비어있는 픽셀에 기록한다. (Depth Test)
void SkyboxRenderer::SkyboxPass(const Matrix& view, const Matrix& projection, const SkyBox& skybox)
{
    // RTV, DSV
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);
    D3D::deviceContext->OMSetRenderTargets(1, D3D::sceneHDRRTV.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->OMSetDepthStencilState(D3D::depthTestOnlyDSS.Get(), 0);

    // IA
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Position.Get());

    // Shader
    D3D::deviceContext->VSSetShader(D3D::VS_Skybox.Get(), nullptr, 0);
    D3D::deviceContext->PSSetShader(D3D::PS_Skybox.Get(), nullptr, 0);

    // RS, DSS
    D3D::deviceContext->RSSetState(D3D::cullfrontRS.Get());
    D3D::deviceContext->OMSetDepthStencilState(D3D::depthTestOnlyDSS.Get(), 0);

    // CB
    /* 카메라 이동행렬 제거->카메라가 이동해도 큐브는 항상 카메라 원점에 고정
       스카이 박스 정점은 카메라 좌표계에서 항상 +- 1 정도 거리의 정점으로 유지
       큐브는 투영후 카메라의 Far Plane에 수렴하는 값으로 나오고 z(깊이)는 1근처가 됨 */
    Matrix viewNoTranslation = view;
    viewNoTranslation._41 = 0.0f;
    viewNoTranslation._42 = 0.0f;
    viewNoTranslation._43 = 0.0f;

    D3D::transformCBData.view = XMMatrixTranspose(viewNoTranslation);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::deviceContext->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0);

    // Draw
    skybox.Draw(view, projection);

    // clear
    D3D::deviceContext->RSSetState(nullptr);
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
}