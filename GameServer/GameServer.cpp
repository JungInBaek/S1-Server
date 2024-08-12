#pragma once
#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include "Protocol.pb.h"
#include "Room.h"
#include "DBConnectionPool.h"
#include "DBBind.h"
#include "XmlParser.h"
#include "DBSynchronizer.h"
#include "GenProcedures.h"
#include "Protocol.pb.h"


enum
{
	WORKER_TICK = 64
};

void DoWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;
		service->GetIocpCore()->Dispatch(10);

		// 예약된 JobQueue
		ThreadManager::DistributeReservedJobs();

		// 글로벌 JobQueue
		ThreadManager::DoGlobalQueueWork();
	}
}

int main()
{
	ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={ODBC Driver 17 for SQL Server};Server=(localdb)\\ProjectModels;Database=serverDB;Trusted_Connection=Yes;"));

	DBConnection* dbConn = GDBConnectionPool->Pop();
	DBSynchronizer dbSync(*dbConn);
	dbSync.Synchronize(L"GameDB.xml");

	ClientPacketHandler::Init();

	ServerServiceRef service = MakeShared<ServerService>(NetAddress(L"127.0.0.1", 7777), MakeShared<IocpCore>(), MakeShared<GameSession>, 100);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch(
			[&service]()
			{
				DoWorkerJob(service);
			}
		);
	}

	// Main Thread
	//DoWorkerJob(service);

	while (true)
	{
		//Protocol::S_CHAT pkt;
		//pkt.set_msg("HelloWorld");
		//auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);

		//GSessionManager.Broadcast(sendBuffer);
		this_thread::sleep_for(1s);
	}

	GThreadManager->Join();
}