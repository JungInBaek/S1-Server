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
	player->playerInfo->set_z(Utils::GetRandom(0.f, 500.f));
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

	// 접속 플레이어에게 기존 플레이어들의 소환 패킷 전송
	{
		Protocol::S_SPAWN spawnPkt;

		for (std::pair<const uint64, PlayerRef>& item : _players)
		{
			// 클라이언트에서 자기 자신의 소환 패킷 예외 처리
			Protocol::PlayerInfo* playerInfo = spawnPkt.add_players();
			playerInfo->CopyFrom(*item.second->playerInfo);
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		if (auto session = player->session.lock())
		{
			session->Send(sendBuffer);
		}
	}

	// 기존에 입장한 플레이어들에게 새로운 플레이어의 입장 패킷 전송
	{
		Protocol::S_SPAWN spawnPkt;

		Protocol::PlayerInfo* playerInfo = spawnPkt.add_players();
		playerInfo->CopyFrom(*player->playerInfo);

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		Broadcast(sendBuffer, player->playerInfo->object_id());
	}

	return success;
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