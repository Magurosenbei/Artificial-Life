#include "stdafx.h"
#include "Vector3D.h"

Vector :: Vector()
	: x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

Vector :: Vector(float _x, float _y)
	: x(_x), y(_y), z(0.0f), w(1.0f)
{
}

Vector :: Vector(float _x, float _y, float _z)
	: x(_x), y(_y), z(_z), w(1.0f)
{
}

Vector :: Vector(float *p)
	: x(p[0]), y(p[1]), z(p[2]), w(1.0f)
{
}

Vector :: Vector(const Vector &v)
	: x(v.x), y(v.y), z(v.z), w(1.0f)
{
}

bool Vector :: operator ==(const Vector &v) const
{
	return x == v.x && y == v.y && z == v.z;
}

bool Vector :: operator !=(const Vector &v) const
{
	return x != v.x || y != v.y || z != v.z;
}

Vector Vector :: operator +() const
{
	return Vector(x, y, z);
}

Vector Vector :: operator +(const Vector& v) const
{
	return Vector(x + v.x, y + v.y, z + v.z);
}

Vector Vector :: operator -() const
{
	return Vector(-x, -y, -z);
}

Vector Vector :: operator -(const Vector& v) const
{
	return Vector(x - v.x, y - v.y, z - v.z);
}

Vector Vector :: operator *(float f) const
{
	return Vector(x * f, y * f, z * f);
}

Vector Vector :: operator /(float f) const
{
	return Vector(x / f, y / f, z / f);
}

Vector& Vector :: operator =(const Vector &v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

Vector& Vector :: operator -=(const Vector &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector& Vector :: operator +=(const Vector &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector& Vector :: operator *=(float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

Vector& Vector :: operator /=(float f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

Vector :: operator float *() const
{
	return (float *)&x;
}

Vector :: operator const float *() const
{
	return &x;
}

float Vector :: Length() const
{
	return sqrt(x * x + y * y + z * z);
}

float Vector :: SquaredLength() const
{
	return (x * x + y * y + z * z);
}

Vector Vector :: Normalize() const
{
	float f = x * x + y * y + z * z;
	if(!f)	return Vector(0.0f, 0.0f, 0.0f);
		
	f = 1.0f / sqrtf(f);
	return Vector(x * f, y * f, z * f);
}

float Vector :: Dot(const Vector &v) const
{
	return x * v.x + y * v.y + z * v.z;
}

Vector Vector :: Cross(const Vector &v) const
{
	return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

Vector operator *(float f, const Vector &v)
{
	return Vector(v.x * f, v.y * f, v.z * f);
}

Vector Vector :: RotateFromPoint_Y_POLE(const Vector &v, float Angle)
{
	Vector nv = Vector(*this - v);

	return (Vector(	nv.z * sin(Angle) + nv.x * cos(Angle), 
					y, 
					nv.z * cos(Angle) - nv.x * sin(Angle)) + v);
	
}

Vector Vector :: RotateFromPoint_X_POLE(const Vector &v, float Angle)
{
	Vector nv = Vector(*this - v);

	return (Vector(	nv.x,
					nv.y * sin(Angle) + nv.z * cos(Angle), 
					nv.y * cos(Angle) - nv.z * sin(Angle)) + v);
}

Vector Vector :: RotateFromPoint_Z_POLE(const Vector &v, float Angle)
{
	Vector nv = Vector(*this - v);

	return (Vector(	nv.y * sin(Angle) + nv.x * cos(Angle), 
					nv.y * cos(Angle) - nv.x * sin(Angle), 
					nv.z) + v);
}

float Vector :: AngleBetween_XZ(const Vector &v)
{
	float Angle(atan2(x, z) - atan2(v.x, v.z));
	if(_isnan(Angle))	// of indefinite checl
		return 0;
	if(Angle < 0)
		Angle += 2.0f * PI;
	return Angle;
}

float Vector :: AngleBetween_XY(const Vector &v)
{
	float Angle(atan2(x, y) - atan2(v.x, v.y));
	if(_isnan(Angle))	// of indefinite checl
		return 0;
	if(Angle < 0)
		Angle += 2.0f * PI;
	return Angle;
}

float Vector :: AngleBetween_ZY(const Vector &v)
{
	float Angle(atan2(z, y) - atan2(v.z, v.y));
	if(_isnan(Angle))	// of indefinite checl
		return 0;
	if(Angle < 0)
		Angle += 2.0f * PI;
	return Angle;
}

float Vector :: AngleBetween_XYZ(const Vector &v)
{
	float Angle = atan2(v.Cross(*this).Length(), v.Dot(*this));

	if(_isnan(Angle))	// of indefinite checl
		return 0;
	if(Angle < 0)
		Angle += 2.0f * PI;
	return Angle;
}

void Vector :: Register(lua_State * State)
{
	luabind::module(State) 
	[
		luabind::class_<Vector>("Vector")
		.def(luabind::constructor<>())
		.def(luabind::constructor<float, float>())
		.def(luabind::constructor<float, float, float>())
		.def_readwrite("x", &Vector::x)
		.def_readwrite("y", &Vector::y)
		.def_readwrite("z", &Vector::x)
		.def(luabind::const_self + Vector())
		.def(luabind::const_self - Vector())
		.def(luabind::const_self == Vector())
		.def(luabind::const_self * float())
		.def(luabind::const_self / float())
	];
}