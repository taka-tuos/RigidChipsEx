#include <string.h>
#if defined(WIN32)
#	 include <Windows.h>
#	 include <GL/gl.h>
#elif defined(__APPLE__) || defined(MACOSX)
#	 include <OpenGL/gl.h>
#else
#	 define GL_GLEXT_PROTOTYPES
#	 include <GL/gl.h>
#endif

#include "Bone.h"

void Bone::copy(const Bone &b)
{
	memcpy(position, b.position, sizeof position);
	memcpy(rotation, b.rotation, sizeof rotation);
	memcpy(animation, b.animation, sizeof animation);
	length = b.length;
	parent = b.parent;
}

void Bone::identity()
{
	animation[ 1] = animation[ 2] = animation[ 3] = animation[ 4] =
	animation[ 6] = animation[ 7] = animation[ 8] = animation[ 9] =
	animation[11] = animation[12] = animation[13] = animation[14] = 0.0f;
	animation[ 0] = animation[ 5] = animation[10] = animation[15] = 1.0f;
}

Bone::Bone(const float *p, const float *r, float l, const Bone *u)
{
	setPosition(p);
	setRotation(r);
	identity();
	length = l;
	parent = u;
}

Bone::Bone(float x, float y, float z, float w,
					 float r, float s, float t, float a,
					 float l, const Bone *u)
{
	setPosition(x, y, z, w);
	setRotation(r, s, t, a);
	identity();
	length = l;
	parent = u;
}

Bone &Bone::setPosition(float x, float y, float z, float w)
{
	position[0] = x;
	position[1] = y;
	position[2] = z;
	position[3] = w;

	return *this;
}

Bone &Bone::setRotation(float x, float y, float z, float a)
{
	rotation[0] = x;
	rotation[1] = y;
	rotation[2] = z;
	rotation[3] = a;
	
	return *this;
}
