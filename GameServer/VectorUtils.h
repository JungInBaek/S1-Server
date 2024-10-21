#pragma once
#include "S1Vector.h"


class VectorUtils
{
public:
	static S1Vector Add(const S1Vector& vector1, const S1Vector& vector2);
	static S1Vector Sub(const S1Vector& vector1, const S1Vector& vector2);
	static float Size(const S1Vector& vector);
	static float Distance(const S1Vector& dest, const S1Vector& start);
	static void Normalize(S1Vector& vector);
	static S1Vector ScalarMultiplication(const S1Vector& vector, float scalar);
	static S1Vector DownVector();
};

