#pragma once


class Object : public enable_shared_from_this<Object>
{
public:
	Object();
	virtual ~Object();

	bool IsPlayer() { return _isPlayer; }
	bool IsEnermy() { return _isEnermy; }

public:
	Protocol::ObjectInfo* objectInfo;
	Protocol::PosInfo* posInfo;

public:
	atomic<weak_ptr<class Room>> room;

protected:
	bool _isPlayer = false;
	bool _isEnermy = false;
};

