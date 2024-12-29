#include "D3D12Lite.h"
#include "WindowManager.h"
#include "Renderer.h"

using namespace D3D12Lite;

int main()
{
	std::wstring applicationName = L"D3D12 Tutorial";
	Uint2 windowSize = { 1600, 900 };
	HINSTANCE moduleHandle = GetModuleHandle(nullptr);

	std::unique_ptr<WindowManager> windowManager = std::make_unique<WindowManager>(applicationName, windowSize);
	if (!windowManager->Initialize(moduleHandle))
	{
		return - 1;
	}

	std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(windowManager->GetWindowHandle(), windowSize);

	bool isRunning = true;
	while (isRunning)
	{
		isRunning = windowManager->ProcessMessages();

		renderer->Render();
	}

	renderer = nullptr;

	return 0;
}