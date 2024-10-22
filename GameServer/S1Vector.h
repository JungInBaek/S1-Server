#pragma once

class S1Vector
{
public:
	S1Vector();
	S1Vector(float x, float y, float z);
	~S1Vector();

public:
	S1Vector operator*(float scalar) const
	{
		return S1Vector(x * scalar, y * scalar, z * scalar);
	}

	S1Vector operator+(const S1Vector& v) const
	{
		return S1Vector(x + v.x, y + v.y, z + v.z);
	}

	S1Vector operator-(const S1Vector& v) const
	{
		return S1Vector(x - v.x, y - v.y, z - v.z);
	}

	S1Vector operator*(const S1Vector& v) const
	{
		return S1Vector(x * v.x, y * v.y, z * v.z);
	}

	S1Vector operator/(const S1Vector& v) const
	{
		return S1Vector(x / v.x, y / v.y, z / v.z);
	}

	S1Vector Normalize()
	{

	}

public:
	float x;
	float y;
	float z;
};

