#ifndef VEC3F_H
#define VEC3F_H

ws::Vec3f lerpVec3(const ws::Vec3f& a, const ws::Vec3f& b, float t)
{
	return {
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t
	};
};

void normalize(ws::Vec3f &v)
{
	double length = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	if(length > 0)
	{
		v.x /= length;
		v.y /= length;
		v.z /= length;
	}
}

float getDistance(ws::Vec3f &v)
{
	return std::sqrt(v.x * v.x + v.y*v.y + v.z*v.z);
}

ws::Vec3f operator/(const ws::Vec3f& v, float s)
{	
    return { v.x / s, v.y / s, v.z / s };
}

// Compound assignment (member or non‑member)
ws::Vec3f& operator/=(ws::Vec3f& v, float s)
{
    v.x /= s;
    v.y /= s;
    v.z /= s;
    return v;
}

ws::Vec3f operator/(const ws::Vec3f& a, const ws::Vec3f& b)
{
   return { a.x / b.x, a.y / b.y, a.z / b.z };
}

ws::Vec3f& operator/=(ws::Vec3f& a, const ws::Vec3f& b)
{
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
    return a;
}

ws::Vec3f operator-(const ws::Vec3f& a, const ws::Vec3f& b)
{
   return { a.x - b.x, a.y - b.y, a.z - b.z };
}


ws::Vec3f operator*(float s,const ws::Vec3f& v)
{	
    return { v.x * s, v.y * s, v.z * s };
}

#endif