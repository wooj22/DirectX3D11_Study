#include "PostProcessRenderer.h"
#include "D3D.h"
#include <d3d11.h>

// [ PostProcess Pass ]
// ToneMapping(LDR) + PostProcess
// Tone Mapping 패스는 화면을 덮는 FullScreen 사각형을 그리면서,
// HDR SRV를 샘플링해 색을 계산하고, 그 결과를 BackBuffer에 기록하는 단계
void PostProcessRenderer::PostProcessPass()
{
    // clear
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
    D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);

    ID3D11RenderTargetView* nullRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
    D3D::deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRTVs, nullptr);

    // RTV
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);
    D3D::deviceContext->OMSetRenderTargets(1, D3D::renderTargetView.GetAddressOf(), nullptr);
    D3D::deviceContext->ClearRenderTargetView(D3D::renderTargetView.Get(), clearColor);

    // IA
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->IASetInputLayout(nullptr);

    // Shader
    D3D::deviceContext->VSSetShader(D3D::VS_FullScreen.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::PS_PostProcess.Get(), NULL, 0);

    // Sampler
    D3D::deviceContext->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());

    // SRV
    D3D::deviceContext->PSSetShaderResources(12, 1, D3D::sceneHDRSRV.GetAddressOf());
    D3D::deviceContext->PSSetShaderResources(13, 1, D3D::finalBloomSRV.GetAddressOf());

    // Draw Call
    D3D::deviceContext.Get()->Draw(3, 0);

    // cleanup
    D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
}