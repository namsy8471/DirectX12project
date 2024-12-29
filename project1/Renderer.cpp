#include "Renderer.h"
#include "Shaders/Shared.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

Renderer::Renderer(HWND windowHandle, Uint2 screenSize)
{
    mDevice = std::make_unique<Device>(windowHandle, screenSize);
    mGraphicsContext = mDevice->CreateGraphicsContext();

    InitializeTriangleResources();
    InitializeMeshResources();
    InitializeImGui(windowHandle);
}

Renderer::~Renderer() {
    mDevice->WaitForIdle();
    mDevice->DestroyContext(std::move(mGraphicsContext));

    mDevice->DestroyPipelineStateObject(std::move(mTrianglePSO));
    mDevice->DestroyShader(std::move(mTriangleVertexShader));
    mDevice->DestroyShader(std::move(mTrianglePixelShader));
    mDevice->DestroyBuffer(std::move(mTriangleVertexBuffer));
    mDevice->DestroyBuffer(std::move(mTriangleConstantBuffer));

    mDevice = nullptr;
}

void Renderer::InitializeImGui(HWND windowHandle)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    Descriptor descriptor = mDevice->GetImguiDescriptor(0);
    Descriptor descriptor2 = mDevice->GetImguiDescriptor(1);

    ImGui_ImplWin32_Init(windowHandle);
    ImGui_ImplDX12_Init(mDevice->GetDevice(), NUM_FRAMES_IN_FLIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, nullptr,
        descriptor.mCPUHandle, descriptor.mGPUHandle, descriptor2.mCPUHandle, descriptor2.mGPUHandle);
}

void Renderer::RenderImGui()
{
    mDevice->BeginFrame();

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    ImGui::Button("Button");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
    ImGui::TextColored(ImVec4(0.00f, 0.00f, 0.00f, 1.00f), "Data Number");

    ImGui::Render();

    TextureResource& backBuffer = mDevice->GetCurrentBackBuffer();

    mGraphicsContext->Reset();
    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    mGraphicsContext->FlushBarriers();

    mGraphicsContext->ClearRenderTarget(backBuffer, Color(0.3f, 0.3f, 0.8f));

    PipelineInfo pipeline;
    pipeline.mPipeline = nullptr;
    pipeline.mRenderTargets.push_back(&backBuffer);

    mGraphicsContext->SetPipeline(pipeline);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mGraphicsContext->GetCommandList());

    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    mGraphicsContext->FlushBarriers();

    mDevice->SubmitContextWork(*mGraphicsContext);

    mDevice->EndFrame();
    mDevice->Present();
}

void Renderer::RenderClearColorTutorial()
{
    mDevice->BeginFrame();

    TextureResource& backBuffer = mDevice->GetCurrentBackBuffer();

    mGraphicsContext->Reset();

    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    mGraphicsContext->FlushBarriers();

    mGraphicsContext->ClearRenderTarget(backBuffer, Color(0.3f, 0.3f, 0.8f));

    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    mGraphicsContext->FlushBarriers();

    mDevice->SubmitContextWork(*mGraphicsContext);

    mDevice->EndFrame();
    mDevice->Present();
}

