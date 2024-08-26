#include "pch.h"
#include "Room.h"
#include "Player.h"


RoomRef GRoom = make_shared<Room>();


Room::Room()
{
}

Room::~Room()
{
}

bool Room::EnterRoom(ObjectRef object, bool randPos)
{
	bool success = AddObject(object);

	// 랜덤 위치
	if (randPos)
	{
		object->posInfo->set_x(Utils::GetRandom(0.f, 500.f));
		object->posInfo->set_y(Utils::GetRandom(0.f, 500.f));
		object->posInfo->set_z(100.f);
		object->posInfo->set_yaw(Utils::GetRandom(0.f, 100.f));
	}

	// 접속 플레이어에게 입장 패킷 전송
	if (object->IsPlayer())
	{
		PlayerRef player = static_pointer_cast<Player>(object);

		Protocol::S_ENTER_GAME enterGamePkt;
		enterGamePkt.set_success(success);

		Protocol::ObjectInfo* playerInfo = new Protocol::ObjectInfo();
		playerInfo->CopyFrom(*player->objectInfo);
		enterGamePkt.set_allocated_player(playerInfo);
		//enterGamePkt.release_player();	// Protobuf에서 해제 해주길 원하지 않는다면 사용

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
		for (std::pair<const uint64, ObjectRef>& item : _objects)
		{
			// 자기 자신의 Player(Object)는 클라이언트에서 예외 처리
			Protocol::ObjectInfo* objectInfo = spawnPkt.add_players();
			objectInfo->CopyFrom(*item.second->objectInfo);
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		if (auto session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}

	// 기존에 입장한 플레이어들에게 새로운 Object의 스폰 패킷 전송
	{
		Protocol::S_SPAWN spawnPkt;

		Protocol::ObjectInfo* objectInfo = spawnPkt.add_players();
		objectInfo->CopyFrom(*object->objectInfo);

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		Broadcast(sendBuffer, object->objectInfo->object_id());
	}

	return success;
}

bool Room::LeaveRoom(ObjectRef object)
{
	if (object == nullptr)
	{
		return false;
	}

	const uint64 objectId = object->objectInfo->object_id();
	bool success = RemoveObject(objectId);

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
			}
		}
	}

	return success;
}

bool Room::HandleEnterPlayer(PlayerRef player)
{
	return EnterRoom(player);
}

bool Room::HandleLeavePlayer(PlayerRef player)
{
	return LeaveRoom(player);
}

void Room::HandleMove(Protocol::C_MOVE pkt)
{
	const uint64 objectId = pkt.info().object_id();
	if (_objects.find(objectId) == _objects.end())
	{
		return;
	}

	// TODO: Move Packet Validate

	if (_objects[objectId]->IsPlayer() == false)
	{
		return;
	}

	// 적용
	PlayerRef player = static_pointer_cast<Player>(_objects[objectId]);
	player->posInfo->CopyFrom(pkt.info());

	// 이동
	{
		Protocol::S_MOVE movePkt;

		Protocol::PosInfo* info = movePkt.mutable_info();
		info->CopyFrom(pkt.info());

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
		Broadcast(sendBuffer);
	}
}

void Room::UpdateTick()
{
	cout << "Update Room" << endl;

	// TODO

	DoTimer(100, &Room::UpdateTick);
}

RoomRef Room::GetRoomRef()
{
	return static_pointer_cast<Room>(shared_from_this());
}

bool Room::AddObject(ObjectRef object)
{
	if (_objects.find(object->objectInfo->object_id()) != _objects.end())
	{
		return false;
	}

	_objects.insert(make_pair(object->objectInfo->object_id(), object));
	object->room.store(GetRoomRef());

	return true;
}

bool Room::RemoveObject(uint64 objectId)
{
	if (_objects.find(objectId) == _objects.end())
	{
		return false;
	}
	
	ObjectRef object = _objects[objectId];
	object->room.store(weak_ptr<Room>());

	_objects.erase(objectId);

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

		if (player->objectInfo->object_id() == exceptId)
		{
			continue;
		}

		if (GameSessionRef session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}
}