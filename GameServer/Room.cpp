#include "pch.h"
#include "Room.h"
#include "Player.h"


RoomRef GRoom = make_shared<Room>();


//void Room::Enter(PlayerRef player)
//{
//	_players[player->playerId] = player;
//}
//
//void Room::Leave(PlayerRef player)
//{
//	_players.erase(player->playerId);
//}
//
//void Room::Broadcast(SendBufferRef sendBuffer)
//{
//	for (auto& p : _players)
//	{
//		p.second->ownerSession->Send(sendBuffer);
//	}
//}


Room::Room()
{
}

Room::~Room()
{
}

bool Room::HandleEnterPlayerLocked(PlayerRef player)
{
	WRITE_LOCK;
	
	bool success = EnterPlayer(player);

	// 랜덤 위치
	player->playerInfo->set_x(Utils::GetRandom(0.f, 500.f));
	player->playerInfo->set_y(Utils::GetRandom(0.f, 500.f));
	player->playerInfo->set_z(100.f);
	player->playerInfo->set_yaw(Utils::GetRandom(0.f, 100.f));

	// 접속 플레이어에게 입장 패킷 전송
	{
		Protocol::S_ENTER_GAME enterGamePkt;
		enterGamePkt.set_success(success);

		Protocol::PlayerInfo* playerInfo = new Protocol::PlayerInfo();
		playerInfo->CopyFrom(*player->playerInfo);
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

		for (std::pair<const uint64, PlayerRef>& item : _players)
		{
			// 클라이언트에서 자기 자신의 스폰 패킷 예외 처리
			Protocol::PlayerInfo* playerInfo = spawnPkt.add_players();
			playerInfo->CopyFrom(*item.second->playerInfo);
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

		Protocol::PlayerInfo* playerInfo = spawnPkt.add_players();
		playerInfo->CopyFrom(*player->playerInfo);

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		Broadcast(sendBuffer, player->playerInfo->object_id());
	}

	return success;
}

bool Room::HandleLeavePlayerLocked(PlayerRef player)
{
	if (player == nullptr)
	{
		return false;
	}

	WRITE_LOCK;

	const uint64 objectId = player->playerInfo->object_id();
	bool success = LeavePlayer(objectId);

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

void Room::HandleMoveLocked(Protocol::C_MOVE pkt)
{
	WRITE_LOCK;

	const uint64 objectId = pkt.info().object_id();
	if (_players.find(objectId) == _players.end())
	{
		return;
	}

	// TODO: Move Packet Validate

	// 적용
	PlayerRef& player = _players[objectId];
	player->playerInfo->CopyFrom(pkt.info());

	// 이동
	{
		Protocol::S_MOVE movePkt;

		Protocol::PlayerInfo* info = movePkt.mutable_info();
		info->CopyFrom(pkt.info());

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
		Broadcast(sendBuffer);
	}
}

bool Room::EnterPlayer(PlayerRef player)
{
	if (_players.find(player->playerInfo->object_id()) != _players.end())
	{
		return false;
	}

	_players.insert(make_pair(player->playerInfo->object_id(), player));
	player->room.store(static_pointer_cast<Room>(shared_from_this()));

	return true;
}

bool Room::LeavePlayer(uint64 objectId)
{
	if (_players.find(objectId) == _players.end())
	{
		return false;
	}
	
	PlayerRef player = _players[objectId];
	player->room.store(weak_ptr<Room>());

	_players.erase(objectId);

	return true;
}

void Room::Broadcast(SendBufferRef sendBuffer, uint64 exceptId)
{
	for (std::pair<const uint64, PlayerRef>& item : _players)
	{
		PlayerRef player = item.second;
		if (player->playerInfo->object_id() == exceptId)
		{
			continue;
		}

		if (GameSessionRef session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}
}