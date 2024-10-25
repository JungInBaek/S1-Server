#pragma once


class S1Vector
{
public:
	S1Vector();
	S1Vector(float x, float y, float z);
	~S1Vector();

public:
	friend std::hash<S1Vector>;

	bool operator==(const S1Vector& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	bool operator<(const S1Vector& other) const
	{
		if (x != other.x)
		{
			return x < other.x;
		}
		else if (y != other.y)
		{
			return y < other.y;
		}
		else if (z != other.z)
		{
			return z < other.z;
		}
		else
		{
			return false;
		}
	}

	S1Vector operator*(float scalar) const
	{
		return S1Vector(x * scalar, y * scalar, z * scalar);
	}

	S1Vector operator+(const S1Vector& other) const
	{
		return S1Vector(x + other.x, y + other.y, z + other.z);
	}

	S1Vector operator-(const S1Vector& other) const
	{
		return S1Vector(x - other.x, y - other.y, z - other.z);
	}

	S1Vector operator*(const S1Vector& other) const
	{
		return S1Vector(x * other.x, y * other.y, z * other.z);
	}

	S1Vector operator/(const S1Vector& other) const
	{
		return S1Vector(x / other.x, y / other.y, z / other.z);
	}

	S1Vector Normalize()
	{
		float resultX = sqrt(x * x);
		if (_isnan(resultX) == 0 && x != 0)
		{
			x = x / resultX;
		}
		float resultY = sqrt(y * y);
		if (_isnan(resultY) == 0 && y != 0)
		{
			y = y / resultY;
		}
		float resultZ = sqrt(z * z);
		if (_isnan(resultZ) == 0 && z != 0)
		{
			z = z / resultZ;
		}
	}

public:
	float x;
	float y;
	float z;
};

namespace std
{
	template<>
	class hash<S1Vector>
	{
	public:
		size_t operator()(const S1Vector& vector) const
		{
			using std::hash;
			return hash<float>()(vector.x) ^ hash<float>()(vector.y) + (hash<float>()(vector.z) << 1);
		}
	};
}