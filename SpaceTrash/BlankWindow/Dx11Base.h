#ifndef _DEMO_BASE_H_
#define _DEMO_BASE_H_

#pragma comment(lib, "dinput8")
#pragma comment(lib, "dxguid")

#include<d3d11.h>
#include<d3dx11.h>
#include<DxErr.h>
#include<d3dcompiler.h>
#include <DINPUT.H>

#define KEYDOWN( name, key ) ( name[key] & 0x80 )
#define BUTTONDOWN(name, key) (name.rgbButtons[key] & 0x80)
class Dx11Base
{
public:
	Dx11Base();
	virtual ~Dx11Base();

	char keyboardKeys[256];
	char prevKeyboardKeys[256];
	bool Initialize(HINSTANCE hInstance, HWND hwnd);
	void Shutdown();
	bool CompileD3DShader(char* filePath, char* entry, char* shaderModel, ID3DBlob** buffer);
	bool gameStarted;

	virtual bool LoadContent();
	virtual void UnloadContent();
	virtual void Update(float dt, HWND hwnd) = 0;
	virtual void Render() = 0;
protected:
	HINSTANCE hInstance_;
	HWND hwnd_;

	D3D_DRIVER_TYPE driverType_;
	D3D_FEATURE_LEVEL featureLevel_;

	LPDIRECTINPUT8 inputObject;
	LPDIRECTINPUTDEVICE8 keyboardDevice;
	LPDIRECTINPUTDEVICE8 mouseDevice;
	DIMOUSESTATE mouseState_;
	DIMOUSESTATE prevMouseState_;
	long mouseX;
	long mouseY;

	ID3D11Device* d3dDevice_;
	ID3D11DeviceContext* d3dContext_;
	IDXGISwapChain* swapChain_;
	ID3D11RenderTargetView* backBufferTarget_;

	ID3D11Texture2D* depthTexture_;
	ID3D11DepthStencilView* depthStencilView_;
};

#endif