#pragma once
#include "d3dUtility.h"

class Billboard
{
public:
	Billboard();
	Billboard(float x, float y, float z);
	~Billboard();

	void createQuad(LPDIRECT3DDEVICE9 pd3dDevice, int indexNum, int width, int height);
	void setPos(D3DXVECTOR3 vec);
	void setMatrix(D3DXMATRIX* mat);
	void setWorldMatrix(D3DXMATRIX mat) { worldMatrix = mat; }
	void multiplyWorldMatrix(D3DXMATRIX mat) { worldMatrix = mat * worldMatrix; }
	void setVisible(bool setValue) { visible = setValue; }
	void setAnimTime(float time) { animTime = time; }
	HRESULT loadTexture(wchar_t* strTexName, LPDIRECT3DDEVICE9 pd3dDevice);
	void scaleBillboard(float x, float y, float z);
	void drawBillboard(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta);
	void drawBillboardOnce(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta);
private:
	LPDIRECT3DTEXTURE9 tex;
	IDirect3DVertexBuffer9* quad = 0;
	IDirect3DIndexBuffer9*  IB = 0;
	d3d::Vertex* v;
	D3DXMATRIX worldMatrix;
	float animTime;
	float frameNum;
	bool visible;
};

