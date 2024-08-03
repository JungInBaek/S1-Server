#include "pch.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "Room.h"
#include "GameSession.h"


PacketHandlerFunc GPacketHandler[UINT16_MAX];


bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO: Log

	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// TODO: Validation

	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	// TODO: DB 플레이어 정보 조회
	// TODO: GameSession에 플레이어 정보 저장 (메모리)

	// ID 발급 (DB ID가 아닌, 인게임 ID)
	static Atomic<uint64> idGenerator = 1;

	{
		Protocol::Player* player = loginPkt.add_players();
		//player->set_id(idGenerator);
		player->set_name(u8"DB에서 긁어온 이름1");
		player->set_playertype(Protocol::PLAYER_TYPE_KNIGHT);

		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->type = player->playertype();
		playerRef->ownerSession = gameSession;
		
		gameSession->_players.push_back(playerRef);
	}

	{
		Protocol::Player* player = loginPkt.add_players();
		//player->set_id(idGenerator);
		player->set_name(u8"DB에서 긁어온 이름2");
		player->set_playertype(Protocol::PLAYER_TYPE_MAGE);

		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->type = player->playertype();
		playerRef->ownerSession = gameSession;

		gameSession->_players.push_back(playerRef);
	}

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	uint64 index = pkt.playerindex();

	// TODO: Validation

	gameSession->_currentPlayer = gameSession->_players[index];
	gameSession->_room = GRoom;
	GRoom->DoAsync(&Room::Enter, gameSession->_currentPlayer);

	Protocol::S_ENTER_GAME enterGamePkt;
	enterGamePkt.set_success(true);
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
	gameSession->_currentPlayer->ownerSession->Send(sendBuffer);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	std::cout << pkt.msg() << endl;

	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(pkt.msg());

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt);
	GRoom->DoAsync(&Room::Broadcast, sendBuffer);

	return true;
}
