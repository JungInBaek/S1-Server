#include "pch.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "Room.h"
#include "ObjectUtils.h"


PacketHandlerFunc GPacketHandler[UINT16_MAX];


bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO: Log

	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	// TODO: DB에서 유저 정보

	Protocol::S_LOGIN loginPkt;
	
	for (int i = 0; i < 3; i++)
	{
		Protocol::ObjectInfo* player = loginPkt.add_players();
		Protocol::PosInfo* posInfo = player->mutable_pos_info();
		posInfo->set_yaw(Utils::GetRandom(0.f, 45.f));
		Protocol::VectorInfo* vectorInfo = posInfo->mutable_vector_info();
		vectorInfo->set_x(Utils::GetRandom(0.f, 100.f));
		vectorInfo->set_y(Utils::GetRandom(0.f, 100.f));
		vectorInfo->set_z(Utils::GetRandom(0.f, 100.f));
	}

	loginPkt.set_success(true);

	SEND_PACKET(loginPkt);

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	// Player 생성
	PlayerRef player = ObjectUtils::CreatePlayer(static_pointer_cast<GameSession>(session), Protocol::PLAYER_TYPE_SURVIVOR, 100);

	// Room 입장
	GRoom->DoAsync(&Room::HandleEnterPlayer, player);

	return true;
}

bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	
	PlayerRef player = gameSession->_player.load();
	if (player == nullptr)
	{
		return false;
	}

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
	{
		return false;
	}

	GRoom->DoAsync(&Room::HandleLeavePlayer, player);

	return true;
}

bool Handle_C_TURN(PacketSessionRef& session, Protocol::C_TURN& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->_player.load();
	if (player == nullptr)
	{
		return false;
	}

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
	{
		return false;
	}

	room->DoAsync(&Room::HandleTurn, pkt);

	return true;
}

bool Handle_C_JUMP(PacketSessionRef& session, Protocol::C_JUMP& pkt)
{
	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->_player.load();
	if (player == nullptr)
	{
		return false;
	}

	// TODO: objectId Validate

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
	{
		return false;
	}

	room->DoAsync(&Room::HandleMove, pkt);

	return true;
}

bool Handle_C_STATE(PacketSessionRef& session, Protocol::C_STATE& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	
	GRoom->DoAsync(&Room::HandleState, pkt);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	return true;
}

bool Handle_C_FIRE(PacketSessionRef& session, Protocol::C_FIRE& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->_player.load();
	if (player == nullptr)
	{
		return false;
	}

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
	{
		return false;
	}

	room->DoAsync(&Room::HandleFire, player);

	return true;
}

bool Handle_C_SNIPER_FIRE(PacketSessionRef& session, Protocol::C_SNIPER_FIRE& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->_player.load();
	if (player == nullptr)
	{
		return false;
	}

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
	{
		return false;
	}

	room->DoAsync(&Room::HandleSniperFire, pkt);

	return true;
}

bool Handle_C_CHANGE_ITEM(PacketSessionRef& session, Protocol::C_CHANGE_ITEM& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->_player.load();
	if (player == nullptr)
	{
		return false;
	}

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
	{
		return false;
	}

	room->DoAsync(&Room::HandleChangeItem, pkt);

	return true;
}
