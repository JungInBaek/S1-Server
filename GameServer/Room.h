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
	bool HandleEnterPlayer(PlayerRef player);
	bool HandleLeavePlayer(PlayerRef player);

	void HandleMove(Protocol::C_MOVE pkt);

	RoomRef GetRoomRef();

private:
	bool EnterPlayer(PlayerRef player);
	bool LeavePlayer(uint64 objectId);

private:
	void Broadcast(SendBufferRef sendBuffer, uint64 exceptId = 0);

public:
	unordered_map<uint64, PlayerRef> _players;
};


extern RoomRef GRoom;