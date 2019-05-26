#include "Dx11Base.h"

Dx11Base::Dx11Base() : driverType_(D3D_DRIVER_TYPE_NULL), featureLevel_(D3D_FEATURE_LEVEL_11_0), d3dDevice_(0), d3dContext_(0), swapChain_(0), backBufferTarget_(0), depthTexture_(0), depthStencilView_(0)
{

}

Dx11Base::~Dx11Base()
{
	Shutdown();
}

bool Dx11Base::LoadContent()
{
	//used for override 
	return true;
}

void Dx11Base::UnloadContent()
{
	//usee for override
}


void Dx11Base::Shutdown()
{
	UnloadContent();

	if (backBufferTarget_) backBufferTarget_->Release();
	if (swapChain_) swapChain_->Release();
	if (d3dContext_) d3dContext_->Release();
	if (d3dDevice_) d3dDevice_->Release();
	if (depthTexture_) depthTexture_->Release();
	if (depthStencilView_) depthStencilView_->Release();

	d3dDevice_ = 0;
	d3dContext_ = 0;
	swapChain_ = 0;
	backBufferTarget_ = 0;
	depthStencilView_ = 0;
	depthTexture_ = 0;
}

bool Dx11Base::CompileD3DShader(char * filePath, char * entry, char * shaderModel, ID3DBlob ** buffer)
{
	DWORD shadderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined(_DEBUG)
	shadderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* errorBuffer = 0;
	HRESULT result;

	result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel, shadderFlags, 0, 0, buffer, &errorBuffer, 0);

	if (FAILED(result))
	{
		if (errorBuffer != 0)
		{
			OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
			errorBuffer->Release();
		}
		return false;
	}

	if (errorBuffer != 0)
	{
		errorBuffer->Release();
	}
	return true;
}

bool Dx11Base::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	hInstance_ = hInstance;
	hwnd_ = hwnd;
	RECT dimensions;
	GetClientRect(hwnd, &dimensions);

	gameStarted = false;

	unsigned int width = dimensions.right - dimensions.left;
	unsigned int height = dimensions.bottom - dimensions.top;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_SOFTWARE
	};

	unsigned int totalDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	unsigned int totalFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	unsigned int creationFlags = 0;

#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result;
	unsigned int driver = 0;

	for (driver = 0; driver < totalDriverTypes; ++driver)
	{
		result = D3D11CreateDeviceAndSwapChain(0, driverTypes[driver], 0, creationFlags, featureLevels, totalFeatureLevels, D3D11_SDK_VERSION, &swapChainDesc, &swapChain_, &d3dDevice_, &featureLevel_, &d3dContext_);

		if (SUCCEEDED(result))
		{
			driverType_ = driverTypes[driver];
			break;
		}
	}

	if (FAILED(result))
	{
		DXTRACE_MSG("Failed to create D3 Device");
		return false;
	}

	ID3D11Texture2D* backBufferTexture;

	result = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);

	if (FAILED(result))
	{
		DXTRACE_MSG("Failed to get swap chain back buffer");
		return false;
	}

	result = d3dDevice_->CreateRenderTargetView(backBufferTexture, 0, &backBufferTarget_);

	if (backBufferTexture)
	{
		backBufferTexture->Release();
	}

	if (FAILED(result))
	{
		DXTRACE_MSG("Failed to create the render target view");
		return false;
	}



	D3D11_TEXTURE2D_DESC depthTexDesc;
	ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
	depthTexDesc.Width = width;
	depthTexDesc.Height = height;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	result = d3dDevice_->CreateTexture2D(&depthTexDesc, NULL, &depthTexture_);

	if (FAILED(result)) {
		DXTRACE_MSG("Failed to create depth texture");
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = depthTexDesc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	
	result = d3dDevice_->CreateDepthStencilView(depthTexture_, &descDSV, &depthStencilView_);
	
	if (FAILED(result)) {
		DXTRACE_MSG("Failed to create depth stencil target view");
		return false;
	}

	d3dContext_->OMSetRenderTargets(1, &backBufferTarget_, depthStencilView_);

	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	d3dContext_->RSSetViewports(1, &viewport);

	return LoadContent();


}