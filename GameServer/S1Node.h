#pragma once
#include "S1Vector.h"


class S1Node
{
public:
	S1Node();
	S1Node(const S1Vector& position, float GCost, float HCost);
	~S1Node();

	bool operator>(const S1Node& other) const
	{
		return FCost() > other.FCost();
	}

	bool operator<(const S1Node& other) const
	{
		return FCost() < other.FCost();
	}

	float FCost() const { return GCost + HCost; }

public:
	S1Vector position;
	float GCost;
	float HCost;
};

