#pragma once
#include "JobQueue.h"


class Room : public JobQueue
{
public:
	Room();
	virtual ~Room();

public:
	bool EnterRoom(ObjectRef object, bool randPos = true);
	bool LeaveRoom(ObjectRef object);

public:
	bool HandleEnterPlayer(PlayerRef player);
	bool HandleLeavePlayer(PlayerRef player);
	void HandleTurn(Protocol::C_TURN pkt);
	void HandleMove(Protocol::C_MOVE pkt);
	void HandleFire(PlayerRef player);

public:
	void UpdateTick();

	RoomRef GetRoomRef();

private:
	bool AddObject(ObjectRef object);
	bool RemoveObject(uint64 objectId);

private:
	void Broadcast(SendBufferRef sendBuffer, uint64 exceptId = 0);

public:
	unordered_map<uint64, ObjectRef> _objects;
	/*unordered_map<uint64, PlayerRef> _players;
	unordered_map<uint64, MonsterRef> _monsters;*/
};


extern RoomRef GRoom;