#include "SkyBox.h"


SkyBox::SkyBox()
{
	mPos = D3DXVECTOR3(0.0f, 1000.0f, 0.0f);
}


SkyBox::~SkyBox()
{
}

void SkyBox::loadModel(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice)
{
	mSkyBoxModel.LoadXFile(strFileName, pd3dDevice);
}

void SkyBox::drawSkyBox(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta)
{
	mSkyBoxModel.setModelPosition(mPos);
	mSkyBoxModel.Draw(pd3dDevice, timeDelta);
}
