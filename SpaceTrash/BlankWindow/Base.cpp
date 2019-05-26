#include "Base.h"
#include "objLoader.h"
#include <iostream>
#include <string>
#include <fstream>



struct VertexPos
{
	XMFLOAT3 pos;
	XMFLOAT2 tex0;
	XMFLOAT3 norm;
};

Base::Base() : solidColorVS_(0), solidColorPS_(0), inputLayout_(0), colorMapSampler_(0), viewCB_(0), projCB_(0),worldCB_(0), /*indexBuffer_(0),*/ alphaBlendState_(0)
{
	vertexBuffer_[0] = 0;
	vertexBuffer_[1] = 0;
	vertexBuffer_[2] = 0;

	colorMap_[0] = 0;
	colorMap_[1] = 0;
	colorMap_[2] = 0;
}


Base::~Base()
{
}

bool Base::LoadModel(char* modelName, char* texName, int bufNum) {
	srand(time(NULL));
	HRESULT d3dResult;
	ObjModel objModel;
	if (objModel.LoadOBJ(modelName) == false)
	{
		DXTRACE_MSG("Error loading 3D model!");
		return false;
	}

	totalVerts_[bufNum] = objModel.GetTotalVerts();

	VertexPos* vertices = new VertexPos[totalVerts_[bufNum]];
	float* vertsPtr = objModel.GetVertices();
	float* texCPtr = objModel.GetTexCoords();
	float* normalPtr = objModel.GetNormals();
	
	float maxX, maxY, maxZ, minX, minY, minZ;
	maxX = *vertsPtr;
	minX = *vertsPtr;
	maxY = *(vertsPtr+1);
	minY = *(vertsPtr+1);
	maxZ = *(vertsPtr+2);
	minZ = *(vertsPtr+2);
	for (int i = 0; i < totalVerts_[bufNum]; i++)
	{
		vertices[i].pos = XMFLOAT3(*(vertsPtr + 0), *(vertsPtr + 1), *(vertsPtr + 2));
		if (vertices[i].pos.x < minX) minX = vertices[i].pos.x;
		if (vertices[i].pos.x > maxX) maxX = vertices[i].pos.x;
		if (vertices[i].pos.y < minY) minY = vertices[i].pos.y;
		if (vertices[i].pos.y > maxY) maxY = vertices[i].pos.y;
		if (vertices[i].pos.z < minZ) minZ = vertices[i].pos.z;
		if (vertices[i].pos.z > maxZ) maxZ = vertices[i].pos.z;
		vertsPtr += 3;

		vertices[i].tex0 = XMFLOAT2(*(texCPtr + 0), 1-*(texCPtr + 1));
		texCPtr += 2;

		vertices[i].norm = XMFLOAT3(*(normalPtr + 0), *(normalPtr + 1), *(normalPtr + 2));
		normalPtr += 3;
	}

	modelRad_[bufNum].x = (maxX - minX)/3.14;
	modelRad_[bufNum].y = (maxY - minY)/3.14;
	modelRad_[bufNum].z = (maxZ - minZ)/3.14;

	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(VertexPos) * totalVerts_[bufNum];

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = vertices;

	d3dResult = d3dDevice_->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer_[bufNum]);

	if (FAILED(d3dResult))
	{
		DXTRACE_MSG("Failed to create vertex buffer!");
		return false;
	}

	delete[] vertices;
	objModel.Release();


	d3dResult = D3DX11CreateShaderResourceViewFromFile(d3dDevice_, texName, 0, 0, &colorMap_[bufNum], 0);

	if (FAILED(d3dResult))
	{
		DXTRACE_MSG("Failed to load the texture");
		return false;
	}

	D3D11_SAMPLER_DESC colorMapDesc;
	ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
	colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

	d3dResult = d3dDevice_->CreateSamplerState(&colorMapDesc, &colorMapSampler_);
	if (FAILED(d3dResult))
	{
		DXTRACE_MSG("Failed to create map sampler");
		return false;
	}

	ID3D11Resource* colorTex;
	colorMap_[bufNum]->GetResource(&colorTex);

	D3D11_TEXTURE2D_DESC colorTexDesc;
	((ID3D11Texture2D*)colorTex)->GetDesc(&colorTexDesc);
	colorTex->Release();
}

