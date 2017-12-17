#include "Bone.h"
#include "Matrix.h"
#include <GL/glut.h>

#include <btBulletDynamicsCommon.h>

#ifndef __RCEX__
#define __RCEX__

 
// 必要無いものはコメントアウト
#ifdef _DEBUG
#pragma comment (lib, "LinearMath_vs2010_debug.lib")
#pragma comment (lib, "BulletCollision_vs2010_debug.lib")
#pragma comment (lib, "BulletDynamics_vs2010_debug.lib")
#else
#pragma comment (lib, "LinearMath_vs2010.lib")
#pragma comment (lib, "BulletCollision_vs2010.lib")
#pragma comment (lib, "BulletDynamics_vs2010.lib")
#endif

enum RC_CHIP_TYPE {
	RC_CHIP,
	RC_CORE,
	RC_FRAME,
	RC_TRIM,
	RC_RUDDER,
	RC_ARM,
	RC_WHEEL,
	RC_RLW,
	RC_JET,
	RC_WEIGHT,
	RC_COWL0,
	RC_COWL1,
	RC_COWL2,
	RC_COWL3,
	RC_COWL4,
	RC_COWL5,
	RC_TRIMF,
	RC_RUDDERF,
	RC_KINDN
};

enum {
	RX_COL_NOTHING = 0,
	RX_COL_GROUND = 1,
	RX_COL_CHIP = 2,
};

typedef struct __VAL {
	float now, def, max, min, step;
	char name[64];
	int inmove, ind;
} Val;


typedef struct __KEY {
	Val *val[256];
	float step[256];
	int valcnt;
} Key;

typedef struct __CHIP {
	float vx2[3];
	float dm[3];

	int color, option;

	float angle, power, mr;

	Val *angle_val;
	Val *color_val;
	Val *power_val;
	Val *spling_val;
	Val *damper_val;

	int angle_vm, color_vm, power_vm;

	Bone *bone;

	btRigidBody *phy;
	btHingeConstraint *joint, *joint2;

	btVector3 pivot[2];

	int dir;
	RC_CHIP_TYPE type;
	struct __CHIP *parent;
	struct __CHIP *childs[4][16];
	int childcnt[4];
} Chip;

#define RADIAN( n ) ( ( n ) * ( 3.14159 / 180.0 ) )

void ReadModel(char *filename, float *ofs);
void DrawModel(void);

void RegisterModel(void);
#endif
