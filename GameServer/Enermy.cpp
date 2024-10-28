#include "pch.h"
#include "Enermy.h"
#include "Player.h"
#include "Room.h"
#include "VectorUtils.h"
#include "PathFinder.h"


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

	/*float dirX = dir.x * 0.1f;
	float dirY = dir.y * 0.1f;
	float dirZ = dir.z * 0.1f;

	VectorUtils::Normalize(dir);

	float distanceX = dir.x * runSpeed * 0.1f;
	float distanceY = dir.y * runSpeed * 0.1f;
	float distanceZ = dir.z * runSpeed * 0.1f;
	
	distanceX = min(dirX, distanceX);
	distanceY = min(dirY, distanceY);
	distanceZ = min(dirZ, distanceZ);

	S1Vector nextLocation(currentLocation.x + distanceX, currentLocation.y + distanceY, currentLocation.z + distanceZ);
	this->position = nextLocation;*/

	/*S1Vector nextLocation;
	nextLocation.x = currentLocation.x + (dir.x * runSpeed * 0.1f);
	nextLocation.y = currentLocation.y + (dir.y * runSpeed * 0.1f);
	nextLocation.z = currentLocation.z + (dir.z * runSpeed * 0.1f);*/
	//nextLocation.z = 89.65;

	/*cout.precision(6);
	cout << "x: " << nextLocation.x << "  y: " << nextLocation.y << "  z: " << nextLocation.z << endl;*/
	
	
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

	this->position = path[pathIndex++];
	this->position.z = 89.65;
}

void Enermy::AttackState()
{
	elapsedTime = ::GetTickCount64() - startTime;
	if (elapsedTime >= attackDelayTime)
	{
		cout << "Attack!" << endl;
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
	elapsedTime = ::GetTickCount64() - startTime;
	if (elapsedTime >= damageDelayTime)
	{
		this->enermyState = Protocol::ENERMY_STATE_IDLE;
	}
}

void Enermy::DieState()
{
	S1Vector P0 = this->position;
	S1Vector v = VectorUtils::ScalarMultiplication(VectorUtils::DownVector(), dieSpeed);
	S1Vector vt = VectorUtils::ScalarMultiplication(v, 0.1f);
	S1Vector P = VectorUtils::Add(P0, vt);
	this->position = P;
	
	if (P.z < -200.0f)
	{
		room.load().lock().get()->DoAsync(&Room::LeaveRoom, shared_from_this());
	}
}
