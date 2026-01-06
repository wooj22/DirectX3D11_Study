#include "FrustumRenderer.h"
#include "D3D.h"

void FrustumRenderer::Init(const Matrix& frustumView, const Matrix& frustumProj)
{
    m_states = std::make_unique<CommonStates>(D3D::device.Get());
    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(D3D::deviceContext.Get());
    m_effect = std::make_unique<BasicEffect>((D3D::device.Get()));
    m_effect->SetVertexColorEnabled(true);
    m_effect->SetView(frustumView);
    m_effect->SetProjection(frustumProj);
    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        D3D::device.Get()->CreateInputLayout(
            VertexPositionColor::InputElements, VertexPositionColor::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_layout.ReleaseAndGetAddressOf());
    }
}

void FrustumRenderer::FrustumDebugDraw(const Matrix& frustumView, const Matrix& frustumProj,
    const Matrix& renderView, const Matrix& renderProj, FXMVECTOR color)
{
    // Frustum Create
    BoundingFrustum frustum{};
    BoundingFrustum::CreateFromMatrix(frustum, frustumProj); // view space 기준
    Matrix invFrustumView = frustumView.Invert();
    frustum.Transform(frustum, invFrustumView);        // view -> world

    // Effect Update (render 기준은 항상 main camera)
    m_effect->SetWorld(Matrix::Identity);
    m_effect->SetView(renderView);
    m_effect->SetProjection(renderProj);
    m_effect->Apply(D3D::deviceContext.Get());

    // Stage Setting
    D3D::deviceContext.Get()->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    D3D::deviceContext.Get()->OMSetDepthStencilState(m_states->DepthNone(), 0);
    D3D::deviceContext.Get()->RSSetState(m_states->CullNone());
    D3D::deviceContext.Get()->IASetInputLayout(m_layout.Get());

    // Draw
    m_batch->Begin();
    Draw(m_batch.get(), frustum, color);
    m_batch->End();

    // UnBind
    const float blendFactor[4] = { 0,0,0,0 };
    D3D::deviceContext.Get()->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFFF);
    D3D::deviceContext.Get()->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext.Get()->RSSetState(nullptr);
}