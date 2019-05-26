#ifndef _BASE_H_
#define _BASE_H_


#include "Dx11Base.h"
#include "objLoader.h"
#include <time.h>
#include <DirectXMath.h>

using namespace DirectX;

#define modelNum 4
#define maxCrates 50
#define maxNum 5

class Base : public Dx11Base
{
	struct Crate{
		XMFLOAT3 position;
		bool isUsed = false;
	};
	struct Player {
		XMFLOAT3 position;
	};
	struct Tube {
		XMFLOAT3 position;
	};
	struct Sprite {
		XMFLOAT3 position;
	};
	struct NumberText {
		XMFLOAT3 position;
		int number;
		bool inUse;
	};

public:
	Base();
	virtual ~Base();

	bool LoadContent();
	void UnloacContent();

	void Update(float dt, HWND hwnd);
	void Render();
private:
	ID3D11VertexShader* solidColorVS_;
	ID3D11PixelShader* solidColorPS_;

	ID3D11InputLayout* inputLayout_;
	ID3D11Buffer* vertexBuffer_[modelNum];
	ID3D11Buffer* numberBuffer_[10];
	int totalVerts_[modelNum];
	XMFLOAT3 modelRad_[modelNum];
	bool LoadModel(char*, char*, int);
	bool LoadNumber(char*, int);
	void updateCratePos();
	void spawnCrate(int);
	void makeCrate(int, XMFLOAT3, float, bool);

	ID3D11ShaderResourceView* colorMap_[modelNum];
	ID3D11ShaderResourceView* numberMap_[10];
	ID3D11SamplerState* colorMapSampler_;
	ID3D11BlendState* alphaBlendState_;

	ID3D11Buffer* viewCB_;
	ID3D11Buffer* projCB_;
	ID3D11Buffer* worldCB_;
	XMMATRIX viewMatrix_;
	XMMATRIX projMatrix_;
	ID3D11Buffer* mvpCB_;

	Player playerObj;
	Tube tubeObj;
	Sprite menuObj;
	Crate crateArray[maxCrates];
	clock_t timer;
	bool startNextSequence;
	time_t nextSpawn;
	int sequencePos;
	int wave;
	int currentSequence;
	int hiSc;
	NumberText numsToDisplay[maxNum];
	int hs;
};

#endif // !_BASE_H_


