#pragma once


class ObjectUtils
{
public:
	static PlayerRef CreatePlayer(GameSessionRef session, Protocol::PlayerType playerType, int32 hp);
	static EnermyRef CreateEnermy(Protocol::EnermyType enermyType, int32 hp);

private:
	static atomic<int64> s_idGenerator;
};

