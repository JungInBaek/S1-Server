#pragma once
#include "Creature.h"


class Player : public Creature
{
public:
	Player();
	virtual ~Player();

public:
	Protocol::ObjectInfo changeToPacket() override;

public:
	Protocol::PlayerType playerType = Protocol::PLAYER_TYPE_NONE;
	Protocol::PlayerState playerState = Protocol::PLAYER_STATE_NONE;

public:
	weak_ptr<class GameSession> session;
};