#pragma once
#include "Session.h"


class Player;

class GameSession : public PacketSession
{
public:
	virtual ~GameSession() override
	{
		cout << "~GameSession" << endl;
		_player.store(shared_ptr<Player>());
	}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

public:
	atomic<shared_ptr<Player>> _player;
};