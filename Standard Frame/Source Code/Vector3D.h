#pragma once

struct CTriangle	// contains index
{
	int Vertex[3];
	int Normal[3];
	int TexCoord[3];
	CTriangle(){}
};

class CTexCoord
{
public:
	float u, v;
	CTexCoord(float s = 0.0f, float t = 0.0f): u(s), v(t)
	{
	}
};

class Vector
{
public:
	//__declspec(align(16))
	float x;
	float y, z, w;
	Vector();
	Vector(float _x, float _y);
	Vector(float _x, float _y, float _z);
	Vector(float *p);
	Vector(const Vector &v);
	bool operator ==(const Vector &v) const;
	bool operator !=(const Vector &v) const;
	Vector operator +() const;
	Vector operator +(const Vector& v) const;
	Vector operator -() const;
	Vector operator -(const Vector& v) const;
	Vector operator *(float f) const;
	Vector operator /(float f) const;
	Vector &operator =(const Vector &v);
	Vector &operator -=(const Vector &v);
	Vector &operator +=(const Vector &v);
	Vector &operator *=(float f);
	Vector &operator /=(float f);
	operator float *() const;
	operator const float *() const;
	float Length() const;
	float SquaredLength() const;
	Vector Normalize() const;
	float Dot(const Vector &v) const;
	Vector Cross(const Vector &v) const;

	Vector RotateFromPoint_Y_POLE(const Vector &v, float Angle);
	Vector RotateFromPoint_X_POLE(const Vector &v, float Angle);
	Vector RotateFromPoint_Z_POLE(const Vector &v, float Angle);

	float AngleBetween_XZ(const Vector &v);
	float AngleBetween_XY(const Vector &v);
	float AngleBetween_ZY(const Vector &v);
	float AngleBetween_XYZ(const Vector &v);

	static void Register(lua_State * State);
};

typedef Vector CVertex, CNormal;

Vector operator *(float f, const Vector &v);

#define SQRT_MAGIC_F 0x5f3759df
#define SQRT_MAGIC_D 0x5fe6ec85e7de30da

inline float FastSqrt_5(const float x)  
{
	union {int i;float x;}u;
	u.x = x;
	u.i = (1<<29) + (u.i >> 1) - (1<<22); 
	return u.x;
}

inline float FastSqrt_2(const float x)
{
	const float xhalf = 0.5f*x;
	union 
	{float x;int i;} u;// get bits for floating value
	u.x = x;
	u.i = SQRT_MAGIC_F - (u.i >> 1);  // gives initial guess y0
	return (u.x*(1.5f - xhalf*u.x*u.x)) * x;// Newton step, repeating increases accuracy
}