#pragma once
#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "D3D12Lite.h"

class WindowManager
{
private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

	std::wstring mApplicationName;
	D3D12Lite::Uint2 mWindowSize;
	HWND mWindowHandle = nullptr;
	HINSTANCE mModuleHandle = nullptr;

public:
	WindowManager(const std::wstring& applicationName, const D3D12Lite::Uint2& windowSize);
	~WindowManager();

	bool Initialize(HINSTANCE moduleHandle);
	bool ProcessMessages();
	HWND GetWindowHandle() const { return mWindowHandle; }
};

#endif // WINDOW_MANAGER_H
