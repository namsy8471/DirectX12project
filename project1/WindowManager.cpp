#include <iostream>

#include "WindowManager.h"
#include "D3D12Lite.h"
#include "Shaders/Shared.h"
#include "Renderer.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

using namespace D3D12Lite;

WindowManager::WindowManager(const std::wstring& applicationName, const D3D12Lite::Uint2& windowSize)
    : mApplicationName(applicationName), mWindowSize(windowSize) { }

WindowManager::~WindowManager()
{
    if (mWindowHandle) {
        DestroyWindow(mWindowHandle);
        mWindowHandle = nullptr;
    }

    if (mModuleHandle) {
        UnregisterClass(mApplicationName.c_str(), mModuleHandle);
        mModuleHandle = nullptr;
    }
}

bool WindowManager::Initialize(HINSTANCE moduleHandle)
{
    mModuleHandle = moduleHandle;

    WNDCLASSEX wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowManager::WndProc;
    wc.hInstance = mModuleHandle;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = mApplicationName.c_str();
    wc.cbSize = sizeof(WNDCLASSEX);

    if (!RegisterClassEx(&wc))
    {
        DWORD error = GetLastError();
        std::wcerr << L"RegisterClassEx failed with error: " << error << std::endl;
        return false;
    }

    mWindowHandle = CreateWindowEx(WS_EX_APPWINDOW, mApplicationName.c_str(), mApplicationName.c_str(),
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED | WS_SIZEBOX,
        (GetSystemMetrics(SM_CXSCREEN) - mWindowSize.x) / 2, (GetSystemMetrics(SM_CYSCREEN) - mWindowSize.y) / 2, mWindowSize.x, mWindowSize.y,
        nullptr, nullptr, mModuleHandle, nullptr);

    if (!mWindowHandle)
    {
        DWORD error = GetLastError();
        std::wcerr << L"m_WindowHandle failed with error: " << error << std::endl;
        return false;
    }

    ShowWindow(mWindowHandle, SW_SHOW);
    SetForegroundWindow(mWindowHandle);
    SetFocus(mWindowHandle);
    ShowCursor(true);

    return true;
}

bool WindowManager::ProcessMessages()
{
    MSG msg = { 0 };
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
        {
            return false;
        }
    }

    return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowManager::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, umessage, wparam, lparam))
    {
        return true;
    }

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
