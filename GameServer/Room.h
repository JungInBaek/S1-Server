#pragma once
#include "JobQueue.h"


//class Room : public JobQueue
//{
//public:
//	void Enter(PlayerRef player);
//	void Leave(PlayerRef player);
//	void Broadcast(SendBufferRef sendBuffer);
//
//private:
//	map<uint64, PlayerRef> _players;
//};


class Room : public JobQueue
{
public:
	Room();
	virtual ~Room();

public:
	bool HandleEnterPlayerLocked(PlayerRef player);
	bool EnterPlayer(PlayerRef player);

private:
	void Broadcast(SendBufferRef sendBuffer, uint64 exceptId = 0);

public:
	USE_LOCK;
	unordered_map<uint64, PlayerRef> _players;
};


extern RoomRef GRoom;