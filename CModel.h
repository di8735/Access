#pragma once
#include "d3dUtility.h"

class CModel
{
public:
	CModel();
	~CModel();
	void setworldMatrix(D3DXMATRIX setValue) { worldMatrix = setValue; }
	void setroateMatrix(D3DXMATRIX setValue) { rotateMatrix = setValue; }

	D3DXMATRIX getworldMatrix() { return worldMatrix; }
	D3DXMATRIX getrotateMatrix() { return rotateMatrix; }
	LPD3DXANIMATIONCONTROLLER getAnimController() { return m_pAnimController; }
	void initModel();
	void initRotateMatrix() { D3DXMatrixIdentity(&rotateMatrix); }
	void rotateMatrixMultiply(D3DXMATRIX mat) { D3DXMatrixMultiply(&rotateMatrix, &mat, &rotateMatrix); }
	void SelectAnimation(int n);
	void selectAnimationPlayOnce(int n, float timeDelta);
	void Draw(IDirect3DDevice9* pd3dDevice, float timeDelta);
	void LoadXFile(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice);
	void setMeshData(LPD3DXFRAME frameRoot, LPD3DXANIMATIONCONTROLLER animController);
	void setModelPosition(D3DXVECTOR3 pos) 
	{
		worldMatrix._41 = pos.x;
		worldMatrix._42 = pos.y;
		worldMatrix._43 = pos.z;
	}
	void setIsEffect(bool setValue) { mIsEffect = setValue; }
	void setScale(D3DXVECTOR3 setValue);
	LPD3DXFRAME getFrameData() { return m_pFrameRoot; }
	int getCurrAnimNum() { return mCurrAnimNum; }
private:
	CAllocateHierarchy Alloc;
	LPD3DXANIMATIONCONTROLLER m_pAnimController;
	LPD3DXFRAME m_pFrameRoot;
	void DrawMeshContainer(IDirect3DDevice9* pd3dDevice, LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase);
	void DrawFrame(IDirect3DDevice9* pd3dDevice, LPD3DXFRAME pFrame);
	HRESULT SetupBoneMatrixPointers(LPD3DXFRAME pFrame);
	HRESULT SetupBoneMatrixPointersOnMesh(LPD3DXMESHCONTAINER pMeshContainerBase);
	D3DXMATRIX worldMatrix;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX scaleMatrix;
	bool mIsEffect;
	int mCurrAnimNum;
	float mAnimTime;
};