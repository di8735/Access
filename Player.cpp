#include "Player.h"

CPlayer::CPlayer()
{
	pos.x = 0.0f;
	pos.y = 0.0f;
	pos.z = 0.0f;
	charSpeed = 300.0f;
	isDrawPlayer = false;
	skelatonKeyDown = 1;
	mSkelatonDirection = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	mState = IDLE_ANIMATION;
	mHp = 100;
}

CPlayer::~CPlayer()
{
}

void CPlayer::loadModel(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice)
{
	mModel.LoadXFile(strFileName, pd3dDevice);
	mModel.SelectAnimation(IDLE_ANIMATION);
}

void CPlayer::movePlayerPos(float timeDelta)
{
	if (timeDelta > 0)
	{
		D3DXVECTOR3 playerDirection;
		playerDirection.x = mModel.getworldMatrix()._31;
		playerDirection.y = mModel.getworldMatrix()._32;
		playerDirection.z = mModel.getworldMatrix()._33;
		D3DXVec3Normalize(&playerDirection, &playerDirection);

		pos -= playerDirection * charSpeed * timeDelta;

		mModel.setModelPosition(pos);
		if (mState == 0)
			mModel.SelectAnimation(RUN_ANIMATION);
	}
}

void CPlayer::setSkelatonPosition(float timeDelta)
{
	if (timeDelta > 0)
		mModel.setModelPosition(pos);
}

void CPlayer::drawPlayer(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta)
{
	if (isDrawPlayer)
	{
		if (mIsFire)
		{
			if (mState == PLAYER_FRONT_RUN)
				mModel.SelectAnimation(RUN_FRONT_FIRE_ANIMATION);
			else if (mState == PLAYER_STOP)
				mModel.SelectAnimation(FIRE_ANIMATION);
			else if (mState == PLAYER_LEFT_RUN)
				mModel.SelectAnimation(RUN_RIGHT_FIRE_ANIMATION);
			else if (mState == PLAYER_RIGHT_RUN)
				mModel.SelectAnimation(RUN_LEFT_FIRE_ANIMATION);
		}

		if (mState == PLAYER_DAMAGED)
			mModel.SelectAnimation(DAMAGED_ANIMATION);
		else if (mHp <= 0)
		{
			mState = PLAYER_DEAD;
			mModel.selectAnimationPlayOnce(DYING_ANIMATION, timeDelta);
		}

		mModel.Draw(pd3dDevice, timeDelta);
	}
}

void CPlayer::drawSkelaton(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta)
{
	if (isDrawPlayer)
	{
		mModel.SelectAnimation(mState);
		mModel.Draw(pd3dDevice, timeDelta);
	}
}

void CPlayer::stopPlayer(int lastKey)
{
	mModel.initRotateMatrix();
	mModel.SelectAnimation(IDLE_ANIMATION);
}

float CPlayer::rotateYPlayer(D3DXVECTOR3 cameraVec)
{
	float angle = 0.0f;
	float rotateAngle = 0.0f;
	D3DXMATRIX tempMatrix;
	D3DXMatrixIdentity(&tempMatrix);

	D3DXVECTOR3 charDirection;

	charDirection.x = mModel.getworldMatrix()._31;
	charDirection.y = mModel.getworldMatrix()._32;
	charDirection.z = mModel.getworldMatrix()._33;
	
	D3DXVec3Normalize(&charDirection, &charDirection);
	D3DXVec3Normalize(&cameraVec, &cameraVec);
	rotateAngle = D3DXVec3Dot(&charDirection, &cameraVec);
	rotateAngle = acos(rotateAngle);

	D3DXVECTOR3 tempCross;
	D3DXVec3Cross(&tempCross, &cameraVec, &charDirection);

	if (rotateAngle > 0.1f)
	{
		angle = -tempCross.y;
		mModel.initRotateMatrix();
		D3DXMatrixRotationY(&tempMatrix, angle);
		mModel.rotateMatrixMultiply(tempMatrix);
	}
	else if (rotateAngle < 0.1f)
	{
		mModel.initRotateMatrix();
	}

	return angle;
}

void CPlayer::fireGun(bool fire, D3DXVECTOR3 cameraLookVec)
{
	if (fire)
	{
		mIsFire = true;
	}
}

void CPlayer::setMoveAnim(char animNum)
{
	mModel.SelectAnimation(animNum);
}

void CPlayer::skelatonRotate()
{
	float angle = 0.0f;
	float rotateAngle = 0.0f;
	D3DXMATRIX tempMatrix;
	D3DXMatrixIdentity(&tempMatrix);

	D3DXVECTOR3 oldCharDirection;
	D3DXVECTOR3 newCharDirection;

	oldCharDirection.x = mModel.getworldMatrix()._31;
	oldCharDirection.y = mModel.getworldMatrix()._32;
	oldCharDirection.z = mModel.getworldMatrix()._33;
	newCharDirection = mSkelatonDirection;

	D3DXVec3Normalize(&newCharDirection, &newCharDirection);
	D3DXVec3Normalize(&oldCharDirection, &oldCharDirection);
	rotateAngle = D3DXVec3Dot(&newCharDirection, &oldCharDirection);
	rotateAngle = acos(rotateAngle);

	D3DXVECTOR3 tempCross;
	D3DXVec3Cross(&tempCross, &newCharDirection, &oldCharDirection);

	if (rotateAngle > 0.1f)
	{
		angle = -tempCross.y;
		mModel.initRotateMatrix();
		D3DXMatrixRotationY(&tempMatrix, angle);
		mModel.rotateMatrixMultiply(tempMatrix);
	}
	else if (rotateAngle < 0.1f)
	{
		mModel.initRotateMatrix();
	}
}

int CPlayer::startRespawn(float timeDelta)
{
	static float waitingTime = 0.0f;
	if (mHp <= 0)
	{
		waitingTime += timeDelta;
		if (waitingTime >= RESPAWN_WAITING_TIME + RESPAWN_TIME)
		{
			waitingTime = 0.0f;
			return RESPAWN_NOW;
		}
		if (waitingTime >= RESPAWN_WAITING_TIME)
			return RESPAWNING;
	}
	
	return NOT_RESPAWN;
}

void CPlayer::initPlayerModel()
{
	mModel.initModel();
}