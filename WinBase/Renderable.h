#pragma once

// [ RenderBlendType ]
// 하이브리드 렌더링 방식의 분기로 사용됩니다.
enum class RenderBlendType
{
    Opaque,     // 불투명 -> Deffered
    Transparent // 투명 -> Forward
};


// Renderable
class Renderable
{
public:
    RenderBlendType blendType = RenderBlendType::Transparent;    // 렌더링 방식 (기본값: 불투명)
    virtual ~Renderable() = default;
};

