#pragma once
#include "Object.h"


class Creature : public Object
{
public:
	Creature();
	virtual ~Creature();

public:
	Protocol::ObjectInfo changeToPacket() override;

public:
	Protocol::CreatureType creatureType = Protocol::CREATURE_TYPE_NONE;
	int32 hp = 100;

	float attackRange = 100.0f;
	float runSpeed = 500.f;
	float dieSpeed = 50.f;
};

