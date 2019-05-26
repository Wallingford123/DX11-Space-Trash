#include <Windows.h>
#include <memory>
#include "Base.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	LPWSTR cmdLine, int cmdshow){
	UNREFERENCED_PARAMETER(prevInstance);
	UNREFERENCED_PARAMETER(cmdLine);


	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

	WNDCLASSEX wndClass = { 0 };

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "DX11BookWindowClass";

	if (!RegisterClassEx(&wndClass))
	{
		return -1;
	}

	RECT rc = { 0,0,1280,720 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindowA("DX11BookWindowClass", "Space Trash", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		return -1;
	}

	ShowWindow(hwnd, cmdshow);
	POINT point; 
	point.x = (rc.right - rc.left) / 2;
	point.y = (rc.bottom - rc.top) / 2;
	ShowCursor(false);
	ClientToScreen(hwnd, &point);
	SetCursorPos(point.x, point.y);

	std::auto_ptr<Dx11Base> base(new Base());

	bool result = base->Initialize(hInstance, hwnd);
	if (result == false)
	{
		return -1;
	}

	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		else
		{
			base->Update(0.0f, hwnd);
			base->Render();

		}
	}

	base->Shutdown();


	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT paintStruct;
	HDC hDC;

	switch (message)
	{
	case WM_PAINT:
		hDC = BeginPaint(hwnd, &paintStruct);
		EndPaint(hwnd, &paintStruct);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}