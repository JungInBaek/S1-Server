#include "pch.h"
#include "ObjectUtils.h"
#include "Player.h"
#include "Enermy.h"
#include "ObjectPool.h"


// TODO: Bit Flag = [ObjectType][...][...][...]
atomic<int64> ObjectUtils::s_idGenerator = 1;


PlayerRef ObjectUtils::CreatePlayer(GameSessionRef session, Protocol::PlayerType playerType, int32 hp)
{
	const int64 newId = s_idGenerator.fetch_add(1);

	PlayerRef player = ObjectPool<Player>::MakeShared();
	player->objectId = newId;
	player->objectType = Protocol::OBJECT_TYPE_CREATURE;
	player->creatureType = Protocol::CREATURE_TYPE_PLAYER;
	player->hp = hp;
	player->playerType = playerType;
	player->playerState = Protocol::PLAYER_STATE_IDLE;

	player->session = session;
	session->_player.store(player);

	return player;
}

EnermyRef ObjectUtils::CreateEnermy(Protocol::EnermyType enermyType, int hp)
{
	const int64 newId = s_idGenerator.fetch_add(1);

	EnermyRef enermy = ObjectPool<Enermy>::MakeShared();
	enermy->objectId = newId;
	enermy->objectType = Protocol::OBJECT_TYPE_CREATURE;
	enermy->creatureType = Protocol::CREATURE_TYPE_ENERMY;
	enermy->hp = hp;
	enermy->enermyType = enermyType;
	enermy->enermyState = Protocol::ENERMY_STATE_IDLE;

	return enermy;
}
