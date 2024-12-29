#include <iostream>

#include "WindowManager.h"
#include "D3D12Lite.h"
#include "Shaders/Shared.h"
#include "Renderer.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

using namespace D3D12Lite;

WindowManager::WindowManager(const std::wstring& applicationName, const D3D12Lite::Uint2& windowSize)
    : m_applicationName(applicationName), m_windowSize(windowSize) { }

WindowManager::~WindowManager()
{
    if (m_windowHandle) {
        DestroyWindow(m_windowHandle);
        m_windowHandle = nullptr;
    }

    if (m_moduleHandle) {
        UnregisterClass(m_applicationName.c_str(), m_moduleHandle);
        m_moduleHandle = nullptr;
    }
}

bool WindowManager::Initialize(HINSTANCE moduleHandle)
{
    m_moduleHandle = moduleHandle;

    WNDCLASSEX wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowManager::WndProc;
    wc.hInstance = m_moduleHandle;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = m_applicationName.c_str();
    wc.cbSize = sizeof(WNDCLASSEX);

    if (!RegisterClassEx(&wc))
    {
        DWORD error = GetLastError();
        std::wcerr << L"RegisterClassEx failed with error: " << error << std::endl;
        return false;
    }

    m_windowHandle = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName.c_str(), m_applicationName.c_str(),
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED | WS_SIZEBOX,
        (GetSystemMetrics(SM_CXSCREEN) - m_windowSize.x) / 2, (GetSystemMetrics(SM_CYSCREEN) - m_windowSize.y) / 2, m_windowSize.x, m_windowSize.y,
        nullptr, nullptr, m_moduleHandle, nullptr);

    if (!m_windowHandle)
    {
        DWORD error = GetLastError();
        std::wcerr << L"m_WindowHandle failed with error: " << error << std::endl;
        return false;
    }

    ShowWindow(m_windowHandle, SW_SHOW);
    SetForegroundWindow(m_windowHandle);
    SetFocus(m_windowHandle);
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
