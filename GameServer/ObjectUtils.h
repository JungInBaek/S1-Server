#pragma once


class ObjectUtils
{
public:
	static PlayerRef CreatePlayer(GameSessionRef session);
	static EnermyRef CreateEnermy();

private:
	static atomic<int64> s_idGenerator;
};

