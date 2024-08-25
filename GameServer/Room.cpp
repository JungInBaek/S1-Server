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

bool Room::HandleEnterPlayer(PlayerRef player)
{
	bool success = AddObject(player);

	// 랜덤 위치
	player->posInfo->set_x(Utils::GetRandom(0.f, 500.f));
	player->posInfo->set_y(Utils::GetRandom(0.f, 500.f));
	player->posInfo->set_z(100.f);
	player->posInfo->set_yaw(Utils::GetRandom(0.f, 100.f));

	// 접속 플레이어에게 입장 패킷 전송
	{
		Protocol::S_ENTER_GAME enterGamePkt;
		enterGamePkt.set_success(success);

		Protocol::ObjectInfo* playerInfo = new Protocol::ObjectInfo();
		playerInfo->CopyFrom(*player->objectInfo);
		enterGamePkt.set_allocated_player(playerInfo);
		//enterGamePkt.release_player();

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
		if (auto session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}

	// 접속 플레이어에게 기존 플레이어들의 스폰 패킷 전송
	{
		Protocol::S_SPAWN spawnPkt;

		for (std::pair<const uint64, ObjectRef>& item : _objects)
		{
			// TODO: Player만 추출 함수 추가
			if (item.second->IsPlayer() == false)
			{
				continue;
			}

			// 클라이언트에서 자기 자신의 스폰 패킷 예외 처리
			Protocol::ObjectInfo* playerInfo = spawnPkt.add_players();
			playerInfo->CopyFrom(*item.second->objectInfo);
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		if (auto session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}

	// 기존에 입장한 플레이어들에게 새로운 플레이어의 스폰 패킷 전송
	{
		Protocol::S_SPAWN spawnPkt;

		Protocol::ObjectInfo* playerInfo = spawnPkt.add_players();
		playerInfo->CopyFrom(*player->objectInfo);

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		Broadcast(sendBuffer, player->objectInfo->object_id());
	}

	return success;
}

bool Room::HandleLeavePlayer(PlayerRef player)
{
	if (player == nullptr)
	{
		return false;
	}

	const uint64 objectId = player->posInfo->object_id();
	bool success = RemoveObject(objectId);

	// 퇴장 플레이어에게 퇴장 패킷 전송
	{
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
		if (GameSessionRef session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}

	return success;
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
	PlayerRef player = dynamic_pointer_cast<Player>(_objects[objectId]);
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