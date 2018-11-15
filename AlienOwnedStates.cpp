#include "AlienOwnedStates.h"
#include "AlienSoldier.h"
#include <cmath>

Stand* Stand::Instance()
{
	static Stand instance;

	return &instance;
}

void Stand::Enter(AlienSoldier* alien)
{
}

void Stand::Execute(AlienSoldier* alien)
{
}

void Stand::Exit(AlienSoldier* alien)
{
}

/////////////////////////////////////////////////////////////

TraceEnemy* TraceEnemy::Instance()
{
	static TraceEnemy instance;

	return &instance;
}

void TraceEnemy::Enter(AlienSoldier* alien)
{
}

void TraceEnemy::Execute(AlienSoldier* alien)
{
	// 길찾기

	Vector3 tracePos, alienPos;

	/*m_pathFinder.FindPath(alien->getPos().x, alien->getPos().z, alien->getTracePlayerPos().x, alien->getTracePlayerPos().z, &(alien->m_path));
	Vector3 direction;
	if (alien->m_path.size() > 0){
		direction.x = (*(alien->m_path.end() - 1))->m_x - alien->getPos().x;
		direction.y = 0;
		direction.z = (*(alien->m_path.end() - 1))->m_y - alien->getPos().z;
	}*/

	
	tracePos.x = alien->getTracePlayerPos().x;
	tracePos.y = alien->getTracePlayerPos().y;
	tracePos.z = alien->getTracePlayerPos().z;
	alienPos.x = alien->getPos().x;
	alienPos.y = alien->getPos().y;
	alienPos.z = alien->getPos().z;
	
	// tracePos 가 길찾기 결과 노드의 첫번째 노드가 되야됨.

	Vector3 direction;
	direction.x = tracePos.x - alienPos.x;
	direction.y = tracePos.y - alienPos.y;
	direction.z = tracePos.z - alienPos.z;
	

	float size = sqrt(pow(tracePos.x - alienPos.x, 2) + pow(tracePos.y - alienPos.y, 2) + pow(tracePos.z - alienPos.z, 2));
	direction.x = direction.x / size;
	direction.y = direction.y / size;
	direction.z = direction.z / size;

	alien->moveAlien(direction);
}

void TraceEnemy::Exit(AlienSoldier* alien)
{
}

///////////////////////////////////////////////////////

AttackEnemy* AttackEnemy::Instance()
{
	static AttackEnemy instance;

	return &instance;
}

void AttackEnemy::Enter(AlienSoldier* alien)
{
}

void AttackEnemy::Execute(AlienSoldier* alien)
{

}

void AttackEnemy::Exit(AlienSoldier* alien)
{
}

Dead* Dead::Instance()
{
	static Dead instance;

	return &instance;
}

void Dead::Enter(AlienSoldier* alien)
{
}

void Dead::Execute(AlienSoldier* alien)
{

}

void Dead::Exit(AlienSoldier* alien)
{
}

Damaged* Damaged::Instance()
{
	static Damaged instance;

	return &instance;
}

void Damaged::Enter(AlienSoldier* alien)
{
}

void Damaged::Execute(AlienSoldier* alien)
{

}

void Damaged::Exit(AlienSoldier* alien)
{
}