bool Base::LoadNumber(char* texName, int bufNum) {
	srand(time(NULL));
	HRESULT d3dResult;
	ObjModel objModel;
	if (objModel.LoadOBJ((char*)"texPlane.obj") == false)
	{
		DXTRACE_MSG("Error loading 3D model!");
		return false;
	}

	int totalV = 6;

	VertexPos* vertices = new VertexPos[totalV];
	float* vertsPtr = objModel.GetVertices();
	float* texCPtr = objModel.GetTexCoords();
	float* normalPtr = objModel.GetNormals();

	float maxX, maxY, maxZ, minX, minY, minZ;
	maxX = *vertsPtr;
	minX = *vertsPtr;
	maxY = *(vertsPtr + 1);
	minY = *(vertsPtr + 1);
	maxZ = *(vertsPtr + 2);
	minZ = *(vertsPtr + 2);
	for (int i = 0; i < totalV; i++)
	{
		vertices[i].pos = XMFLOAT3(*(vertsPtr + 0), *(vertsPtr + 1), *(vertsPtr + 2));
		if (vertices[i].pos.x < minX) minX = vertices[i].pos.x;
		if (vertices[i].pos.x > maxX) maxX = vertices[i].pos.x;
		if (vertices[i].pos.y < minY) minY = vertices[i].pos.y;
		if (vertices[i].pos.y > maxY) maxY = vertices[i].pos.y;
		if (vertices[i].pos.z < minZ) minZ = vertices[i].pos.z;
		if (vertices[i].pos.z > maxZ) maxZ = vertices[i].pos.z;
		vertsPtr += 3;

		vertices[i].tex0 = XMFLOAT2(*(texCPtr + 0), 1 - *(texCPtr + 1));
		texCPtr += 2;

		vertices[i].norm = XMFLOAT3(*(normalPtr + 0), *(normalPtr + 1), *(normalPtr + 2));
		normalPtr += 3;
	}

	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(VertexPos) * totalV;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = vertices;

	d3dResult = d3dDevice_->CreateBuffer(&vertexDesc, &resourceData, &numberBuffer_[bufNum]);

	if (FAILED(d3dResult))
	{
		DXTRACE_MSG("Failed to create vertex buffer!");
		return false;
	}

	delete[] vertices;
	objModel.Release();


	d3dResult = D3DX11CreateShaderResourceViewFromFile(d3dDevice_, texName, 0, 0, &numberMap_[bufNum], 0);

	if (FAILED(d3dResult))
	{
		DXTRACE_MSG("Failed to load the texture");
		return false;
	}

	D3D11_SAMPLER_DESC colorMapDesc;
	ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
	colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

	d3dResult = d3dDevice_->CreateSamplerState(&colorMapDesc, &colorMapSampler_);
	if (FAILED(d3dResult))
	{
		DXTRACE_MSG("Failed to create map sampler");
		return false;
	}

	ID3D11Resource* colorTex;
	numberMap_[bufNum]->GetResource(&colorTex);

	D3D11_TEXTURE2D_DESC colorTexDesc;
	((ID3D11Texture2D*)colorTex)->GetDesc(&colorTexDesc);
	colorTex->Release();
}


bool Base::LoadContent()
{
	ID3DBlob* vsBuffer = 0;

	bool complieResult = CompileD3DShader((char*)"TextureMap.fx", (char*)"VS_Main", (char*)"vs_4_0", &vsBuffer);

	if (complieResult == false)
	{
		MessageBox(0, "Error loading vertex shader", "Compile Error", MB_OK);
		return false;
	}

	HRESULT d3dResult;

	d3dResult = d3dDevice_->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), 0, &solidColorVS_);

	if (FAILED(d3dResult))
	{
		if (vsBuffer)
		{
			vsBuffer->Release();
		}
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	unsigned int totalLayoutElements = ARRAYSIZE(solidColorLayout);

	d3dResult = d3dDevice_->CreateInputLayout(solidColorLayout, totalLayoutElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &inputLayout_);

	vsBuffer->Release();

	if (FAILED(d3dResult))
	{
		return false;
	}
	ID3DBlob* psBuffer = 0;

	complieResult = CompileD3DShader((char*)"TextureMap.fx", (char*)"PS_Main", (char*)"ps_4_0", &psBuffer);

	if (complieResult == false)
	{
		MessageBox(0, "Error loading pixel shadder", "Compile Error", MB_OK);
		return false;
	}

	d3dResult = d3dDevice_->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, &solidColorPS_);

	psBuffer->Release();

	if (FAILED(d3dResult))
	{
		return false;
	}


//////////////////--------MODELS----------//////////////////////////////////
	
	LoadModel((char*)"ship.obj", (char*) "shipUV.dds", 0);
	LoadModel((char*)"cube.obj", (char*) "decal1.dds", 1);
	LoadModel((char*)"tube.obj", (char*) "decal2.dds", 2);
	LoadModel((char*)"plane.obj", (char*) "menuTex.dds", 3);

	LoadNumber((char*)"0.dds", 0);
	LoadNumber((char*)"1.dds", 1);
	LoadNumber((char*)"2.dds", 2);
	LoadNumber((char*)"3.dds", 3);
	LoadNumber((char*)"4.dds", 4);
	LoadNumber((char*)"5.dds", 5);
	LoadNumber((char*)"6.dds", 6);
	LoadNumber((char*)"7.dds", 7);
	LoadNumber((char*)"8.dds", 8);
	LoadNumber((char*)"9.dds", 9);


