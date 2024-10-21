#include "pch.h"
#include "Creature.h"


Creature::Creature()
{
	
}

Creature::~Creature()
{
}

Protocol::ObjectInfo Creature::changeToPacket()
{
	Protocol::ObjectInfo object = Object::changeToPacket();

	Protocol::CreatureInfo* creature = object.mutable_creature_info();
	creature->set_creature_type(creatureType);
	creature->set_hp(hp);

	return object;
}
