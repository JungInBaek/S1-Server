#include "pch.h"
#include "ObjectUtils.h"
#include "Player.h"
#include "Enermy.h"
#include "ObjectPool.h"


// TODO: Bit Flag = [ObjectType][...][...][...]
atomic<int64> ObjectUtils::s_idGenerator = 1;


PlayerRef ObjectUtils::CreatePlayer(GameSessionRef session)
{
	const int64 newId = s_idGenerator.fetch_add(1);

	PlayerRef player = ObjectPool<Player>::MakeShared();

	player->objectInfo->set_object_id(newId);
	player->objectInfo->set_object_type(Protocol::OBJECT_TYPE_CREATURE);

	Protocol::CreatureInfo* creatureInfo = player->objectInfo->mutable_creature_info();
	creatureInfo->set_creature_type(Protocol::CREATURE_TYPE_PLAYER);
	
	Protocol::PlayerInfo* playerInfo = player->objectInfo->mutable_player_info();
	playerInfo->set_player_type(Protocol::PLAYER_TYPE_SURVIVOR);

	player->session = session;
	session->_player.store(player);

	return player;
}

EnermyRef ObjectUtils::CreateEnermy()
{
	const int64 newId = s_idGenerator.fetch_add(1);

	EnermyRef enermy = ObjectPool<Enermy>::MakeShared();

	enermy->objectInfo->set_object_id(newId);
	enermy->objectInfo->set_object_type(Protocol::OBJECT_TYPE_CREATURE);

	Protocol::CreatureInfo* creatureInfo = enermy->objectInfo->mutable_creature_info();
	creatureInfo->set_creature_type(Protocol::CREATURE_TYPE_ENERMY);

	Protocol::EnermyInfo* enermyInfo = enermy->objectInfo->mutable_enermy_info();
	enermyInfo->set_enermy_type(Protocol::ENERMY_TYPE_ZOMBIE);

	return enermy;
}