//////////////////--------MODELS----------//////////////////////////////////

	D3D11_BUFFER_DESC constDesc;
	ZeroMemory(&constDesc, sizeof(constDesc));
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.ByteWidth = sizeof(XMMATRIX);
	constDesc.Usage = D3D11_USAGE_DEFAULT;

	d3dResult = d3dDevice_->CreateBuffer(&constDesc, 0, &viewCB_);
	if (FAILED(d3dResult))
	{
		return false;
	}

	d3dResult = d3dDevice_->CreateBuffer(&constDesc, 0, &projCB_);
	if (FAILED(d3dResult))
	{
		return false;
	}

	d3dResult = d3dDevice_->CreateBuffer(&constDesc, 0, &worldCB_);
	if (FAILED(d3dResult))
	{
		return false;
	}

	viewMatrix_ = XMMatrixIdentity();
	projMatrix_ = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1280.0f / 720.0f, 0.01f, 1000.0f);

	viewMatrix_ = XMMatrixTranspose(viewMatrix_);
	projMatrix_ = XMMatrixTranspose(projMatrix_);


/*	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	d3dDevice_->CreateBlendState(&blendDesc, &alphaBlendState_);
	d3dContext_->OMSetBlendState(alphaBlendState_, blendFactor, 0xFFFFFFFF);*/

	HRESULT hre = DirectInput8Create(hInstance_, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&inputObject, 0);
	if FAILED(hre) {
		return false;
	}
	hre = inputObject->CreateDevice(GUID_SysKeyboard, &keyboardDevice, 0);
	if FAILED(hre) {
		false;
	}
	hre = keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	if FAILED(hre) {
		false;
	}
	hre = keyboardDevice->SetCooperativeLevel(hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if FAILED(hre) {
		false;
	}
	hre = keyboardDevice->Acquire();
	if FAILED(hre) {
		false;
	}

	hre = inputObject->CreateDevice(GUID_SysMouse, &mouseDevice, 0);
	if FAILED(hre) {
		false;
	}
	hre = mouseDevice->SetDataFormat(&c_dfDIMouse);
	if FAILED(hre) {
		false;
	}
	hre = mouseDevice->SetCooperativeLevel(hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if FAILED(hre) {
		false;
	}
	hre = mouseDevice->Acquire();
	if FAILED(hre) {
		false;
	}

//////////////////--------MODELS POSITION----------/////////////////////

	playerObj.position = {
		0,0,15
	};

	tubeObj.position = {
		8,0,45
	};
	menuObj.position = {
		0,-2,30
	};
	timer = clock();
	startNextSequence = true;
	nextSpawn = timer;
	sequencePos = 0;
	wave = 1;
	gameStarted = false;
	numsToDisplay[0].inUse = true;
	numsToDisplay[0].number = 0;;
	numsToDisplay[0].position = {
		-2,-3,20
	};
	numsToDisplay[1].position = {
		-1.125f,-3,20
	};
	numsToDisplay[2].position = {
		-0.25f,-3,20
	};
	numsToDisplay[3].position = {
		0.625f,-3,20
	};
	numsToDisplay[4].position = {
		1.5f,-3,20
	};
	std::string line;
	std::ofstream hsOut("notahighscore.txt", std::ios_base::app);
	std::ifstream hsIn ("notahighscore.txt");
	if (hsIn.is_open())
	{
		std::getline(hsIn, line);
		std::cout << line << std::endl;
		if (line.empty()) {
			if (hsOut.is_open()) {
				hsOut << "0" << std::endl;
				hiSc = 0;
				hs = 0;
			}
		}
		else {
			hs = std::stoi(line);
			hiSc = hs;
		}
	}
	else std::cout << "Unable to open file";
//	if (line.empty()) {
//		hsOut << "1";
//	}



//////////////////--------MODELS POSITION----------//////////////////////

	return true;
}

void Base::UnloacContent()
{

	if (colorMapSampler_)colorMapSampler_->Release();
	if (solidColorVS_)solidColorVS_->Release();
	if (solidColorPS_)solidColorPS_->Release();
	if (inputLayout_)inputLayout_->Release();
	if (vertexBuffer_[0])vertexBuffer_[0]->Release();
	if (vertexBuffer_[1])vertexBuffer_[1]->Release();
	if (vertexBuffer_[2])vertexBuffer_[2]->Release();
	if (colorMap_[0])colorMap_[0]->Release();
	if (colorMap_[1])colorMap_[0]->Release();
	if (colorMap_[2])colorMap_[0]->Release();
	if (alphaBlendState_)alphaBlendState_->Release();
	if (keyboardDevice)keyboardDevice->Unacquire();
	if (keyboardDevice)keyboardDevice->Release();
	if (inputObject)inputObject->Release();
//	if (indexBuffer_)indexBuffer_->Release();
	if (viewCB_)viewCB_->Release();
	if (projCB_)projCB_->Release();
	if (worldCB_)worldCB_->Release();
	

	colorMapSampler_ = 0;
	solidColorVS_ = 0;
	solidColorPS_ = 0;
	inputLayout_ = 0;
	vertexBuffer_[0] = 0;
	vertexBuffer_[1] = 0;
	vertexBuffer_[2] = 0;
//	indexBuffer_ = 0;
	alphaBlendState_ = 0;
	viewCB_ = 0;
	projCB_ = 0;
	worldCB_ = 0;
}

void Base::makeCrate(int spawnID, XMFLOAT3 pos, float difMod, bool isLast) {
	if (timer > nextSpawn) {
		crateArray[spawnID].isUsed = true;
		crateArray[spawnID].position = {
			pos.x,pos.y,pos.z
		};

		nextSpawn = timer + (clock_t)difMod;
		sequencePos++;
		if (isLast) {
			startNextSequence = true;
			sequencePos = 0;
		}
	}
}

void Base::spawnCrate(int sequence) {
	timer = clock();
	float difMod = 1000.0f;
	int spawnID;
	for (int i = 0; i < wave; i++) difMod*=0.935;
	bool go = true;
	for (int i = 0; go == true; i++) {
		if (!crateArray[i].isUsed) {
			spawnID = i;
			go = false;
		}
	}
	switch (sequence) {
	case 0:
		switch (sequencePos) {
		case 0:
			makeCrate(spawnID, XMFLOAT3(-8, 0, 50), difMod, false);
			break;
		case 1:
			makeCrate(spawnID, XMFLOAT3(8, 0, 50), difMod, false);
			break;
		case 2:
			makeCrate(spawnID, XMFLOAT3(0, 5, 50), difMod, false);
			break;
		case 3:
			makeCrate(spawnID, XMFLOAT3(0, -5, 50), difMod, false);
			break;
		case 4:
			makeCrate(spawnID, XMFLOAT3(0, 0, 50), difMod, true);
			break;
		}
		break;
	case 1:
		switch (sequencePos) {
		case 0:
			makeCrate(spawnID, XMFLOAT3(0, 5, 50), difMod, false);
			break;
		case 1:
			makeCrate(spawnID, XMFLOAT3(-4, 4, 50), difMod, false);
			break;
		case 2:
			makeCrate(spawnID, XMFLOAT3(-6.5f, 2.5f, 50), difMod, false);
			break;
		case 3:
			makeCrate(spawnID, XMFLOAT3(-8, 0, 50), difMod, false);
			break;
		case 4:
			makeCrate(spawnID, XMFLOAT3(-6.5, -2.5f, 50), difMod, false);
			break;
		case 5:
			makeCrate(spawnID, XMFLOAT3(-4, -4, 50), difMod, false);
			break;
		case 6:
			makeCrate(spawnID, XMFLOAT3(0, -5, 50), difMod, false);
			break;
		case 7:
			makeCrate(spawnID, XMFLOAT3(4, -4, 50), difMod, false);
			break;
		case 8:
			makeCrate(spawnID, XMFLOAT3(6.5, -2.5f, 50), difMod, false);
			break;
		case 9:
			makeCrate(spawnID, XMFLOAT3(8, 0, 50), difMod, false);
			break;
		case 10:
			makeCrate(spawnID, XMFLOAT3(6.5, 2.5f, 50), difMod, false);
			break;
		case 11:
			makeCrate(spawnID, XMFLOAT3(4, 4, 50), difMod, false);
			break;
		case 12:
			makeCrate(spawnID, XMFLOAT3(0, 5, 50), difMod, true);
			break;
		}

		break;
	case 2:
		switch (sequencePos) {
		case 0:
			makeCrate(spawnID, XMFLOAT3(-8, 5, 50), difMod, false);
			break;
		case 1:
			makeCrate(spawnID, XMFLOAT3(0, 0, 50), difMod, false);
			break;
		case 2:
			makeCrate(spawnID, XMFLOAT3(8, -5, 50), difMod, false);
			break;
		case 3:
			makeCrate(spawnID, XMFLOAT3(8, 5, 50), difMod, false);
			break;
		case 4:
			makeCrate(spawnID, XMFLOAT3(0, 0, 50), difMod, false);
			break;
		case 5:
			makeCrate(spawnID, XMFLOAT3(-8, -5, 50), difMod, true);
			break;
		}
		break;
	case 3:
		switch (sequencePos) {
		case 0:
			makeCrate(spawnID, XMFLOAT3(0, 5, 50), difMod, false);
			break;
		case 1:
			makeCrate(spawnID, XMFLOAT3(0, 0, 50), difMod, false);
			break;
		case 2:
			makeCrate(spawnID, XMFLOAT3(0, -5, 50), difMod, false);
			break;
		case 3:
			makeCrate(spawnID, XMFLOAT3(-8, 0, 50), difMod, false);
			break;
		case 4:
			makeCrate(spawnID, XMFLOAT3(0, 0, 50), difMod, false);
			break;
		case 5:
			makeCrate(spawnID, XMFLOAT3(8, 0, 50), difMod, true);
			break;
		}
		break;
	case 4:
		switch (sequencePos) {
		case 0:
			makeCrate(spawnID, XMFLOAT3(-8, 5, 50), difMod, false);
			break;
		case 1:
			makeCrate(spawnID, XMFLOAT3(-8, 3, 50), difMod, false);
			break;
		case 2:
			makeCrate(spawnID, XMFLOAT3(-8, 1, 50), difMod, false);
			break;
		case 3:
			makeCrate(spawnID, XMFLOAT3(-8, -1, 50), difMod, false);
			break;
		case 4:
			makeCrate(spawnID, XMFLOAT3(-8, -3, 50), difMod, false);
			break;
		case 5:
			makeCrate(spawnID, XMFLOAT3(-8, -5, 50), difMod, true);
			break;
		}
		break;
	case 5:
		switch (sequencePos) {
		case 0:
			makeCrate(spawnID, XMFLOAT3(8, 5, 50), difMod, false);
			break;
		case 1:
			makeCrate(spawnID, XMFLOAT3(8, 3, 50), difMod, false);
			break;
		case 2:
			makeCrate(spawnID, XMFLOAT3(8, 1, 50), difMod, false);
			break;
		case 3:
			makeCrate(spawnID, XMFLOAT3(8, -1, 50), difMod, false);
			break;
		case 4:
			makeCrate(spawnID, XMFLOAT3(8, -3, 50), difMod, false);
			break;
		case 5:
			makeCrate(spawnID, XMFLOAT3(8, -5, 50), difMod, true);
			break;
		}
		break;
	case 6:
		switch (sequencePos) {
		case 0:
			makeCrate(spawnID, XMFLOAT3(8, 5, 50), difMod, false);
			break;
		case 1:
			makeCrate(spawnID, XMFLOAT3(8, 3, 50), difMod, false);
			break;
		case 2:
			makeCrate(spawnID, XMFLOAT3(8, 1, 50), difMod, false);
			break;
		case 3:
			makeCrate(spawnID, XMFLOAT3(8, -1, 50), difMod, false);
			break;
		case 4:
			makeCrate(spawnID, XMFLOAT3(8, -3, 50), difMod, false);
			break;
		case 5:
			makeCrate(spawnID, XMFLOAT3(8, -5, 50), difMod, true);
			break;
		}
		break;
	case 7:
		switch (sequencePos) {
		case 0:
			makeCrate(spawnID, XMFLOAT3(8, 5, 50), difMod, false);
			break;
		case 1:
			makeCrate(spawnID, XMFLOAT3(6, 5, 50), difMod, false);
			break;
		case 2:
			makeCrate(spawnID, XMFLOAT3(4, 5, 50), difMod, false);
			break;
		case 3:
			makeCrate(spawnID, XMFLOAT3(2, 5, 50), difMod, false);
			break;
		case 4:
			makeCrate(spawnID, XMFLOAT3(0, 5, 50), difMod, false);
			break;
		case 5:
			makeCrate(spawnID, XMFLOAT3(2, 5, 50), difMod, false);
			break;
		case 6:
			makeCrate(spawnID, XMFLOAT3(4, 5, 50), difMod, false);
			break;
		case 7:
			makeCrate(spawnID, XMFLOAT3(6, 5, 50), difMod, false);
			break;
		case 8:
			makeCrate(spawnID, XMFLOAT3(8, 5, 50), difMod, true);
			break;

		}
		break;
	}
}

void Base::updateCratePos() {
	float difMod = 0.1f;
	for (int i = 0; i < wave; i++) difMod *= 1.05f;
	for (int i = 0; i < maxCrates; i++) {
		if (crateArray[i].isUsed) {
			crateArray[i].position.z -= difMod;
			if (crateArray[i].position.z < 10) {
				gameStarted = false;
				for (int z = 0; z < maxCrates; z++) {
					crateArray[z].isUsed = false;
				}
				numsToDisplay[0].position = {
					-2,-3,20
				};
				numsToDisplay[1].position = {
					-1.125f,-3,20
				};
				numsToDisplay[2].position = {
					-0.25f,-3,20
				};
				numsToDisplay[3].position = {
					0.625f,-3,20
				};
				numsToDisplay[4].position = {
					1.5f,-3,20
				};
				if (hiSc > hs) {
					std::ofstream hsOut("notahighscore.txt");
					hs = hiSc;
					if (hsOut.is_open()) {
						hsOut << std::to_string(hs);
					}
				}
				hiSc = hs;
			}
		}
	}
}

void Base::Update(float dt, HWND hwnd)
{
	std::string hi = std::to_string(hiSc);
	char c1 = hi.at(0);
	std::string s1(1, c1);
	numsToDisplay[0].number = std::stoi(s1);
	if (hiSc >= 10) {
		char c2 = hi.at(1);
		std::string s2(1, c2);
		numsToDisplay[1].number = std::stoi(s2);
		numsToDisplay[1].inUse = true;
	}
	else numsToDisplay[1].inUse = false;
	if (hiSc >= 100) {
		char c3 = hi.at(2);
		std::string s3(1, c3);
		numsToDisplay[2].number = std::stoi(s3);
		numsToDisplay[2].inUse = true;
	}
	else numsToDisplay[2].inUse = false;
	if (hiSc >= 1000) {
		char c4 = hi.at(3);
		std::string s4(1, c4);
		numsToDisplay[3].number = std::stoi(s4);
		numsToDisplay[3].inUse = true;
	}
	else numsToDisplay[3].inUse = false;
	if (hiSc >= 10000) {
		char c5 = hi.at(4);
		std::string s5(1, c5);
		numsToDisplay[4].number = std::stoi(s5);
		numsToDisplay[4].inUse = true;
	}
	else numsToDisplay[4].inUse = false;
	
	
	HRESULT hr = keyboardDevice->GetDeviceState(sizeof(keyboardKeys), (LPVOID)&keyboardKeys);
	if FAILED(hr) {
		hr = keyboardDevice->Acquire();
		while (hr == DIERR_INPUTLOST) {
			hr = keyboardDevice->Acquire();
		}
	}
	hr = mouseDevice->GetDeviceState(sizeof(mouseState_), (LPVOID)&mouseState_);
	if FAILED(hr) {
		hr = mouseDevice->Acquire();
		while (hr == DIERR_INPUTLOST) {
			hr = mouseDevice->Acquire();
		}
	}
	else {
		RECT rc;
		GetWindowRect(hwnd, &rc);
		POINT point;
		point.x = (rc.right - rc.left) / 2;
		point.y = (rc.bottom - rc.top) / 2;
		ClientToScreen(hwnd, &point);
		SetCursorPos(point.x, point.y);
	}
//	CODE FOR OnKeyUp
//	if (KEYDOWN(prevKeyboardKeys, DIK_LEFT) && !KEYDOWN(keyboardKeys,DIK_LEFT)) {

//	CODE FOR OnKeyDown
//	if (KEYDOWN(keyboardKeys, DIK_LEFT) && !KEYDOWN(prevKeyboardKeys,DIK_LEFT)) {

//	CODE FOR OnKey
//	if (KEYDOWN(keyboardKeys, DIK_LEFT)) {


/*	if(KEYDOWN(keyboardKeys,DIK_LEFT)){
		playerObj.position.x -= 0.25f;
	}
	if (KEYDOWN(keyboardKeys, DIK_RIGHT)) {
		playerObj.position.x += 0.25f;
	}
	if (KEYDOWN(keyboardKeys, DIK_UP)) {
		playerObj.position.y += 0.25f;
	}*/
	if (KEYDOWN(keyboardKeys, DIK_ESCAPE) && !KEYDOWN(prevKeyboardKeys, DIK_ESCAPE)) {
		if (gameStarted) {
			gameStarted = false;
			for (int z = 0; z < maxCrates; z++) {
				crateArray[z].isUsed = false;
			}
			numsToDisplay[0].position = {
				-2,-3,20
			};
			numsToDisplay[1].position = {
				-1.125f,-3,20
			};
			numsToDisplay[2].position = {
				-0.25f,-3,20
			};
			numsToDisplay[3].position = {
				0.625f,-3,20
			};
			numsToDisplay[4].position = {
				1.5f,-3,20
			};
			startNextSequence = true;
			wave = 0;
			if (hiSc > hs) {
				std::ofstream hsOut("notahighscore.txt");
				hs = hiSc;
				if (hsOut.is_open()) {
					hsOut << std::to_string(hs);
				}
			}
			hiSc = hs;
		}
		else PostQuitMessage(0);
	}
	if (KEYDOWN(keyboardKeys, DIK_SPACE) && !KEYDOWN(prevKeyboardKeys, DIK_SPACE) && !gameStarted) {
		gameStarted = true;
		numsToDisplay[0].position = {
			-9.875f,5.355f,14
		};
		numsToDisplay[1].position = {
			-9,5.355f,14
		};
		numsToDisplay[2].position = {
			-8.125,5.355f,14
		};
		numsToDisplay[3].position = {
			-7.25,5.355f,14
		};
		numsToDisplay[4].position = {
			-6.375,5.355f,14
		};
		hiSc = 0;

	}
	

	if (gameStarted) {
		for (int i = 0; i < maxCrates; i++) {
			if (crateArray[i].isUsed) {
				if ((playerObj.position.x + modelRad_[0].x) >= (crateArray[i].position.x - modelRad_[1].x) && (playerObj.position.x - modelRad_[0].x) <= (crateArray[i].position.x + modelRad_[1].x)) {
					if ((playerObj.position.y + modelRad_[0].y) >= (crateArray[i].position.y - modelRad_[1].y) && (playerObj.position.y - modelRad_[0].y) <= (crateArray[i].position.y + modelRad_[1].y)) {
						if ((playerObj.position.z + modelRad_[0].z) >= (crateArray[i].position.z - modelRad_[1].z) && (playerObj.position.z - modelRad_[0].z) <= (crateArray[i].position.z + modelRad_[1].z)) {
							PlaySound("sound.wav", NULL, SND_FILENAME | SND_ASYNC);
							crateArray[i].position.x += 10000;
							crateArray[i].isUsed = false;
							hiSc++;
						}
					}
				}
			}
		}

		/*if ((playerBoundMax.x - playerWidth) >= modelMax.x && playerBoundMax.x <= (modelMax.x - modelWidth) &&
		(playerBoundMax.y - playerHeight) >= modelMax.y && playerBoundMax.y <= (modelMax.y - modelHeight) &&
			(playerBoundMax.z - playerDepth) >= modelMax.z && playerBoundMax.z <= (modelMax.z - modelDepth)) {*/



		if (startNextSequence) {
			currentSequence = rand() % 8 + 0;
			startNextSequence = false;
			wave++;
		}
		if (wave > 25)wave = 25;
		spawnCrate(currentSequence);
		updateCratePos();
	}
	playerObj.position.x += mouseState_.lX / 64.0f;
	playerObj.position.y -= mouseState_.lY / 64.0f;

	mouseX += mouseState_.lX;
	mouseY += mouseState_.lY;

	if (BUTTONDOWN(mouseState_, 0) && !BUTTONDOWN(prevMouseState_, 0)) {
		if (playerObj.position.x > -6 && playerObj.position.x < 5.75f && playerObj.position.y > 0.75f && playerObj.position.y < 4) {
			gameStarted = true;
			numsToDisplay[0].position = {
				-9.875f,5.355f,14
			};
			numsToDisplay[1].position = {
				-9,5.355f,14
			};
			numsToDisplay[2].position = {
				-8.125,5.355f,14
			};
			numsToDisplay[3].position = {
				-7.25,5.355f,14
			};
			numsToDisplay[4].position = {
				-6.375,5.355f,14
			};
			hiSc = 0;
		}
	}

	if (playerObj.position.x < -10) playerObj.position.x = -10;
	if (playerObj.position.x > 10) playerObj.position.x = 10;
	if (playerObj.position.y < -5.3f) playerObj.position.y = -5.3f;
	if (playerObj.position.y > 5.3f) playerObj.position.y = 5.3f;
	memcpy(&prevMouseState_, &mouseState_, sizeof(mouseState_));
	memcpy(prevKeyboardKeys, keyboardKeys, sizeof(keyboardKeys));
}

void Base::Render()
{
	if (d3dContext_ == 0)
	{
		return;
	}

	float clearColor[4] = { 0.0f, 0.0f, 0.25f, 1.0f };
	d3dContext_->ClearRenderTargetView(backBufferTarget_, clearColor);
	d3dContext_->ClearDepthStencilView(depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

	unsigned int stride = sizeof(VertexPos);
	unsigned int offset = 0;

	d3dContext_->IASetInputLayout(inputLayout_);
	d3dContext_->IASetVertexBuffers(0, 1, &vertexBuffer_[0], &stride, &offset);
//	d3dContext_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R16_UINT, 0);
	d3dContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d3dContext_->VSSetShader(solidColorVS_, 0, 0);
	d3dContext_->PSSetShader(solidColorPS_, 0, 0);
	d3dContext_->PSSetShaderResources(0, 1, &colorMap_[0]);
	d3dContext_->PSSetSamplers(0, 1, &colorMapSampler_);

	XMMATRIX scaleMat = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX rotationMat = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	XMMATRIX translationMat = XMMatrixTranslation(playerObj.position.x, playerObj.position.y, playerObj.position.z);
	XMMATRIX worldMat = scaleMat * rotationMat * translationMat;
	worldMat = XMMatrixTranspose(worldMat);

	d3dContext_->UpdateSubresource(worldCB_, 0, 0, &worldMat, 0, 0);
	d3dContext_->UpdateSubresource(viewCB_, 0, 0, &viewMatrix_, 0, 0);
	d3dContext_->UpdateSubresource(projCB_, 0, 0, &projMatrix_, 0, 0);

	d3dContext_->VSSetConstantBuffers(0, 1, &worldCB_);
	d3dContext_->VSSetConstantBuffers(1, 1, &viewCB_);
	d3dContext_->VSSetConstantBuffers(2, 1, &projCB_);

	d3dContext_->Draw(totalVerts_[0], 0);
	if (gameStarted) {
		d3dContext_->PSSetShaderResources(0, 1, &colorMap_[1]);
		d3dContext_->IASetVertexBuffers(0, 1, &vertexBuffer_[1], &stride, &offset);
		for (int i = 0; i < maxCrates; i++) {
			if (crateArray[i].isUsed) {
				scaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
				rotationMat = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
				translationMat = XMMatrixTranslation(crateArray[i].position.x, crateArray[i].position.y, crateArray[i].position.z);
				worldMat = scaleMat * rotationMat * translationMat;
				worldMat = XMMatrixTranspose(worldMat);

				d3dContext_->UpdateSubresource(worldCB_, 0, 0, &worldMat, 0, 0);
				d3dContext_->UpdateSubresource(viewCB_, 0, 0, &viewMatrix_, 0, 0);
				d3dContext_->UpdateSubresource(projCB_, 0, 0, &projMatrix_, 0, 0);

				d3dContext_->VSSetConstantBuffers(0, 1, &worldCB_);
				d3dContext_->VSSetConstantBuffers(1, 1, &viewCB_);
				d3dContext_->VSSetConstantBuffers(2, 1, &projCB_);

				d3dContext_->Draw(totalVerts_[1], 0);
			}
		}

		d3dContext_->PSSetShaderResources(0, 1, &colorMap_[2]);
		d3dContext_->IASetVertexBuffers(0, 1, &vertexBuffer_[2], &stride, &offset);
		scaleMat = XMMatrixScaling(50.0f, 50.0f, 50.0f);
		rotationMat = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		translationMat = XMMatrixTranslation(tubeObj.position.x, tubeObj.position.y, tubeObj.position.z);
		worldMat = scaleMat * rotationMat * translationMat;
		worldMat = XMMatrixTranspose(worldMat);

		d3dContext_->UpdateSubresource(worldCB_, 0, 0, &worldMat, 0, 0);
		d3dContext_->UpdateSubresource(viewCB_, 0, 0, &viewMatrix_, 0, 0);
		d3dContext_->UpdateSubresource(projCB_, 0, 0, &projMatrix_, 0, 0);

		d3dContext_->VSSetConstantBuffers(0, 1, &worldCB_);
		d3dContext_->VSSetConstantBuffers(1, 1, &viewCB_);
		d3dContext_->VSSetConstantBuffers(2, 1, &projCB_);

		d3dContext_->Draw(totalVerts_[2], 0);

		/*	for (int i = 0; i < 3; i++)
			{
				XMMATRIX world = sprites_[i].GetWorldMatrix();
				XMMATRIX mvp = world * vpMatrix_;
				//XMMATRIX mvp = DirectX::XMMatrixMultiply(world, vpMatrix_);
				mvp = DirectX::XMMatrixTranspose(mvp);

				d3dContext_->UpdateSubresource(mvpCB_, 0, 0, &mvp, 0, 0);
				d3dContext_->VSSetConstantBuffers(0, 1, &mvpCB_);

				d3dContext_->Draw(6, 0);
			}*/
	}
	else {
		d3dContext_->PSSetShaderResources(0, 1, &colorMap_[3]);
		d3dContext_->IASetVertexBuffers(0, 1, &vertexBuffer_[3], &stride, &offset);
		scaleMat = XMMatrixScaling(10.0f, 10.0f, 10.0f);
		rotationMat = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		translationMat = XMMatrixTranslation(menuObj.position.x, menuObj.position.y, menuObj.position.z);
		worldMat = scaleMat * rotationMat * translationMat;
		worldMat = XMMatrixTranspose(worldMat);

		d3dContext_->UpdateSubresource(worldCB_, 0, 0, &worldMat, 0, 0);
		d3dContext_->UpdateSubresource(viewCB_, 0, 0, &viewMatrix_, 0, 0);
		d3dContext_->UpdateSubresource(projCB_, 0, 0, &projMatrix_, 0, 0);

		d3dContext_->VSSetConstantBuffers(0, 1, &worldCB_);
		d3dContext_->VSSetConstantBuffers(1, 1, &viewCB_);
		d3dContext_->VSSetConstantBuffers(2, 1, &projCB_);

		d3dContext_->Draw(totalVerts_[3], 0);
	}

	for (int i = 0; i < maxNum; i++) {
		if (numsToDisplay[i].inUse) {
			int num = numsToDisplay[i].number;
			XMFLOAT3 pos = numsToDisplay[i].position;
			d3dContext_->PSSetShaderResources(0, 1, &numberMap_[num]);
			d3dContext_->IASetVertexBuffers(0, 1, &numberBuffer_[num], &stride, &offset);
			scaleMat = XMMatrixScaling(0.175f, 0.175f, 0.175f);
			rotationMat = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
			translationMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
			worldMat = scaleMat * rotationMat * translationMat;
			worldMat = XMMatrixTranspose(worldMat);

			d3dContext_->UpdateSubresource(worldCB_, 0, 0, &worldMat, 0, 0);
			d3dContext_->UpdateSubresource(viewCB_, 0, 0, &viewMatrix_, 0, 0);
			d3dContext_->UpdateSubresource(projCB_, 0, 0, &projMatrix_, 0, 0);

			d3dContext_->VSSetConstantBuffers(0, 1, &worldCB_);
			d3dContext_->VSSetConstantBuffers(1, 1, &viewCB_);
			d3dContext_->VSSetConstantBuffers(2, 1, &projCB_);

			d3dContext_->Draw(6, 0);
		}
	}

	swapChain_->Present(1, 0);
}

