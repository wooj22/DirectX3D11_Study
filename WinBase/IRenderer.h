#pragma once

// Renderer (Pass) Interface

class IRenderer
{
public:
    virtual ~IRenderer() = default;
    virtual void Initialize() {}
    virtual void RenderPass() = 0;
};

