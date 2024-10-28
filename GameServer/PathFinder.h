#pragma once
#include "JobQueue.h"
#include "S1Vector.h"

class PathFinder : public JobQueue
{
public:
	Vector<S1Vector> AStar(const S1Vector& start, const S1Vector& goal/*, Map<S1Vector, Vector<S1Vector>>& edges*/);
	void ReadFile();

	float GetApproximationValue(int value);

private:
	S1Vector parseToS1Vector(const string& str);

public:
	Map<S1Vector, Vector<S1Vector>> edgeMap;
	bool IsRead = false;
};