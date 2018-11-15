#pragma once
#include "CModel.h"

class Effect
{
public:
	Effect();
	~Effect();
	
	CModel* getModel() { return &effect; }
	void setVisible(bool setValue) { visible = setValue; }
	bool getVisible() { return visible; }

	void setWorldMatrix(D3DXMATRIX mat) { effect.setworldMatrix(mat); }
	void rotateYawToCamera(D3DXVECTOR3 cameraPos);
	void rotatePitchToCamera(D3DXVECTOR3 cameraPos);
	void rotateY(float angle);
	void loadEffect(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice);
	void setPos(D3DXVECTOR3 pos) { effect.setModelPosition(pos); }
	void drawEffect(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta);
private:
	CModel effect;
	bool visible;
};

