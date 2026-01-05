#include "BloomRenderer.h"
#include "D3D.h"
#include "Structures.hpp"

void BloomRenderer::BloomPass()
{
    // clear
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
    D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
    D3D::deviceContext->PSSetShaderResources(14, 1, nullSRV);

    ID3D11RenderTargetView* nullRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
    D3D::deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRTVs, nullptr);

    // Full Screen VS
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->IASetInputLayout(nullptr);
    D3D::deviceContext->VSSetShader(D3D::VS_FullScreen.Get(), NULL, 0);

    // 1. Prefilter Pass ------------------------------------
    //  - HDR을 샘플링하여 BloomA의 mip0에 처리할 픽셀만 기록
    //  - sceneHDR read -> mip0 write
    {
        // View Port
        D3D::SetViewportForMip(D3D::bloomW, D3D::bloomH, 0);

        // output : A.mip0
        D3D::deviceContext->OMSetRenderTargets(1, D3D::bloomARTVs[0].GetAddressOf(), nullptr);
        D3D::deviceContext->ClearRenderTargetView(D3D::bloomARTVs[0].Get(), clearColor);

        // input : sceneHDR
        D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
        D3D::deviceContext->PSSetShaderResources(12, 1, D3D::sceneHDRSRV.GetAddressOf());

        // CB
        UINT w0, h0;
        D3D::GetMipSize(D3D::bloomW, D3D::bloomH, 0, w0, h0);
        D3D::bloomCBData.srcMip = 0;
        D3D::bloomCBData.srcTexelSize = DirectX::XMFLOAT2(1.0f / (float)w0, 1.0f / (float)h0);
        D3D::deviceContext->UpdateSubresource(D3D::bloomBuffer.Get(), 0, nullptr, &D3D::bloomCBData, 0, 0);

        // Draw Call
        D3D::deviceContext->PSSetShader(D3D::PS_BloomPrefilter.Get(), NULL, 0);
        D3D::deviceContext.Get()->Draw(3, 0);

        // cleanup
        D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
    }


    // 2. DownSample Blur Pass ------------------------------
    //  - BloomA mip0을 시작으로 Mip Chain 형성 + 블러 처리한다.
    //  - BloomA와 BloomB를 SRV와 RTV로 ping-pong하며 read & write 교대
    //  - mip(i-1) read -> mip(i) write
    {
        for (int i = 1; i < D3D::bloomMipCount; ++i)
        {
            // RTV UnBind
            D3D::deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRTVs, nullptr);

            // View Port
            D3D::SetViewportForMip(D3D::bloomW, D3D::bloomH, i);

            // Ping Pong
            // i 홀수 : A(SRV) -> B(RTV)
            // i 짝수 : B(SRV) -> A(RTV)
            bool AtoB = (i % 2 != 0) ? true : false;

            // input : A or B mip(i-1)
            ID3D11ShaderResourceView* bloomSRV = AtoB ? D3D::bloomASRV.Get() : D3D::bloomBSRV.Get();
            D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
            D3D::deviceContext->PSSetShaderResources(13, 1, &bloomSRV);

            // output : A or B mip(i)
            ID3D11RenderTargetView* bloomRTV = AtoB ? D3D::bloomBRTVs[i].Get() : D3D::bloomARTVs[i].Get();
            D3D::deviceContext->OMSetRenderTargets(1, &bloomRTV, nullptr);
            D3D::deviceContext->ClearRenderTargetView(bloomRTV, clearColor);

            // CB
            UINT sw, sh;
            D3D::GetMipSize(D3D::bloomW, D3D::bloomH, i - 1, sw, sh);
            D3D::bloomCBData.srcTexelSize = DirectX::XMFLOAT2(1.0f / (float)sw, 1.0f / (float)sh);
            D3D::bloomCBData.srcMip = (float)(i - 1);
            D3D::deviceContext->UpdateSubresource(D3D::bloomBuffer.Get(), 0, nullptr, &D3D::bloomCBData, 0, 0);

            // Draw Call
            D3D::deviceContext->PSSetShader(D3D::PS_BloomDownsampleBlur.Get(), NULL, 0);
            D3D::deviceContext.Get()->Draw(3, 0);

            // cleanup
            D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
        }
    }

    // 3. UpSample Combine Pass -----------------------------
    //  - mipN에서 mip0으로 올라오며 업샘플 + 가산합성하여 최종 블룸 이미지를 도출한다.
    //  - DownSample 결과의 시작 누적(accum)은 last mip이 들어있는 텍스처에서 시작한다.
    //  - BloomA, BloomB : read only
    //  - AccumA, AccumB : small(accum) read, out write
    //  - mip(i, i+1) read -> mip(i)  write
    {
        // LastMip은 bloomMipCount-1이 홀수면 B, 짝수면 A에 있음 (downpass에서 핑퐁했기 때문에)
        int lastMipLevel = (int)D3D::bloomMipCount - 1;
        bool lastMipOnBloomB = (lastMipLevel % 2) != 0;

        // 첫 루프에서는 accum에 아직 누적 텍스처가 없으므로, bloom에서 big과 small 가져온다.
        bool accumOnB = false;  // bloomA를 small로 시작 (2개를 한번에 합할거라 순서 상관 x)

        for (int i = lastMipLevel - 1; i >= 0; --i)
        {
            // cleanup
            D3D::deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRTVs, nullptr);
            D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
            D3D::deviceContext->PSSetShaderResources(14, 1, nullSRV);

            // View Port
            D3D::SetViewportForMip(D3D::bloomW, D3D::bloomH, i);

            // 1) Big SRV : 더 큰 해상도의 mip이 있는 SRV
            ID3D11ShaderResourceView* bigSRV = nullptr;
            if (i == 0) bigSRV = D3D::bloomASRV.Get();      // prefilter 단계에서 mip0은 BloomA에 저장함
            else bigSRV = ((i % 2) != 0) ? D3D::bloomBSRV.Get() : D3D::bloomASRV.Get();

            // 2) Small SRV : 가산 누적된 mip이 있는 SRV (업샘플 소스)
            // 첫 루프에서는 accum에 아직 가산한 누적 텍스처가 없으므로, small = Bloom A or B의 last mip
            // 다음부터는 small = AccumA or AccumB의 mip(i+1)
            ID3D11ShaderResourceView* smallSRV = nullptr;
            if (i == lastMipLevel - 1)
                smallSRV = lastMipOnBloomB ? D3D::bloomBSRV.Get() : D3D::bloomASRV.Get();
            else
                smallSRV = accumOnB ? D3D::accumBSRV.Get() : D3D::accumASRV.Get();

            // 3) out RTV : 현재 패스에서 기록할 texture. (smallSRV와 겹치면 안됨!)
            const bool outOnB = !accumOnB;
            ID3D11RenderTargetView* outRTV = outOnB ? D3D::accumBRTVs[i].Get() : D3D::accumARTVs[i].Get();

            // SRV, RTV Bind
            D3D::deviceContext->PSSetShaderResources(13, 1, &bigSRV);
            D3D::deviceContext->PSSetShaderResources(14, 1, &smallSRV);
            D3D::deviceContext->OMSetRenderTargets(1, &outRTV, nullptr);

            // CB
            UINT wi, hi;
            D3D::GetMipSize(D3D::bloomW, D3D::bloomH, (UINT)i, wi, hi);
            D3D::bloomCBData.srcTexelSize = DirectX::XMFLOAT2(1.0f / (float)wi, 1.0f / (float)hi);
            D3D::bloomCBData.srcMip = (float)i;
            D3D::deviceContext->UpdateSubresource(D3D::bloomBuffer.Get(), 0, nullptr, &D3D::bloomCBData, 0, 0);

            // Draw Call
            D3D::deviceContext->PSSetShader(D3D::PS_BloomUpsampleCombine.Get(), NULL, 0);
            D3D::deviceContext->Draw(3, 0);

            // cleanup
            D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
            D3D::deviceContext->PSSetShaderResources(14, 1, nullSRV);

            // 누적 위치 갱신
            accumOnB = outOnB;
        }

        // Final Bloom Texture
        // accumOnB가 가리키는 Accum 텍스처의 mip0 -> PostProcess에 활용
        D3D::finalBloomSRV = accumOnB ? D3D::accumBSRV.Get() : D3D::accumASRV.Get();
    }
}