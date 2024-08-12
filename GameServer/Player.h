#pragma once


//class Player
//{
//public:
//	uint64 playerId = 0;
//	string name;
//	Protocol::PlayerType type = Protocol::PLAYER_TYPE_NONE;
//	GameSessionRef ownerSession;	// Cycle
//};


class Player : public enable_shared_from_this<Player>
{
public:
	Player();
	virtual ~Player();

public:
	Protocol::PlayerInfo* playerInfo;
	weak_ptr<class GameSession> session;

public:
	atomic<weak_ptr<class Room>> room;
};