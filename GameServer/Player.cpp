#include "pch.h"
#include "Player.h"


Player::Player()
{
	_isPlayer = true;
}

Player::~Player()
{
	cout << "~Player" << endl;
	session = weak_ptr<GameSession>();
}

Protocol::ObjectInfo Player::changeToPacket()
{
	Protocol::ObjectInfo object = Creature::changeToPacket();

	Protocol::PlayerInfo* player = object.mutable_player_info();
	player->set_player_type(playerType);
	player->set_player_state(playerState);
	
	return object;
}
