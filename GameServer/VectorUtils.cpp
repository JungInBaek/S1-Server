#include "pch.h"
#include "VectorUtils.h"
#include "math.h"


S1Vector VectorUtils::Add(const S1Vector& vector1, const S1Vector& vector2)
{
	S1Vector vector;
	vector.x = vector1.x + vector2.x;
	vector.y = vector1.y + vector2.y;
	vector.z = vector1.z + vector2.z;

	return vector;
}

S1Vector VectorUtils::Sub(const S1Vector& vector1, const S1Vector& vector2)
{
	S1Vector vector;
	vector.x = vector1.x - vector2.x;
	vector.y = vector1.y - vector2.y;
	vector.z = vector1.z - vector2.z;

	return vector;
}

float VectorUtils::Size(const S1Vector& vector)
{
	return sqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
}

float VectorUtils::Distance(const S1Vector& dest, const S1Vector& start)
{
	S1Vector dir = Sub(dest, start);
	float distance = Size(dir);
	return distance;
}

void VectorUtils::Normalize(S1Vector& vector)
{
	float resultX = sqrt(vector.x * vector.x);
	if (_isnan(resultX) == 0 && vector.x != 0)
	{
		vector.x = vector.x / resultX;
	}
	float resultY = sqrt(vector.y * vector.y);
	if (_isnan(resultY) == 0 && vector.y != 0)
	{
		vector.y = vector.y / resultY;
	}
	float resultZ = sqrt(vector.z * vector.z);
	if (_isnan(resultZ) == 0 && vector.z != 0)
	{
		vector.z = vector.z / resultZ;
	}
}

S1Vector VectorUtils::ScalarMultiplication(const S1Vector& vector, float scalar)
{
	S1Vector result;
	result.x = vector.x * scalar;
	result.y = vector.y * scalar;
	result.z = vector.z * scalar;
	return result;
}

S1Vector VectorUtils::DownVector()
{
	S1Vector downVector;
	downVector.x = 0;
	downVector.y = 0;
	downVector.z = -1;

	return downVector;
}
