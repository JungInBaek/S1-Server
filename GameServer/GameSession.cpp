#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Room.h"


void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));

	if (_currentPlayer == nullptr)
	{
		return;
	}

	auto room = _room.lock();

	if (room == nullptr)
	{
		return;
	}

	room->DoAsync(&Room::Leave, _currentPlayer);

	_currentPlayer = nullptr;
	_players.clear();
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO: 패킷 ID, 대역 체크
    ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{

}