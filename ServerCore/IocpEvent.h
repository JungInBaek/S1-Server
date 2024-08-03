#pragma once


class Session;

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	//PreRecv,
	Recv,
	Send,
};

// 가상함수 사용 주의: 0번 offset이 OVERLAPPED여야 하는데 가상함수 테이블이 채워짐. 단, 상속일 경우 0번 offset은 OVERLAPPED로 유지됨
/*----------------------
		IocpEvent
----------------------*/

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);

	void Init();
	
public:
	EventType eventType;
	IocpObjectRef owner;
};


/*-------------------------
		ConnectEvent
-------------------------*/

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) {}
};


/*-------------------------
		DisconnectEvent
-------------------------*/

class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) {}
};


/*-------------------------
		AcceptEvent
-------------------------*/

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}

public:
	SessionRef session = nullptr;
};


/*-------------------------
		RecvEvent
-------------------------*/

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
};


/*-------------------------
		SendEvent
-------------------------*/

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}

	Vector<SendBufferRef> sendBuffers;
};