#include "AlienSoldier.h"

void AlienSoldier::loadModel(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice)
{
	mModel.LoadXFile(strFileName, pd3dDevice);
	mModel.SelectAnimation(IDLE_ANIMATION);
}

void AlienSoldier::setMesh(LPD3DXFRAME frame, LPD3DXANIMATIONCONTROLLER animController)
{
	mModel.setMeshData(frame, animController);
	mModel.SelectAnimation(IDLE_ANIMATION);
}

void AlienSoldier::drawAlien(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta)
{
	if(!mIsAlive)
	{
		mModel.selectAnimationPlayOnce(DYING_ANIMAION, timeDelta);
		mModel.getAnimController()->SetTrackSpeed(0, 0.8f);
	}
	else
	{
		if (mLastState == DAMAGED_STATE)
		{
			static float startTime = 0.0f;
			startTime += timeDelta;
			if (startTime < 0.1f)
			{
				mModel.Draw(pd3dDevice, timeDelta);
				return;
			}
			else
				startTime = 0.0f;
		}

		if (mState == IDLE_STATE)
		{
			mModel.SelectAnimation(IDLE_ANIMATION);
			mLastState = IDLE_STATE;
		}
		else if (mState == TRACE_STATE)
		{
			mModel.SelectAnimation(RUN_ANIMATION);
			mLastState = TRACE_STATE;
		}
		else if (mState == ATTACK_STATE)
		{
			mModel.SelectAnimation(ATTACK_ANIMATION);
			mLastState = ATTACK_STATE;
		}
		else if (mState == DAMAGED_STATE)
		{
			mModel.SelectAnimation(DAMAGED_ANIMATION);
			mLastState = DAMAGED_STATE;
		}
	}
	mModel.Draw(pd3dDevice, timeDelta);
}

void AlienSoldier::moveAlienPos(float timeDelta)
{
	if (timeDelta > 0)
	{
		mModel.setModelPosition(mPos);
	}
}

void AlienSoldier::rotateAlien()
{
	D3DXVECTOR3 direction = mPos - mTraceEnemyPos;
	D3DXVECTOR3 alienLook;
	D3DXMATRIX tempMatrix;
	D3DXMatrixIdentity(&tempMatrix);
	float angle = 0.0f;
	float rotateAngle = 0.0f;

	alienLook.x = mModel.getworldMatrix()._31;
	alienLook.y = mModel.getworldMatrix()._32;
	alienLook.z = mModel.getworldMatrix()._33;

	D3DXVec3Normalize(&alienLook, &alienLook);
	D3DXVec3Normalize(&direction, &direction);
	rotateAngle = D3DXVec3Dot(&alienLook, &direction);
	rotateAngle = acos(rotateAngle);

	D3DXVECTOR3 cross;
	D3DXVec3Cross(&cross, &direction, &alienLook);

	if (rotateAngle > 0.1f)
	{
		if (cross.y > 0.0f)
			angle = -cross.y;
		else if (cross.y < 0.0f)
			angle = -cross.y;
		mModel.initRotateMatrix();
		D3DXMatrixRotationY(&tempMatrix, angle);
		mModel.rotateMatrixMultiply(tempMatrix);
	}
	else if (rotateAngle < 0.1f)
	{
		mModel.initRotateMatrix();
	}
}

void AlienSoldier::createSphere(LPDIRECT3DDEVICE9 pd3dDevice)
{
	D3DXCreateSphere(pd3dDevice, mBodyBoundingSphere._radius, 20, 20, &mBodySphere, 0);
	D3DXCreateSphere(pd3dDevice, mHeadBoundingSphere._radius, 20, 20, &mHeadSphere, 0);
}

void AlienSoldier::drawSphere(LPDIRECT3DDEVICE9 pd3dDevice)
{
	static D3DXMATRIX world;
	D3DXVECTOR3 alienLook;

	alienLook.x = mModel.getworldMatrix()._31;
	alienLook.y = mModel.getworldMatrix()._32;
	alienLook.z = mModel.getworldMatrix()._33;

	D3DXVec3Normalize(&alienLook, &alienLook);

	mBodyBoundingSphere._center = -23.0f * alienLook;

	D3DXMatrixIdentity(&world);

	world._41 += mPos.x + mBodyBoundingSphere._center.x;
	world._42 += mPos.y + 130.0f;
	world._43 += mPos.z + mBodyBoundingSphere._center.z;

	mBodyBoundingSphere._center.x = world._41;
	mBodyBoundingSphere._center.y = world._42;
	mBodyBoundingSphere._center.z = world._43;

	pd3dDevice->SetTransform(D3DTS_WORLD, &world);

	//mBodySphere->DrawSubset(0);

	/////////////////////////////////////////////////////////////

	mHeadBoundingSphere._center = -60.0f * alienLook;

	D3DXMatrixIdentity(&world);

	world._41 += mPos.x + mHeadBoundingSphere._center.x - 10.0f;
	world._42 += mPos.y + 160.0f;
	world._43 += mPos.z + mHeadBoundingSphere._center.z;

	mHeadBoundingSphere._center.x = world._41;
	mHeadBoundingSphere._center.y = world._42;
	mHeadBoundingSphere._center.z = world._43;

	pd3dDevice->SetTransform(D3DTS_WORLD, &world);

	//mHeadSphere->DrawSubset(0);
}