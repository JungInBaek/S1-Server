#pragma once
#include "JobQueue.h"
#include "S1Vector.h"

class PathFinder : public JobQueue
{
public:
	Vector<S1Vector> AStar(S1Vector& start, S1Vector& goal, Map<S1Vector, Vector<S1Vector>>& edges);
	void ReadFile();

private:
	S1Vector parseToS1Vector(const string& str);

public:
	Map<S1Vector, Vector<S1Vector>> EdgeMap;
	bool IsRead = false;
};

extern PathFinder GPathFinder;