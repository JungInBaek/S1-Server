#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "Enermy.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ObjectUtils.h"
#include "VectorUtils.h"
#include "PathFinder.h"


RoomRef GRoom = MakeShared<Room>();

Room::Room()
{
	pathFinder = MakeShared<PathFinder>();
}

Room::~Room()
{
}

void Room::Init()
{
	pathFinder->ReadFile();
	auto start = S1Vector(-3500.f, -3400.f, 0.f);
	auto end = S1Vector(-3100.f, -3200.f, 0.f);
	auto path = pathFinder->AStar(start, end);

	for (auto p : path)
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
	}

	EnermyRef enermy = ObjectUtils::CreateEnermy(Protocol::ENERMY_TYPE_ZOMBIE, 150);
	EnterRoom(enermy);
}

bool Room::EnterRoom(ObjectRef object, bool randPos)
{
	bool success = AddObject(object);

	// 랜덤 위치
	if (randPos)
	{
		S1Vector position;
		object->position.x = Utils::GetRandom(-500.0f, 500.f);
		object->position.y = Utils::GetRandom(-500.0f, 500.f);
		object->position.z = 89.65f;
		object->yaw = Utils::GetRandom(0.f, 100.f);
	}

	// 접속 플레이어에게 입장 패킷 전송
	if (object->IsPlayer())
	{
		PlayerRef player = static_pointer_cast<Player>(object);

		Protocol::S_ENTER_GAME enterGamePkt;
		enterGamePkt.set_success(success);
		
		Protocol::ObjectInfo* objectInfo = enterGamePkt.mutable_player();
		objectInfo->CopyFrom(player->changeToPacket());

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
		if (auto session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}

	// 접속 플레이어에게 기존 오브젝트들의 스폰 패킷 전송
	if (object->IsPlayer())
	{
		PlayerRef player = static_pointer_cast<Player>(object);

		Protocol::S_SPAWN spawnPkt;
		for (std::pair<const uint64, PlayerRef>& item : _players)
		{
			// 자기 자신의 Player(Object)는 클라이언트에서 예외 처리
			Protocol::ObjectInfo* objectInfo = spawnPkt.add_objects();
			objectInfo->CopyFrom(item.second->changeToPacket());
		}
		for (std::pair<const uint64, EnermyRef>& item : _enermies)
		{
			Protocol::ObjectInfo* objectInfo = spawnPkt.add_objects();
			objectInfo->CopyFrom(item.second->changeToPacket());
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		if (auto session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}

	// 기존 플레이어들에게 새로운 Object의 스폰 패킷 전송
	{
		Protocol::S_SPAWN spawnPkt;

		Protocol::ObjectInfo* objectInfo = spawnPkt.add_objects();
		if (object->IsPlayer())
		{
			PlayerRef player = static_pointer_cast<Player>(object);
			objectInfo->CopyFrom(player->changeToPacket());
		}
		else if (object->IsEnermy())
		{
			EnermyRef enermy = static_pointer_cast<Enermy>(object);
			objectInfo->CopyFrom(enermy->changeToPacket());
		}
		else
		{
			objectInfo->CopyFrom(object->changeToPacket());
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		Broadcast(sendBuffer, object->objectId);
	}

	return success;
}

bool Room::LeaveRoom(ObjectRef object)
{
	if (object == nullptr)
	{
		return false;
	}

	const uint64 objectId = object->objectId;
	bool success = RemoveObject(objectId);
	if (!success)
	{
		return success;
	}

	// 퇴장 플레이어에게 퇴장 패킷 전송
	if (object->IsPlayer())
	{
		PlayerRef player = static_pointer_cast<Player>(object);

		Protocol::S_LEAVE_GAME leavePkt;

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(leavePkt);
		if (GameSessionRef session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}

	// 남아있는 플레이어들에게 퇴장 플레이어의 디스폰 패킷 전송
	{
		Protocol::S_DESPAWN despawnPkt;
		despawnPkt.add_object_ids(objectId);

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(despawnPkt);
		Broadcast(sendBuffer, objectId);

		// 퇴장 플레이어는 이미 room에서 빠져나간 상태라 Broadcast 불가
		if (object->IsPlayer())
		{
			PlayerRef player = static_pointer_cast<Player>(object);
			if (GameSessionRef session = player->session.lock())
			{
				session->Send(sendBuffer);
				GSessionManager.Remove(session);
			}
		}
	}

	return success;
}

bool Room::AttackEnermy(EnermyRef enermy)
{
	Protocol::S_ATTACK_ENERMY attackPkt;
	attackPkt.set_object_id(enermy->objectId);
	attackPkt.set_target_id(enermy->targetPlayer.lock()->objectId);
	attackPkt.set_damage(enermy->att);

	Broadcast(ClientPacketHandler::MakeSendBuffer(attackPkt));

	return true;
}

bool Room::HandleEnterPlayer(PlayerRef player)
{
	return EnterRoom(player);
}

bool Room::HandleLeavePlayer(PlayerRef player)
{
	return LeaveRoom(player);
}

void Room::HandleTurn(Protocol::C_TURN pkt)
{
	const uint64 objectId = pkt.object_id();
	if (_objects.find(objectId) == _objects.end())
	{
		return;
	}

	if (_objects[objectId]->IsPlayer() == false)
	{
		return;
	}

	PlayerRef player = static_pointer_cast<Player>(_objects[objectId]);
	float yaw = pkt.yaw();

	{
		Protocol::S_TURN turnPkt;
		turnPkt.set_object_id(objectId);
		turnPkt.set_yaw(yaw);
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(turnPkt);
		Broadcast(sendBuffer, objectId);
	}
}

void Room::HandleMove(Protocol::C_MOVE pkt)
{
	const uint64 objectId = pkt.object_id();
	if (_objects.find(objectId) == _objects.end())
	{
		return;
	}

	// TODO: 이동 패킷 Validation (속도, 충돌 등)

	if (_objects[objectId]->IsPlayer())
	{
		// 적용
		PlayerRef player = static_pointer_cast<Player>(_objects[objectId]);
		Protocol::VectorInfo vectorInfo = pkt.info().vector_info();
		player->position = S1Vector(vectorInfo.x(), vectorInfo.y(), vectorInfo.z());
		player->yaw = pkt.info().yaw();
	}

	// 이동
	{
		Protocol::S_MOVE movePkt;
		movePkt.set_object_id(objectId);
		movePkt.mutable_info()->CopyFrom(pkt.info());

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
		Broadcast(sendBuffer, objectId);
	}
}

void Room::HandleState(Protocol::C_STATE pkt)
{
	const uint64 objectId = pkt.object_id();
	Protocol::S_STATE statePkt;
	statePkt.set_object_id(objectId);
	if (pkt.player_state() != Protocol::PLAYER_STATE_NONE)
	{
		statePkt.set_player_state(pkt.player_state());
	}
	else if (pkt.enermy_state() != Protocol::ENERMY_STATE_NONE)
	{
		statePkt.set_enermy_state(pkt.enermy_state());
	}
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(statePkt);
	Broadcast(sendBuffer, objectId);
}

void Room::HandleFire(PlayerRef player)
{
	const uint64 objectId = player->objectId;
	Protocol::S_FIRE firePkt;
	firePkt.set_object_id(objectId);
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(firePkt);
	Broadcast(sendBuffer, objectId);
}

void Room::HandleChangeItem(Protocol::C_CHANGE_ITEM pkt)
{
	Protocol::S_CHANGE_ITEM changePkt;
	changePkt.set_object_id(pkt.object_id());
	changePkt.set_key(pkt.key());
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(changePkt);
	Broadcast(sendBuffer, pkt.object_id());
}

void Room::HandleSniperFire(Protocol::C_SNIPER_FIRE pkt)
{
	Protocol::S_SNIPER_FIRE firePkt;
	firePkt.set_object_id(pkt.object_id());
	firePkt.mutable_start()->CopyFrom(pkt.start());
	firePkt.mutable_end()->CopyFrom(pkt.end());
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(firePkt);
	Broadcast(sendBuffer, pkt.object_id());
}

void Room::HandleDamageEnermy(Protocol::C_DAMAGE_ENERMY pkt)
{
	const uint64 objectId = pkt.object_id();
	if (_objects.find(objectId) == _objects.end())
	{
		return;
	}

	const uint64 targetId = pkt.target_id();
	if (_enermies[targetId]->IsEnermy() == false)
	{
		return;
	}

	EnermyRef enermy =_enermies[targetId];
	enermy->enermyState = Protocol::ENERMY_STATE_DAMAGE;
	enermy->hp -= pkt.damage();

	if (enermy->hp <= 0)
	{
		enermy->startTime = ::GetTickCount64();
		enermy->enermyState = Protocol::ENERMY_STATE_DIE;
	}
}

void Room::UpdateTick()
{
	//std::cout << "Update Room" << endl;

	// TODO: 게임 로직
	// Enermy 로직
	for (std::pair<const uint64, EnermyRef>& item : _enermies)
	{
		float min = FLT_MAX;
		EnermyRef enermy = static_pointer_cast<Enermy>(item.second);

		for (std::pair<const uint64, PlayerRef>& item : _players)
		{
			PlayerRef player = item.second;
			float distance = VectorUtils::Distance(player->position, enermy->position);
			if (min > distance)
			{
				min = distance;
				enermy->targetPlayer = player;
			}
		}

		if (enermy->targetPlayer.lock())
		{
			switch (enermy->enermyState)
			{
			case Protocol::ENERMY_STATE_IDLE:
				enermy->IdleState();
				break;
			case Protocol::ENERMY_STATE_MOVE:
				enermy->MoveState();
				break;
			case Protocol::ENERMY_STATE_ATTACK:
				enermy->AttackState();
				break;
			case Protocol::ENERMY_STATE_DAMAGE:
				enermy->DamageState();
				break;
			case Protocol::ENERMY_STATE_DIE:
				enermy->DieState();
				break;
			}
		}

		Protocol::S_ENERMY_INFO enermyPacket;
		enermyPacket.mutable_objectinfo()->CopyFrom(enermy->changeToPacket());
		if (enermy->targetPlayer.lock())
		{
			enermyPacket.set_target_id(enermy->targetPlayer.lock()->objectId);
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(enermyPacket);
		Broadcast(sendBuffer);
	}

	DoTimer(100, &Room::UpdateTick);
}

RoomRef Room::GetRoomRef()
{
	return static_pointer_cast<Room>(shared_from_this());
}

bool Room::AddObject(ObjectRef object)
{
	if (_objects.find(object->objectId) != _objects.end())
	{
		return false;
	}

	_objects.insert(make_pair(object->objectId, object));
	object->room.store(GetRoomRef());

	if (object->IsPlayer())
	{
		_players.insert(make_pair(object->objectId, static_pointer_cast<Player>(object)));
	}

	if (object->IsEnermy())
	{
		_enermies.insert(make_pair(object->objectId, static_pointer_cast<Enermy>(object)));
	}

	return true;
}

bool Room::RemoveObject(uint64 objectId)
{
	if (_objects.find(objectId) == _objects.end())
	{
		return false;
	}
	
	ObjectRef object = _objects[objectId];
	if (object->IsPlayer())
	{
		PlayerRef player = static_pointer_cast<Player>(object);
		object->room.store(weak_ptr<Room>());
	}

	_objects.erase(objectId);

	if (object->IsPlayer())
	{
		_players.erase(objectId);
	}

	if (object->IsEnermy())
	{
		_enermies.erase(objectId);
	}

	return true;
}

void Room::Broadcast(SendBufferRef sendBuffer, uint64 exceptId)
{
	for (std::pair<const uint64, ObjectRef>& item : _objects)
	{
		PlayerRef player = dynamic_pointer_cast<Player>(item.second);	// dynamic_cast 속도 느림
		if (player == nullptr)
		{
			continue;
		}

		if (player->objectId == exceptId)
		{
			continue;
		}

		if (GameSessionRef session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}
}