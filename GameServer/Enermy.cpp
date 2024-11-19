#include "pch.h"
#include "Enermy.h"
#include "Player.h"
#include "Room.h"
#include "VectorUtils.h"
#include "PathFinder.h"
#include "math.h"


Enermy::Enermy()
{
	_isEnermy = true;
}

Enermy::~Enermy()
{
	cout << "~Enermy" << endl;
	targetPlayer = weak_ptr<Player>();
}

Protocol::ObjectInfo Enermy::changeToPacket()
{
	Protocol::ObjectInfo object = Creature::changeToPacket();

	Protocol::EnermyInfo* enermy = object.mutable_enermy_info();
	enermy->set_enermy_type(enermyType);
	enermy->set_enermy_state(enermyState);

	return object;
}

void Enermy::IdleState()
{
	elapsedTime = ::GetTickCount64() - startTime;
	if (elapsedTime >= idleDelayTime)
	{
		this->enermyState = Protocol::ENERMY_STATE_MOVE;
	}
}

void Enermy::MoveState()
{
	S1Vector currentLocation = this->position;
	S1Vector destLocation = targetPlayer.lock()->position;

	S1Vector dir = destLocation - currentLocation;
	float size = VectorUtils::Size(dir);

	currentLocation.x = GRoom->pathFinder->GetApproximationValue(currentLocation.x);
	currentLocation.y = GRoom->pathFinder->GetApproximationValue(currentLocation.y);
	currentLocation.z = 0;

	destLocation.x = GRoom->pathFinder->GetApproximationValue(destLocation.x);
	destLocation.y = GRoom->pathFinder->GetApproximationValue(destLocation.y);
	destLocation.z = 0;

	if (size <= attackRange)
	{
		this->enermyState = Protocol::ENERMY_STATE_ATTACK;
		return;
	}

	if (pathIndex == path.size())
	{
		path = GRoom->pathFinder->AStar(currentLocation, destLocation);
		pathIndex = 0;
		/*for (auto p : path)
		{
			cout << "(" << p.x << ", " << p.y << ", " << p.z << ")";
			if (path.back() != p)
			{
				cout << " -> ";
			}
			else
			{
				cout << endl;
			}
		}*/
	}

	if (path.empty())
	{
		cout << "경로 없음" << endl;
		return;
	}

	this->position = path[pathIndex++];
	this->position.z += 89.65f;
}

void Enermy::AttackState()
{
	elapsedTime = ::GetTickCount64() - startTime;
	if (elapsedTime >= attackDelayTime)
	{
		//cout << "Attack!" << endl;
		GRoom->DoAsync(&Room::AttackEnermy, static_pointer_cast<Enermy>(shared_from_this()));
		startTime = ::GetTickCount64();
	}

	float distance = VectorUtils::Distance(targetPlayer.lock()->position, this->position);
	if (distance > attackRange)
	{
		this->enermyState = Protocol::ENERMY_STATE_MOVE;
	}
}

void Enermy::DamageState()
{
	startTime = ::GetTickCount64();
	if (hp <= 0)
	{
		enermyState = Protocol::ENERMY_STATE_DIE;
		return;
	}
	this->enermyState = Protocol::ENERMY_STATE_IDLE;
}

void Enermy::DieState()
{
	elapsedTime = ::GetTickCount64() - startTime;
	if (elapsedTime >= dieDelayTime)
	{
		room.load().lock().get()->DoAsync(&Room::LeaveRoom, shared_from_this());
	}
}
