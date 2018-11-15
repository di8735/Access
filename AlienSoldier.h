#pragma once
#include "CModel.h"

#define IDLE_ANIMATION			6
#define ATTACK_ANIMATION		5
#define RUN_ANIMATION			4
#define DYING_ANIMAION			3
#define DAMAGED_ANIMATION		2

#define IDLE_STATE				4
#define ATTACK_STATE			3
#define TRACE_STATE				2
#define DYING_STATE				1
#define DAMAGED_STATE			0

class AlienSoldier
{
public:
	AlienSoldier(D3DXVECTOR3 pos) : mHp(100), mPos(pos)
	{
		mBodyBoundingSphere._center.x = 0.0f;
		mBodyBoundingSphere._center.y = 0.0f;
		mBodyBoundingSphere._center.z = 0.0f;
		mBodyBoundingSphere._radius = 30.0f;

		mHeadBoundingSphere._center.x = 0.0f;
		mHeadBoundingSphere._center.y = 0.0f;
		mHeadBoundingSphere._center.z = 0.0f;
		mHeadBoundingSphere._radius = 30.0f;

		mState = IDLE_STATE;
		mIsAlive = true;
		mLastState = IDLE_STATE;
		mType = rand() % 2;
	}
	AlienSoldier() : mHp(100)
	{
		mBodyBoundingSphere._center.x = 0.0f;
		mBodyBoundingSphere._center.y = 0.0f;
		mBodyBoundingSphere._center.z = 0.0f;
		mBodyBoundingSphere._radius = 30.0f;

		mHeadBoundingSphere._center.x = 0.0f;
		mHeadBoundingSphere._center.y = 0.0f;
		mHeadBoundingSphere._center.z = 0.0f;
		mHeadBoundingSphere._radius = 30.0f;

		mState = IDLE_STATE;
		mIsAlive = true;
		mLastState = IDLE_STATE;
		mType = rand() % 2;
	}
	~AlienSoldier()
	{
	}

	void loadModel(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice);
	void setMesh(LPD3DXFRAME frame, LPD3DXANIMATIONCONTROLLER animController);
	LPD3DXFRAME getFrame() { return mModel.getFrameData(); }
	LPD3DXANIMATIONCONTROLLER getAnimController() { return mModel.getAnimController(); }
	void drawAlien(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta);
	void moveAlienPos(float timeDelta);
	void rotateAlien();
	void createSphere(LPDIRECT3DDEVICE9 pd3dDevice);
	void drawSphere(LPDIRECT3DDEVICE9 pd3dDevice);

	void setPos(D3DXVECTOR3 setValue) { mPos = setValue; }
	void setTraceEnemyPos(D3DXVECTOR3 setValue) { mTraceEnemyPos = setValue; }
	void setState(int setValue) { mState = setValue; }
	void setAlive(bool setValue) { mIsAlive = setValue; }
	void setLastStateAnim() { mModel.SelectAnimation(mLastState); }
	D3DXVECTOR3 getPos() { return mPos; }
	int getState() { return mState; }
	bool getAlive() { return mIsAlive; }
	CModel getModel() { return mModel; }
	void setModel(CModel& model) { mModel = model; }
	int getType() { return mType; }
	void setType(int setValue) { mType = setValue;; }
	d3d::BoundingSphere getBodyBoundingSphere() { return mBodyBoundingSphere; }
	d3d::BoundingSphere getHeadBoundingSphere() { return mHeadBoundingSphere; }
private:
	int mHp;
	D3DXVECTOR3 mPos;
	CModel mModel;
	D3DXVECTOR3 mTraceEnemyPos;
	d3d::BoundingSphere mHeadBoundingSphere;
	d3d::BoundingSphere mBodyBoundingSphere;
	ID3DXMesh* mHeadSphere;
	ID3DXMesh* mBodySphere;
	int mState;
	int mType;
	bool mIsAlive;
	int mLastState;
};

