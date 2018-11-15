#pragma once
#include "StateMachine.h"
#include "AlienOwnedStates.h"
#include <stdlib.h>

#define IDLE_STATE				4
#define ATTACK_STATE			3
#define TRACE_STATE				2
#define DYING_STATE				1
#define DAMAGED_STATE			0

class AlienSoldier
{
public:
	AlienSoldier(float x, float y, float z) : mHp(50)
	{
		mPos.x = x;
		mPos.y = y;
		mPos.z = z;		
		mTraceSpeed = 10.0f;
		state = IDLE_STATE;
		is_alive = true;
		m_pStateMachine = new StateMachine<AlienSoldier>(this);
		m_pStateMachine->SetCurrentState(Stand::Instance());
	}
	AlienSoldier() : mHp(50)
	{
		mPos.x = 0;
		mPos.y = 0;
		mPos.z = 0;
		stagePosition = 11;
		type = rand() % 3;
		is_alive = true;	
		state = IDLE_STATE;
		mTraceSpeed = 10.0f;
		m_pStateMachine = new StateMachine<AlienSoldier>(this);
		m_pStateMachine->SetCurrentState(Stand::Instance());
	}
	~AlienSoldier()
	{
		delete m_pStateMachine;
	}

	void updateStateMachine();
	StateMachine<AlienSoldier>*  GetFSM()const{ return m_pStateMachine; }
	Vector3 getPos() { return mPos; }
	Vector3 getTracePlayerPos() { return mTracePlayerPos; }
	void moveAlien(Vector3 dest);
	void setTracePlayerPos(Vector3 pos) { mTracePlayerPos.x = pos.x; mTracePlayerPos.y = pos.y; mTracePlayerPos.z = pos.z; }
	int states(){ return state; }
	int changeState_idle() { state = IDLE_STATE; m_pStateMachine->ChangeState(Stand::Instance()); return state; }
	int changeState_trace(){ state = TRACE_STATE; m_pStateMachine->ChangeState(TraceEnemy::Instance()); return state; }
	int changeState_attack(){ state = ATTACK_STATE; m_pStateMachine->ChangeState(AttackEnemy::Instance()); return state; }
	int changeState_dead() { state = DYING_STATE; m_pStateMachine->ChangeState(Dead::Instance()); return state; }
	int changeState_damaged() { state = DAMAGED_STATE; m_pStateMachine->ChangeState(Damaged::Instance()); return state; }
	bool getAlive() { return is_alive; }
	void setAlive(bool setValue) { is_alive = setValue; }
	int getHp() { return mHp; }
	void setHp(int hp){ mHp = hp; }
	void decreaseHp(int setValue) { mHp -= setValue; }
	int getAttackPlayerId(){ return mAttackPlayerId; }
	void setAttackPlayerId(int setValue) { mAttackPlayerId = setValue; }
	Vector3 setPos(int x, int y, int z){ mPos.x = x, mPos.y = y; mPos.z = z; return mPos; };
	int getStagePosition() { return stagePosition; }
	void setStagePosition(int setValue) { stagePosition = setValue; }
	int getType() { return type; }
	void setType(int setValue) { type = setValue; }
	std::vector<Node_2D *> m_path;

private:
	StateMachine<AlienSoldier>*  m_pStateMachine;
	int mHp;
	Vector3 mPos;
	Vector3 mTracePlayerPos;
	float mTraceSpeed;
	int type;
	int state;
	int stagePosition;
	bool is_alive;
	int mAttackPlayerId;
};

