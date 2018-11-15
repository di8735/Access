#pragma once
#include "State.h"
#include "PathFinder.h"

class AlienSoldier;

static Map m_Map;
static PathFinder m_pathFinder(&m_Map);

class Stand : public State<AlienSoldier>
{
private:
	Stand(){}

	//copy ctor and assignment should be private
	Stand(const Stand&);
	Stand& operator=(const Stand&);
public:

	static Stand* Instance();

public:

	virtual void Enter(AlienSoldier* alien);

	virtual void Execute(AlienSoldier* alien);

	virtual void Exit(AlienSoldier* alien);

};

class TraceEnemy : public State<AlienSoldier>
{
private:

	TraceEnemy(){}

	//copy ctor and assignment should be private
	TraceEnemy(const TraceEnemy&);
	TraceEnemy& operator=(const TraceEnemy&);

public:

	static TraceEnemy* Instance();

	virtual void Enter(AlienSoldier* Alien);

	virtual void Execute(AlienSoldier* Alien);

	virtual void Exit(AlienSoldier* Alien);
};

class AttackEnemy : public State<AlienSoldier>
{
private:

	AttackEnemy(){}

	//copy ctor and assignment should be private
	AttackEnemy(const AttackEnemy&);
	AttackEnemy& operator=(const AttackEnemy&);

public:

	static AttackEnemy* Instance();

	virtual void Enter(AlienSoldier* Alien);

	virtual void Execute(AlienSoldier* Alien);

	virtual void Exit(AlienSoldier* Alien);
};

class Dead : public State<AlienSoldier>
{
private:

	Dead(){}

	//copy ctor and assignment should be private
	Dead(const Dead&);
	Dead& operator=(const Dead&);

public:

	static Dead* Instance();

	virtual void Enter(AlienSoldier* Alien);

	virtual void Execute(AlienSoldier* Alien);

	virtual void Exit(AlienSoldier* Alien);
};

class Damaged : public State<AlienSoldier>
{
private:

	Damaged(){}

	//copy ctor and assignment should be private
	Damaged(const Damaged&);
	Damaged& operator=(const Damaged&);

public:

	static Damaged* Instance();

	virtual void Enter(AlienSoldier* Alien);

	virtual void Execute(AlienSoldier* Alien);

	virtual void Exit(AlienSoldier* Alien);
};