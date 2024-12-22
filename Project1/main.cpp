#include "D3D12Lite.h"
#include "Shaders/Shared.h"


using namespace D3D12Lite;

class Renderer{
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

public:
    Renderer(HWND windowHandle, Uint2 screenSize)
    {
        mDevice = std::make_unique<Device>(windowHandle, screenSize);
        mGraphicsContext = mDevice->CreateGraphicsContext();

        InitializeTriangleResources();
    }

    ~Renderer() {
        mDevice->WaitForIdle();
        mDevice->DestroyContext(std::move(mGraphicsContext));

        mDevice -> DestroyPipelineStateObject(std::move(mTrianglePSO));
        mDevice -> DestroyShader(std::move(mTriangleVertexShader));
        mDevice -> DestroyShader(std::move(mTrianglePixelShader));
        mDevice -> DestroyBuffer(std::move(mTriangleVertexBuffer));
        mDevice -> DestroyBuffer(std::move(mTriangleConstantBuffer));

        mDevice = nullptr;
    }

    void RenderClearColorTutorial()
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

    void InitializeTriangleResources()
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

    void RenderTriangleTutorial()
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

    void Render()
    {
        //RenderClearColorTutorial();
        RenderTriangleTutorial();
    }
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch (umessage)
    {
    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
        else
        {
            return DefWindowProc(hwnd, umessage, wparam, lparam);
        }

    case WM_DESTROY:
        [[fallthrough]];
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, umessage, wparam, lparam);
    }
}

int main()
{
    std::wstring applicationName = L"D3D12 Tutorial";
    Uint2 windowSize = { 1600, 900 };
    HINSTANCE moduleHandle = GetModuleHandle(nullptr);

    WNDCLASSEX wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = moduleHandle;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = applicationName.c_str();
    wc.cbSize = sizeof(WNDCLASSEX);
    RegisterClassEx(&wc);

    HWND windowHandle = CreateWindowEx(WS_EX_APPWINDOW, applicationName.c_str(), applicationName.c_str(),
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED | WS_SIZEBOX,
        (GetSystemMetrics(SM_CXSCREEN) - windowSize.x) / 2, (GetSystemMetrics(SM_CYSCREEN) - windowSize.y) / 2, windowSize.x, windowSize.y,
        nullptr, nullptr, moduleHandle, nullptr);

    ShowWindow(windowHandle, SW_SHOW);
    SetForegroundWindow(windowHandle);
    SetFocus(windowHandle);
    ShowCursor(true);

    std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(windowHandle, windowSize);

    bool shouldExit = false;
    while (!shouldExit)
    {
        MSG msg{ 0 };
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT)
        {
            shouldExit = true;
        }

        renderer->Render();
    }

    renderer = nullptr;

    DestroyWindow(windowHandle);
    windowHandle = nullptr;

    UnregisterClass(applicationName.c_str(), moduleHandle);
    moduleHandle = nullptr;

    return 0;
}