void Renderer::InitializeTriangleResources()
{
    std::array<TriangleVertex, 3> vertices;
    vertices[0].position = { -0.5f, -0.5f };
    vertices[0].color = { 1.0f, 0.0f, 0.0f };
    vertices[1].position = { 0.0f, 0.5f };
    vertices[1].color = { 0.0f, 1.0f, 0.0f };
    vertices[2].position = { 0.5f, -0.5f };
    vertices[2].color = { 0.0f, 0.0f, 1.0f };

    BufferCreationDesc triangleBufferDesc{};
    triangleBufferDesc.mSize = sizeof(vertices);
    triangleBufferDesc.mAccessFlags = BufferAccessFlags::hostWritable;
    triangleBufferDesc.mViewFlags = BufferViewFlags::srv;
    triangleBufferDesc.mStride = sizeof(TriangleVertex);
    triangleBufferDesc.mIsRawAccess = true;

    mTriangleVertexBuffer = mDevice->CreateBuffer(triangleBufferDesc);
    mTriangleVertexBuffer->SetMappedData(&vertices, sizeof(vertices));

    BufferCreationDesc triangleConstantDesc{};
    triangleConstantDesc.mSize = sizeof(TriangleConstants);
    triangleConstantDesc.mAccessFlags = BufferAccessFlags::hostWritable;
    triangleConstantDesc.mViewFlags = BufferViewFlags::cbv;

    TriangleConstants triangleConstants;
    triangleConstants.vertexBufferIndex = mTriangleVertexBuffer->mDescriptorHeapIndex;

    mTriangleConstantBuffer = mDevice->CreateBuffer(triangleConstantDesc);
    mTriangleConstantBuffer->SetMappedData(&triangleConstants, sizeof(TriangleConstants));

    ShaderCreationDesc triangleShaderVSDesc;
    triangleShaderVSDesc.mShaderName = L"Triangle.hlsl";
    triangleShaderVSDesc.mEntryPoint = L"VertexShader";
    triangleShaderVSDesc.mType = ShaderType::vertex;

    ShaderCreationDesc triangleShaderPSDesc;
    triangleShaderPSDesc.mShaderName = L"Triangle.hlsl";
    triangleShaderPSDesc.mEntryPoint = L"PixelShader";
    triangleShaderPSDesc.mType = ShaderType::pixel;

    mTriangleVertexShader = mDevice->CreateShader(triangleShaderVSDesc);
    mTrianglePixelShader = mDevice->CreateShader(triangleShaderPSDesc);

    mTrianglePerObjectSpace.SetCBV(mTriangleConstantBuffer.get());
    mTrianglePerObjectSpace.Lock();

    PipelineResourceLayout resourceLayout;
    resourceLayout.mSpaces[PER_OBJECT_SPACE] = &mTrianglePerObjectSpace;

    GraphicsPipelineDesc trianglePipelineDesc = GetDefaultGraphicsPipelineDesc();
    trianglePipelineDesc.mVertexShader = mTriangleVertexShader.get();
    trianglePipelineDesc.mPixelShader = mTrianglePixelShader.get();
    trianglePipelineDesc.mRenderTargetDesc.mNumRenderTargets = 1;
    trianglePipelineDesc.mRenderTargetDesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    mTrianglePSO = mDevice->CreateGraphicsPipeline(trianglePipelineDesc, resourceLayout);
}

void Renderer::RenderTriangleTutorial()
{
    mDevice->BeginFrame();

    TextureResource& backBuffer = mDevice->GetCurrentBackBuffer();

    mGraphicsContext->Reset();
    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    mGraphicsContext->FlushBarriers();

    mGraphicsContext->ClearRenderTarget(backBuffer, Color(0.3f, 0.3f, 0.8f));

    PipelineInfo pipeline;
    pipeline.mPipeline = mTrianglePSO.get();
    pipeline.mRenderTargets.push_back(&backBuffer);

    mGraphicsContext->SetPipeline(pipeline);
    mGraphicsContext->SetPipelineResources(PER_OBJECT_SPACE, mTrianglePerObjectSpace);
    mGraphicsContext->SetDefaultViewPortAndScissor(mDevice->GetScreenSize());
    mGraphicsContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mGraphicsContext->Draw(3);

    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    mGraphicsContext->FlushBarriers();

    mDevice->SubmitContextWork(*mGraphicsContext);

    mDevice->EndFrame();
    mDevice->Present();
}

