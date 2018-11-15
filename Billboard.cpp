#include "Billboard.h"


Billboard::Billboard()
{
	D3DXMatrixIdentity(&worldMatrix);
	animTime = 0.0f;
	frameNum = 0.0f;
	visible = false;
}

Billboard::Billboard(float x, float y, float z)
{
	D3DXMatrixIdentity(&worldMatrix);
	animTime = 0.0f;
	frameNum = 0.0f;
	visible = false;
}

Billboard::~Billboard()
{
}

void Billboard::createQuad(LPDIRECT3DDEVICE9 pd3dDevice, int indexNum, int width, int height)
{
	pd3dDevice->CreateVertexBuffer(6 * indexNum * sizeof(d3d::Vertex), D3DUSAGE_WRITEONLY,
		d3d::Vertex::FVF, D3DPOOL_MANAGED, &quad, 0);

	frameNum = indexNum;

	quad->Lock(0, 0, (void**)&v, 0);

	for (int i = 0; i < indexNum * 6; i += 6)
	{
		v[i] = d3d::Vertex(-100.0f, -100.0f, 0.0f, 0.0f, 0.0f, -100.0f, 
			((float)(i) / (float)(width * 6)) - (i / (width * 6)), 
			((float)(1 + (i / (width * 6))) / (float)height));

		v[i + 1] = d3d::Vertex(-100.0f, 100.0f, 0.0f, 0.0f, 0.0f, -100.0f, 
			((float)(i) / (float)(width * 6)) - (i / (width * 6)), 
			((float)(i / (width * 6)) / (float)height));

		v[i + 2] = d3d::Vertex(100.0f, 100.0f, 0.0f, 0.0f, 0.0f, -100.0f, 
			((float)(i + 6) / (float)(width * 6)) - (i / (width * 6)), 
			((float)(i / (width * 6)) / (float)height));

		v[i + 3] = d3d::Vertex(-100.0f, -100.0f, 0.0f, 0.0f, 0.0f, -100.0f, 
			((float)(i) / (float)(width * 6)) - (i / (width * 6)), 
			((float)(1 + (i / (width * 6))) / (float)height));
		v[i + 4] = d3d::Vertex(100.0f, 100.0f, 0.0f, 0.0f, 0.0f, -100.0f, 
			((float)(i + 6) / (float)(width * 6)) - (i / (width * 6)), 
			((float)(i / (width * 6)) / (float)height));
		v[i + 5] = d3d::Vertex(100.0f, -100.0f, 0.0f, 0.0f, 0.0f, -100.0f, 
			((float)(i + 6) / (float)(width * 6)) - (i / (width * 6)), 
			((float)(1 + (i / (width * 6))) / (float)height));
	}

	quad->Unlock();

	pd3dDevice->CreateIndexBuffer(6 * indexNum * sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &IB, 0);

	WORD* indices = 0;
	IB->Lock(0, 0, (void**)&indices, 0);

	for (int i = 0; i < indexNum * 6; i += 6)
	{
		indices[i] = 0; 
		indices[i + 1] = 1; 
		indices[i + 2] = 2;
		indices[i + 3] = 3; 
		indices[i + 4] = 4; 
		indices[i + 5] = 5;
	}

	IB->Unlock();

}

void Billboard::setPos(D3DXVECTOR3 vec)
{
	worldMatrix._41 = vec.x;
	worldMatrix._42 = vec.y;
	worldMatrix._43 = vec.z;
}

void Billboard::setMatrix(D3DXMATRIX* mat)
{
	worldMatrix._11 = mat->_11;
	worldMatrix._21 = mat->_21;
	worldMatrix._31 = mat->_31;

	worldMatrix._13 = mat->_13;
	worldMatrix._23 = mat->_23;
	worldMatrix._23 = mat->_23;
}

HRESULT Billboard::loadTexture(wchar_t* strTexName, LPDIRECT3DDEVICE9 pd3dDevice)
{
	HRESULT hr;
	hr = D3DXCreateTextureFromFile(pd3dDevice, strTexName, &tex);

	return hr;
}

void Billboard::scaleBillboard(float x, float y, float z)
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	D3DXMatrixScaling(&mat, x, y, z);
	worldMatrix = mat * worldMatrix;
}

void Billboard::drawBillboard(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta)
{
	pd3dDevice->SetTexture(0, tex);
	pd3dDevice->SetStreamSource(0, quad, 0, sizeof(d3d::Vertex));
	pd3dDevice->SetIndices(IB);
	pd3dDevice->SetFVF(d3d::Vertex::FVF);

	pd3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);

	static float countTimer = 0.0f;
	float runningTime = animTime;
	static int frameIndex = 0;	

	if (countTimer > ((float)frameIndex / frameNum) * runningTime)
		frameIndex += 1;

	if ((float)frameIndex / frameNum >= 1.0f)
	{
		frameIndex = 0.0f;
		countTimer = 0.0f;
	}
	else
		countTimer += timeDelta;

	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 6 * frameIndex, 0, 6, 0, 2);
}

void Billboard::drawBillboardOnce(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta)
{
	pd3dDevice->SetTexture(0, tex);
	pd3dDevice->SetStreamSource(0, quad, 0, sizeof(d3d::Vertex));
	pd3dDevice->SetIndices(IB);
	pd3dDevice->SetFVF(d3d::Vertex::FVF);

	pd3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);

	static float countTimer = 0.0f;
	float runningTime = animTime;
	static int frameIndex = 0;

	if (visible)
	{
		if (countTimer > ((float)frameIndex / frameNum) * runningTime)
			frameIndex += 1;

		if ((float)frameIndex / frameNum >= 1.0f)
		{
			frameIndex = 0.0f;
			countTimer = 0.0f;
			visible = false;
		}
		else
			countTimer += timeDelta;

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 6 * frameIndex, 0, 6, 0, 2);
	}
}

