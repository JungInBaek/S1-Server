#pragma once
#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "ServerPacketHandler.h"


char sendData[] = "Hello World";

class ServerSession : public PacketSession
{
public:
	virtual ~ServerSession() override
	{
		cout << "~ServerSession" << endl;
	}

	virtual void OnConnected() override
	{
		//cout << "Connected to Server" << endl;

		Protocol::C_LOGIN pkt;
		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
        PacketSessionRef session = GetPacketSessionRef();
        PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

        // TODO: 패킷 ID, 대역 체크
        ServerPacketHandler::HandlePacket(session, buffer, len);
	}

	virtual void OnSend(int32 len) override
	{
		//cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		//cout << "Disconnected" << endl;
	}
};

int main()
{
    ServerPacketHandler::Init();

	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(NetAddress(L"127.0.0.1", 7777), MakeShared<IocpCore>(), MakeShared<ServerSession>, 1000);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch(
			[=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			}
		);
	}

	Protocol::C_CHAT chatPkt;
	chatPkt.set_msg(u8"Hello World ! ");
	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(chatPkt);

	while (true)
	{
		service->Broadcast(sendBuffer);
		this_thread::sleep_for(1s);
	}

	GThreadManager->Join();
}