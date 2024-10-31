#pragma once
#include "JobQueue.h"
#include "S1Vector.h"

class PathFinder
{
public:
	Vector<S1Vector> AStar(const S1Vector& start, const S1Vector& goal);
	void ReadFile();

	float GetApproximationValue(int value);

private:
	S1Vector parseToS1Vector(const string& str);
	int32 parseToInt32(const string& str);

public:
	Map<S1Vector, int32> nodeMap;
	HashMap<S1Vector, S1NodeRef> allNodes;
	bool IsRead = false;
};