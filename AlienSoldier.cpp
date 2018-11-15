#include "AlienSoldier.h"

void AlienSoldier::updateStateMachine()
{
	m_pStateMachine->Update();
}

void AlienSoldier::moveAlien(Vector3 dest)
{
	mPos.x += dest.x * mTraceSpeed;
	mPos.y += dest.y * mTraceSpeed;
	mPos.z += dest.z * mTraceSpeed;
}