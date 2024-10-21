#pragma once
#include "S1Vector.h"


class Object : public enable_shared_from_this<Object>
{
public:
	Object();
	virtual ~Object();

	bool IsPlayer() { return _isPlayer; }
	bool IsEnermy() { return _isEnermy; }

	virtual Protocol::ObjectInfo changeToPacket();

public:
	uint64 objectId;
	Protocol::ObjectType objectType;
	S1Vector position;
	float yaw;

public:
	atomic<weak_ptr<class Room>> room;

protected:
	bool _isPlayer = false;
	bool _isEnermy = false;
};

