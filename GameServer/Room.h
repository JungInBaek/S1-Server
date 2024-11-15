#pragma once
#include "JobQueue.h"


class Room : public JobQueue
{
public:
	Room();
	virtual ~Room();
	
public:
	void Init();
	void UpdateTick();

public:
	bool EnterRoom(ObjectRef object, bool randPos = true);
	bool LeaveRoom(ObjectRef object);
	bool AttackEnermy(EnermyRef enermy);

public:
	bool HandleEnterPlayer(PlayerRef player);
	bool HandleLeavePlayer(PlayerRef player);
	void HandleTurn(Protocol::C_TURN pkt);
	void HandleMove(Protocol::C_MOVE pkt);
	void HandleState(Protocol::C_STATE pkt);
	void HandleFire(PlayerRef player);
	void HandleChangeItem(Protocol::C_CHANGE_ITEM pkt);
	void HandleSniperFire(Protocol::C_SNIPER_FIRE pkt);
	void HandleDamageEnermy(Protocol::C_DAMAGE_ENERMY pkt);

public:
	RoomRef GetRoomRef();

private:
	bool AddObject(ObjectRef object);
	bool RemoveObject(uint64 objectId);

private:
	void Broadcast(SendBufferRef sendBuffer, uint64 exceptId = 0);

public:
	unordered_map<uint64, ObjectRef> _objects;
	unordered_map<uint64, PlayerRef> _players;
	unordered_map<uint64, EnermyRef> _enermies;

public:
	PathFinderRef pathFinder;
};


extern RoomRef GRoom;