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



#endif