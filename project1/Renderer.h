#pragma once
#include "D3D12Lite.h"

using namespace D3D12Lite;

class Renderer {
private:

    // Member Variable for Render
    std::unique_ptr<Device> mDevice;
    std::unique_ptr<GraphicsContext> mGraphicsContext;

    // Member variables for Triangle
    std::unique_ptr<BufferResource> mTriangleVertexBuffer;
    std::unique_ptr<BufferResource> mTriangleConstantBuffer;
    std::unique_ptr<Shader> mTriangleVertexShader;
    std::unique_ptr<Shader> mTrianglePixelShader;
    std::unique_ptr<PipelineStateObject> mTrianglePSO;
    PipelineResourceSpace mTrianglePerObjectSpace;

    // Member variables for Meshes
    std::unique_ptr<TextureResource> mDepthBuffer;
    std::unique_ptr<TextureResource> mWoodTexture;
    std::unique_ptr<BufferResource> mMeshVertexBuffer;
    std::array<std::unique_ptr<BufferResource>, NUM_FRAMES_IN_FLIGHT> mMeshConstantBuffers;
    std::unique_ptr<BufferResource> mMeshPassConstantBuffer;
    PipelineResourceSpace mMeshPerObjectResourceSpace;
    PipelineResourceSpace mMeshPerPassResourceSpace;
    std::unique_ptr<Shader> mMeshVertexShader;
    std::unique_ptr<Shader> mMeshPixelShader;
    std::unique_ptr<PipelineStateObject> mMeshPSO;

public:
    Renderer(HWND windowHandle, Uint2 screenSize);
    ~Renderer();

    void InitializeImGui(HWND windowHandle);
    void RenderImGui();

    void RenderClearColorTutorial();

    void InitializeTriangleResources();
    void RenderTriangleTutorial();

    void InitializeMeshResources();
    void RenderMeshTutorial();

    void Render();
};