void Renderer::InitializeMeshResources()
{
    MeshVertex meshVertices[36] = {
        { { 1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f },{ 0.0f, -1.0f, 0.0f } },
        { { 1.0f, -1.0f, -1.0f },{ 1.0f, 0.0f },{ 0.0f, -1.0f, 0.0f } },
        { { -1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f },{ 0.0f, -1.0f, 0.0f } },
        { { -1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f },{ 0.0f, -1.0f, 0.0f } },
        { { 1.0f, -1.0f, -1.0f },{ 1.0f, 0.0f },{ 0.0f, -1.0f, 0.0f } },
        { { -1.0f, -1.0f, -1.0f },{ 0.0f, 0.0f },{ 0.0f, -1.0f, 0.0f } },
        { { 1.0f, 1.0f, -1.0f },{ 1.0f, 1.0f },{ 0.0f, 1.0f, 0.0f } },
        { { 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
        { { -1.0f, 1.0f, -1.0f },{ 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f } },
        { { -1.0f, 1.0f, -1.0f },{ 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f } },
        { { 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
        { { -1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
        { { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f },{ -1.0f, 0.0f, 0.0f } },
        { { -1.0f, 1.0f, -1.0f },{ 1.0f, 0.0f },{ -1.0f, 0.0f, 0.0f } },
        { { -1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f },{ -1.0f, 0.0f, 0.0f } },
        { { -1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f },{ -1.0f, 0.0f, 0.0f } },
        { { -1.0f, 1.0f, -1.0f },{ 1.0f, 0.0f },{ -1.0f, 0.0f, 0.0f } },
        { { -1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f },{ -1.0f, 0.0f, 0.0f } },
        { { -1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
        { { -1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
        { { 1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
        { { 1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
        { { -1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
        { { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
        { { 1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f },{ 1.0f, 0.0f, 0.0f } },
        { { 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
        { { 1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f } },
        { { 1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f } },
        { { 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
        { { 1.0f, 1.0f, -1.0f },{ 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
        { { 1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f },{ 0.0f, 0.0f, -1.0f } },
        { { 1.0f, 1.0f, -1.0f },{ 1.0f, 0.0f },{ 0.0f, 0.0f, -1.0f } },
        { { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ 0.0f, 0.0f, -1.0f } },
        { { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ 0.0f, 0.0f, -1.0f } },
        { { 1.0f, 1.0f, -1.0f },{ 1.0f, 0.0f },{ 0.0f, 0.0f, -1.0f } },
        { { -1.0f, 1.0f, -1.0f },{ 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f } },
    };

    BufferCreationDesc meshVertexBufferDesc{};
    meshVertexBufferDesc.mSize = sizeof(meshVertices);
    meshVertexBufferDesc.mAccessFlags = BufferAccessFlags::gpuOnly;
    meshVertexBufferDesc.mViewFlags = BufferViewFlags::srv;
    meshVertexBufferDesc.mStride = sizeof(MeshVertex);
    meshVertexBufferDesc.mIsRawAccess = true;

    mMeshVertexBuffer = mDevice->CreateBuffer(meshVertexBufferDesc);

    auto bufferUpload = std::make_unique<BufferUpload>();
    bufferUpload->mBuffer = mMeshVertexBuffer.get();
    bufferUpload->mBufferData = std::make_unique<uint8_t[]>(sizeof(meshVertices));
    bufferUpload->mBufferDataSize = sizeof(meshVertices);

    memcpy_s(bufferUpload->mBufferData.get(), sizeof(meshVertices), meshVertices, sizeof(meshVertices));

    mDevice->GetUploadContextForCurrentFrame().AddBufferUpload(std::move(bufferUpload));

    mWoodTexture = mDevice->CreateTextureFromFile("Wood.dds");

    MeshConstants meshConstants;
    meshConstants.vertexBufferIndex = mMeshVertexBuffer->mDescriptorHeapIndex;
    meshConstants.textureIndex = mWoodTexture->mDescriptorHeapIndex;

    BufferCreationDesc meshConstantDesc{};
    meshConstantDesc.mSize = sizeof(MeshConstants);
    meshConstantDesc.mAccessFlags = BufferAccessFlags::hostWritable;
    meshConstantDesc.mViewFlags = BufferViewFlags::cbv;

    for (uint32_t frameIndex = 0; frameIndex < NUM_FRAMES_IN_FLIGHT; frameIndex++)
    {
        mMeshConstantBuffers[frameIndex] = mDevice->CreateBuffer(meshConstantDesc);
        mMeshConstantBuffers[frameIndex]->SetMappedData(&meshConstants, sizeof(MeshConstants));
    }

    BufferCreationDesc meshPassConstantDesc{};
    meshPassConstantDesc.mSize = sizeof(MeshPassConstants);
    meshPassConstantDesc.mAccessFlags = BufferAccessFlags::hostWritable;
    meshPassConstantDesc.mViewFlags = BufferViewFlags::cbv;

    Uint2 screenSize = mDevice->GetScreenSize();

    float fieldOfView = 3.14159f / 4.0f;
    float aspectRatio = (float)screenSize.x / (float)screenSize.y;
    Vector3 cameraPosition = Vector3(-3.0f, 3.0f, -8.0f);

    MeshPassConstants passConstants;
    passConstants.viewMatrix = Matrix::CreateLookAt(cameraPosition, Vector3(0, 0, 0), Vector3(0, 1, 0));
    passConstants.projectionMatrix = Matrix::CreatePerspectiveFieldOfView(fieldOfView, aspectRatio, 0.001f, 1000.0f);
    passConstants.cameraPosition = cameraPosition;

    mMeshPassConstantBuffer = mDevice->CreateBuffer(meshPassConstantDesc);
    mMeshPassConstantBuffer->SetMappedData(&passConstants, sizeof(MeshPassConstants));

    TextureCreationDesc depthBufferDesc;
    depthBufferDesc.mResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthBufferDesc.mResourceDesc.Width = screenSize.x;
    depthBufferDesc.mResourceDesc.Height = screenSize.y;
    depthBufferDesc.mViewFlags = TextureViewFlags::srv | TextureViewFlags::dsv;

    mDepthBuffer = mDevice->CreateTexture(depthBufferDesc);

    ShaderCreationDesc meshShaderVSDesc;
    meshShaderVSDesc.mShaderName = L"Mesh.hlsl";
    meshShaderVSDesc.mEntryPoint = L"VertexShader";
    meshShaderVSDesc.mType = ShaderType::vertex;

    ShaderCreationDesc meshShaderPSDesc;
    meshShaderPSDesc.mShaderName = L"Mesh.hlsl";
    meshShaderPSDesc.mEntryPoint = L"PixelShader";
    meshShaderPSDesc.mType = ShaderType::pixel;

    mMeshVertexShader = mDevice->CreateShader(meshShaderVSDesc);
    mMeshPixelShader = mDevice->CreateShader(meshShaderPSDesc);

    GraphicsPipelineDesc meshPipelineDesc = GetDefaultGraphicsPipelineDesc();
    meshPipelineDesc.mVertexShader = mMeshVertexShader.get();
    meshPipelineDesc.mPixelShader = mMeshPixelShader.get();
    meshPipelineDesc.mRenderTargetDesc.mNumRenderTargets = 1;
    meshPipelineDesc.mRenderTargetDesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    meshPipelineDesc.mDepthStencilDesc.DepthEnable = true;
    meshPipelineDesc.mRenderTargetDesc.mDepthStencilFormat = depthBufferDesc.mResourceDesc.Format;
    meshPipelineDesc.mDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

    mMeshPerObjectResourceSpace.SetCBV(mMeshConstantBuffers[0].get());
    mMeshPerObjectResourceSpace.Lock();

    mMeshPerPassResourceSpace.SetCBV(mMeshPassConstantBuffer.get());
    mMeshPerPassResourceSpace.Lock();

    PipelineResourceLayout meshResourceLayout;
    meshResourceLayout.mSpaces[PER_OBJECT_SPACE] = &mMeshPerObjectResourceSpace;
    meshResourceLayout.mSpaces[PER_PASS_SPACE] = &mMeshPerPassResourceSpace;

    mMeshPSO = mDevice->CreateGraphicsPipeline(meshPipelineDesc, meshResourceLayout);
}

void Renderer::RenderMeshTutorial()
{
    mDevice->BeginFrame();

    TextureResource& backBuffer = mDevice->GetCurrentBackBuffer();

    mGraphicsContext->Reset();
    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    mGraphicsContext->AddBarrier(*mDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    mGraphicsContext->FlushBarriers();

    mGraphicsContext->ClearRenderTarget(backBuffer, Color(0.3f, 0.3f, 0.8f));
    mGraphicsContext->ClearDepthStencilTarget(*mDepthBuffer, 1.0f, 0);

    static float rotation = 0.0f;
    rotation += 0.0001f;

    if (mMeshVertexBuffer->mIsReady && mWoodTexture->mIsReady)
    {
        MeshConstants meshConstants;
        meshConstants.vertexBufferIndex = mMeshVertexBuffer->mDescriptorHeapIndex;
        meshConstants.textureIndex = mWoodTexture->mDescriptorHeapIndex;
        meshConstants.worldMatrix = Matrix::CreateRotationY(rotation);

        mMeshConstantBuffers[mDevice->GetFrameId()]->SetMappedData(&meshConstants, sizeof(MeshConstants));

        mMeshPerObjectResourceSpace.SetCBV(mMeshConstantBuffers[mDevice->GetFrameId()].get());

        PipelineInfo pipeline;
        pipeline.mPipeline = mMeshPSO.get();
        pipeline.mRenderTargets.push_back(&backBuffer);
        pipeline.mDepthStencilTarget = mDepthBuffer.get();

        mGraphicsContext->SetPipeline(pipeline);
        mGraphicsContext->SetPipelineResources(PER_OBJECT_SPACE, mMeshPerObjectResourceSpace);
        mGraphicsContext->SetPipelineResources(PER_PASS_SPACE, mMeshPerPassResourceSpace);
        mGraphicsContext->SetDefaultViewPortAndScissor(mDevice->GetScreenSize());
        mGraphicsContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mGraphicsContext->Draw(36);
    }

    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    mGraphicsContext->FlushBarriers();

    mDevice->SubmitContextWork(*mGraphicsContext);

    mDevice->EndFrame();
    mDevice->Present();
}

void Renderer::Render()
{
    //RenderClearColorTutorial();
    //RenderTriangleTutorial();
    //RenderMeshTutorial();
    RenderImGui();
}
