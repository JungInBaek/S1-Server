#pragma once
#include "Creature.h"


class Enermy : public Creature
{
public:
	Enermy();
	virtual ~Enermy();

public:
	Protocol::ObjectInfo changeToPacket() override;

public:
	void IdleState();
	void MoveState();
	void AttackState();
	void DamageState();
	void DieState();

public:
	Protocol::EnermyType enermyType = Protocol::ENERMY_TYPE_ZOMBIE;
	Protocol::EnermyState enermyState = Protocol::ENERMY_STATE_NONE;

	weak_ptr<class Player> targetPlayer;
	Vector<S1Vector> path;
	int32 pathIndex = 0;

	uint64 startTime = ::GetTickCount64();
	uint64 elapsedTime = 0;
	uint64 idleDelayTime = 2000;
	uint64 attackDelayTime = 2000;
	uint64 damageDelayTime = 2000;
	uint64 dieDelayTime = 5000;
};

