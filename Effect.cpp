#include "Effect.h"


Effect::Effect()
{
	effect.setIsEffect(true);
	visible = false;
}


Effect::~Effect()
{
}

void Effect::loadEffect(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice)
{
	effect.LoadXFile(strFileName, pd3dDevice);
}

void Effect::rotateYawToCamera(D3DXVECTOR3 cameraPos)
{
	float angle = 0.0f;
	float rotateAngle = 0.0f;
	D3DXMATRIX tempMatrix;
	D3DXMatrixIdentity(&tempMatrix);

	D3DXVECTOR3 effectDirection;

	effectDirection.x = effect.getworldMatrix()._31;
	effectDirection.y = 0.0f;
	effectDirection.z = effect.getworldMatrix()._33;

	effectDirection = -effectDirection;

	D3DXVec3Normalize(&effectDirection, &effectDirection);
	D3DXVec3Normalize(&cameraPos, &cameraPos);
	rotateAngle = D3DXVec3Dot(&effectDirection, &cameraPos);
	rotateAngle = acos(rotateAngle);

	D3DXVECTOR3 tempCross;
	D3DXVec3Cross(&tempCross, &cameraPos, &effectDirection);

	if (rotateAngle > 0.1f)
	{
		angle = -tempCross.y;
		effect.initRotateMatrix();
		D3DXMatrixRotationY(&tempMatrix, angle);
		effect.rotateMatrixMultiply(tempMatrix);
	}
	else if (rotateAngle < 0.1f)
	{
		effect.initRotateMatrix();
	}
}

void Effect::rotatePitchToCamera(D3DXVECTOR3 cameraPos)
{
	float angle = 0.0f;
	float rotateAngle = 0.0f;
	D3DXMATRIX tempMatrix;
	D3DXMatrixIdentity(&tempMatrix);

	D3DXVECTOR3 effectDirection;

	effectDirection.x = 0.0f;
	effectDirection.y = effect.getworldMatrix()._32;
	effectDirection.z = effect.getworldMatrix()._33;

	effectDirection = -effectDirection;

	D3DXVec3Normalize(&effectDirection, &effectDirection);
	D3DXVec3Normalize(&cameraPos, &cameraPos);
	rotateAngle = D3DXVec3Dot(&effectDirection, &cameraPos);
	rotateAngle = acos(rotateAngle);

	D3DXVECTOR3 tempCross;
	D3DXVec3Cross(&tempCross, &cameraPos, &effectDirection);

	if (rotateAngle > 0.1f)
	{
		angle = -tempCross.y;
		effect.initRotateMatrix();
		D3DXMatrixRotationX(&tempMatrix, angle);
		effect.rotateMatrixMultiply(tempMatrix);
	}
	else if (rotateAngle < 0.1f)
	{
		effect.initRotateMatrix();
	}
}

void Effect::drawEffect(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta)
{
	if (visible)
	{
		effect.SelectAnimation(0);
		effect.Draw(pd3dDevice, timeDelta);
	}
}

void Effect::rotateY(float angle)
{
	D3DXMATRIX rotate;
	D3DXMatrixRotationY(&rotate, angle);
	effect.setworldMatrix(rotate);
}