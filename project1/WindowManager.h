#pragma once
#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "D3D12Lite.h"

class WindowManager
{
private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

	std::wstring m_applicationName;
	D3D12Lite::Uint2 m_windowSize;
	HWND m_windowHandle = nullptr;
	HINSTANCE m_moduleHandle = nullptr;

public:
	WindowManager(const std::wstring& applicationName, const D3D12Lite::Uint2& windowSize);
	~WindowManager();

	bool Initialize(HINSTANCE moduleHandle);
	bool ProcessMessages();
	HWND GetWindowHandle() const { return m_windowHandle; }
};

#endif // WINDOW_MANAGER_H
