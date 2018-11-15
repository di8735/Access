#pragma once
#include "CModel.h"

class SkyBox
{
public:
	SkyBox();
	~SkyBox();

	void loadModel(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice);
	void drawSkyBox(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta);
private:
	CModel mSkyBoxModel;
	D3DXVECTOR3 mPos;
};

