#ifndef BONE_H
#define BONE_H

#include <string.h>

class Bone {
	float position[4];
	float rotation[4];
	float animation[16];
	float length;
	const Bone *parent;
	void copy(const Bone &b);
	void identity();
public:
	Bone() {}
	Bone(const float *p, const float *r, float l = 1.0f, const Bone *u = 0);
	Bone(float x, float y, float z, float w = 1.0f,
			 float r = 0.0f, float s = 0.0f, float t = 1.0f, float a = 0.0f,
			 float l = 1.0f, const Bone *u = 0);
	Bone(const Bone &b) { copy(b); }
	virtual ~Bone() {}
	Bone &operator=(const Bone &b) { copy(b); return *this; }
	Bone &setPosition(float x, float y, float z, float w = 1.0f);
	Bone &setPosition(const float *p) { return setPosition(p[0], p[1], p[2], p[3]); }
	Bone &setRotation(float x, float y, float z, float a);
	Bone &setRotation(const float *r) { return setRotation(r[0], r[1], r[2], r[3]); }
	Bone &setAnimation(const float *a) { memcpy(animation, a, sizeof animation); return *this; }
	Bone &setLength(float l) { length = l; return *this; }
	Bone &setParent(const Bone *p) { parent = p; return *this; }
	const float *getPosition() const { return position; }
	const float *getRotation() const { return rotation; }
	const float *getAnimation() const { return animation; }
	float getLength() const { return length; }
	const Bone *getParent() const { return parent; }
};

#endif
