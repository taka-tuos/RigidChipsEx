#include "RCEX.h"
#include "Bone.h"
#include "Matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

btDynamicsWorld* g_pDynamicsWorld;

Matrix projectionMatrix;
Matrix viewMatrix;

extern GLuint chip_graphic[RC_KINDN];

Bone *chips_bone[512];
float bonepos[256][4];
int bones;

Chip *Chips;
Val Vals[256];
Key Keys[16];

char *chipop[] = {
	"Chip",
	"Core",
	"Frame",
	"Trim",
	"Rudder",
	"Arm",
	"Wheel",
	"RLW",
	"Jet",
	"Weight",
	"Cowl",
	"Cowl",
	"Cowl",
	"Cowl",
	"Cowl",
	"Cowl",
	"TrimF",
	"RudderF",
};

int r_spz = 512;
float stackz[512];
int r_spx = 512;
float stackx[512];
int r_spc = 512;
int stackc[512];


char *rc_getop(char *s)
{
	char *str = (char *)malloc(512);

	int i;
	char *p = s;

	for(;*p==' '||*p==0x09;p++);
	for(i = 0;*p!='(';p++,i++) {
		str[i]=*p;
	}

	str[i]=0;

	return str;
}

char *rc_getargstr(char *s,int num)
{
	char *str = (char *)malloc(512);

	int i;
	char *p = s;

	for(;*p==' '||*p==0x09;p++);
	for(i = 0;*p!='(';p++,i++);

	p++;

	for(i=0;i<num;i++) {
		for(;*p!=','&&*p!=')';p++);
		p++;
	}

	for(i=0;*p!=','&&*p!=')';p++,i++) {
		str[i]=*p;
	}

	str[i]=0;

	return str;
}

char *rc_getskipspt(char *s)
{
	char *ans = (char *)malloc(256);
	int a, b;

	for(a = 0, b = 0; s[b] != 0;) {
		if(s[b] != ' ' && s[b] != '\t') {
			ans[a] = s[b];
			a++;
		}
		b++;
	}
	ans[a] = 0;

	return ans;
}

RC_CHIP_TYPE rc_getchiptype(char *s)
{
	int i;
	for(i = 0; i < RC_KINDN; i++) {
		if(strcmp(s, chipop[i]) == 0) {
			break;
		}
	}
	return (RC_CHIP_TYPE)i;
}

void FreeRotate( float n[3], float r )
{
    float v[16];
    float w =(float)cos( RADIAN( r ) / 2.0f );
    float w2 = w * w;
    float s = (float)sin( RADIAN( r ) / 2.0f );
    float x = n[0] * s;
    float y = n[1] * s;
    float z = n[2] * s;
    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;

    v[0] = w2 + x2 - y2 - z2;
    v[4] = 2 * ( ( x * y ) - ( w * z ) );
    v[8] = 2 * ( ( x * z ) + ( w * y ) );
    v[12] = 0.0f;

    v[1] = 2 * ( ( x * y ) + ( w * z ) );
    v[5] = w2 - x2 + y2 - z2;
    v[9] = 2 * ( ( y * z ) - ( w * x ) );
    v[13] = 0.0f;

    v[2] = 2 * ( ( x * z ) - ( w * y ) );
    v[6] = 2 * ( ( y * z ) + ( w * x ) );
    v[10] = w2 - x2 - y2 + z2;
    v[14] = 0.0f;

    v[3] = 0.0f;
    v[7] = 0.0f;
    v[11] = 0.0f;
    v[15] = 1.0f;

    glMultMatrixf( v );
}

void FreeRotateVertex(float *n, float *ve, float r)
{
	float v[16];
	float w = (float)cos(RADIAN(r) / 2.0f);
	float w2 = w * w;
	float s = (float)sin(RADIAN(r) / 2.0f);
	float x = n[0] * s;
	float y = n[1] * s;
	float z = n[2] * s;
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;

	v[0] = w2 + x2 - y2 - z2;
	v[4] = 2 * ((x * y) - (w * z));
	v[8] = 2 * ((x * z) + (w * y));
	v[12] = 0.0f;

	v[1] = 2 * ((x * y) + (w * z));
	v[5] = w2 - x2 + y2 - z2;
	v[9] = 2 * ((y * z) - (w * x));
	v[13] = 0.0f;

	v[2] = 2 * ((x * z) - (w * y));
	v[6] = 2 * ((y * z) + (w * x));
	v[10] = w2 - x2 - y2 + z2;
	v[14] = 0.0f;

	v[3] = 0.0f;
	v[7] = 0.0f;
	v[11] = 0.0f;
	v[15] = 1.0f;

	Matrix A = Matrix(v);

	A.projection(ve, ve);
}


void rc_rotate3d(float *ox, float *oy, float *oz, float ax, float ay, float az, double angle, int axis)
{
	double cosX, sinX, x, y, z;

	x = ax;
	y = ay;
	z = az;

	sinX = sin(RADIAN(angle));
	cosX = cos(RADIAN(angle));

	*ox = x;
	*oy = y;
	*oz = z;
	
	switch(axis)
	{
		case 0:
			*oy = y * cosX - z * sinX;
			*oz = y * sinX + z * cosX;
			break;
		case 1:
			*ox = x * cosX + z * sinX;
			*oz = z * cosX - x * sinX;
			break;
		case 2:
			*ox = x * cosX - y * sinX;
			*oy = x * sinX + y * cosX;
			break;
	}
}

void rc_pushz(float data)
{
	//cpu->r_sp -= 1;
	stackz[--r_spz] = data;
}

float rc_popz(void)
{
	return stackz[r_spz++];
}

void rc_pushx(float data)
{
	//cpu->r_sp -= 1;
	stackx[--r_spx] = data;
}

float rc_popx(void)
{
	return stackx[r_spx++];
}

void rc_pushc(int data)
{
	//cpu->r_sp -= 1;
	stackc[--r_spc] = data;
}

int rc_popc(void)
{
	return stackc[r_spc++];
}

void rc_fromcolor(unsigned col, int *r, int *g, int *b)
{
	*r = (col >> 16) & 0xff;
	*g = (col >> 8) & 0xff;
	*b = col & 0xff;
}

int rc_numarg(char *s)
{
	int i = 0;
	if(strstr(s,"()") != 0) return 0;
	for(;*s != 0; s++) if(*s == ',') i++;
	return i+1;
}

Val *rc_searchval(char *name)
{
	for (int i = 0; i < 256; i++) {
		if (strcmpi(Vals[i].name, name) == 0) return &Vals[i];
	}
	return NULL;
}

#define DIFF_CHIP 0.6

void RotateVecPrepare(float *vx2, int type, float r2)
{
	if (type == RC_TRIM || type == RC_TRIMF) {
		vx2[0] = 0;
		vx2[1] = 0;
		vx2[2] = 1;
	}
	else if (type == RC_RUDDER || type == RC_RUDDERF) {
		vx2[0] = 0;
		vx2[1] = 1;
		vx2[2] = 0;
	}
	else {
		vx2[0] = 1;
		vx2[1] = 0;
		vx2[2] = 0;
	}

	//float vxR[] = { 0,0,1 };
	//FreeRotateVertex(vxR, vx2, r2);

	rc_rotate3d(&vx2[0], &vx2[1], &vx2[2], vx2[0], vx2[1], vx2[2], r2, 1);

}

void ResetChilds(Chip *now)
{
	for (int i = 0; i < 4; i++) {
		memset(now->childs[i], 0, sizeof(now->childs[i]));
		now->childcnt[i] = 0;
	}
}

void SetDrawMove(Chip *now)
{
	memset(now->dm, 0, sizeof(now->dm));

	if (!now->parent) {
		return;
	}

	int pdir = now->parent->dir;

	float *x, *y, *z;
	float mx = 0, my = 0, mz = 0;
	float mx2 = 0, my2 = 0, mz2 = 0;

	if (pdir == -1) {
		const float *diff = now->bone->getPosition();
		now->dm[0] = diff[0];
		now->dm[1] = diff[1];
		now->dm[2] = diff[2];
		return;
	}

	switch (now->dir) {
	case 0:
		mz = 0.5;
		break;
	case 1:
		mz = -0.5;
		break;
	case 2:
		mx = -0.5;
		//if (pdir == 0) mx2 = -1;
		//if (pdir == 1) mx2 = -1;
		break;
	case 3:
		mx = 0.5;
		//if (pdir == 0) mx2 = 1;
		//if (pdir == 1) mx2 = 1;
		break;
	}

	now->dm[0] = -DIFF_CHIP * mx - DIFF_CHIP * mx2;
	now->dm[1] = -DIFF_CHIP * my - DIFF_CHIP * my2;
	now->dm[2] = -DIFF_CHIP * mz - DIFF_CHIP * mz2;
}

void ReadModel(char *filename, float *ofs)
{
	FILE *fp;
	fp = fopen(filename,"rt");
	bool bodyf = false;
	bool valf = false;
	bool keyf = false;

	Chips = new Chip;

	Chip *now = Chips;

	now->parent = 0;

	int nowp = 0;
	int nowm = -1;

	int nowv = 0;

	memset(Vals, 0, sizeof(Vals));
	memset(Keys, 0, sizeof(Keys));

	do {
		char ln[1024];
		float ma = 1;
		float mb = 1;

		float dx, dy, dz;

		fgets(ln,1024,fp);
		if (keyf == true) {
			if (*ln == '}') {
				keyf = false;
				goto skip_keyf;
			}
			char *a = rc_getskipspt(ln);
			char *n = rc_getskipspt(ln);
			char *np = n;
			while (*n != ':') n++;
			*n = 0;
			int ksel = strtol(np, &np, 0);

			Key *now = &Keys[ksel];
			while (*a != ':') a++;
			a++;
			int ann = rc_numarg(a);
			int an;
			for (an = 0; an < ann; an++) {
				char *tp = rc_getop(a);

				int nn = now->valcnt;

				now->val[nn] = rc_searchval(tp);

				char *ag = rc_getargstr(ln, an);

				if (strncmp(ag, "step=", 5) == 0) {
					char *p = ag + 5;
					if (isdigit(*p) || *p == '-') {
						now->step[nn] = strtod(p, &p);
					}
					else if (*p == '#') {
						p++;
						now->step[nn] = strtol(p, &p, 16);
					}
				}

				now->valcnt++;

				while (an + 1 < ann && *n != ',') n++;
				if (*n == ',') n++;
			}
		skip_keyf:
			;
		}

		if (valf == true) {
			if (*ln == '}') {
				valf = false; goto skip_valf;
			}
			int an;
			int ann = rc_numarg(ln);

			char *tp = rc_getop(ln);

			strcpy(Vals[nowv].name, tp);

			for (an = 0; an < ann; an++) {
				char *ag = rc_getargstr(ln, an);
				while (*ag == ' ' || *ag == '\t') ag++;
				if (strncmp(ag, "default=", 8) == 0) {
					char *p = ag + 8;
					if (isdigit(*p) || *p == '-') {
						Vals[nowv].def = strtod(p, &p);
					}
					else if (*p == '#') {
						p++;
						Vals[nowv].def = strtol(p, &p, 16);
					}
				}
				if (strncmp(ag, "max=", 4) == 0) {
					char *p = ag + 4;
					if (isdigit(*p) || *p == '-') {
						Vals[nowv].max = strtod(p, &p);
					}
					else if (*p == '#') {
						p++;
						Vals[nowv].max = strtol(p, &p, 16);
					}
				}
				if (strncmp(ag, "min=", 4) == 0) {
					char *p = ag + 4;
					if (isdigit(*p) || *p == '-') {
						Vals[nowv].min = strtod(p, &p);
					}
					else if (*p == '#') {
						p++;
						Vals[nowv].min = strtol(p, &p, 16);
					}
				}
				if (strncmp(ag, "step=", 5) == 0) {
					char *p = ag + 5;
					if (isdigit(*p) || *p == '-') {
						Vals[nowv].step = strtod(p, &p);
					}
					else if (*p == '#') {
						p++;
						Vals[nowv].step = strtol(p, &p, 16);
					}
				}
			}
			if (Vals[nowv].def > Vals[nowv].max) Vals[nowv].def = Vals[nowv].max;
			if (Vals[nowv].def < Vals[nowv].min) Vals[nowv].def = Vals[nowv].min;
			Vals[nowv].now = Vals[nowv].def;
			Vals[nowv].ind = nowv;
			nowv++;
		skip_valf:
			;
		}

		if(bodyf == true) {
			if (*ln == '}') {
				bodyf = false;
				goto skip_bodyf;
			}
			char *a = rc_getskipspt(ln);

			if(*a == '}') {
				now = now->parent;
				if (!now) {
					bodyf = false;
					goto skip_bodyf;
				}
				goto skip;
			}
			char *tp = rc_getop(ln);

			int mr = 0;

			float diffmul = now->dir < 0 ? 0.5 : 1.0;

			if(strncmp(tp,"Core",4) == 0) {
				now->bone = new Bone(ofs[0], ofs[1], ofs[2], 1, 0, 0, 0, 0, 0, NULL);
				now->dir = -1;

				now->type = rc_getchiptype(tp);

				ResetChilds(now);

				memset(now->vx2, 0, sizeof(now->vx2));

				mr = 1;
			}
			if(*tp == 'N') {
				rc_pushc((int)now);

				int id = now->childcnt[0];
				now->childcnt[0]++;

				now->childs[0][id] = new Chip;

				now->childs[0][id]->parent = now;

				now->childs[0][id]->bone = new Bone(0, 0, 0, 1, 0, 0, 0, 0, 0, now->bone);
				now = now->childs[0][id];

				now->dir = 0;

				now->type = rc_getchiptype(tp + 2);

				float vx2[3];

				RotateVecPrepare(vx2, now->type, 180);

				mr = 1;

				if (now->type == RC_RUDDER || now->type == RC_RUDDERF) mr = -1;

				ResetChilds(now);

				memcpy(now->vx2, vx2, sizeof(vx2));
			} else if(*tp == 'S') {
				rc_pushc((int)now);

				int id = now->childcnt[1];
				now->childcnt[1]++;

				now->childs[1][id] = new Chip;

				now->childs[1][id]->parent = now;

				now->childs[1][id]->bone = new Bone(0, 0, 0, 1, 0, 0, 0, 0, 0, now->bone);
				now = now->childs[1][id];

				now->type = rc_getchiptype(tp + 2);

				float vx2[3];

				RotateVecPrepare(vx2, now->type, 0);

				now->dir = 1;

				mr = 1;

				if (now->type == RC_RUDDER || now->type == RC_RUDDERF) mr = -1;

				ResetChilds(now);

				memcpy(now->vx2, vx2, sizeof(vx2));
			} else if(*tp == 'E') {
				rc_pushc((int)now);

				int id = now->childcnt[2];
				now->childcnt[2]++;

				now->childs[2][id] = new Chip;

				now->childs[2][id]->parent = now;

				now->childs[2][id]->bone = new Bone(0, 0, 0, 1, 0, 0, 0, 0, 0, now->bone);
				now = now->childs[2][id];

				now->type = rc_getchiptype(tp + 2);

				float vx2[3];

				RotateVecPrepare(vx2, now->type, 90);

				now->dir = 2;

				mr = 1;

				if (now->type == RC_RUDDER || now->type == RC_RUDDERF) mr = -1;

				ResetChilds(now);

				memcpy(now->vx2, vx2, sizeof(vx2));
			} else if(*tp == 'W') {
				rc_pushc((int)now);

				int id = now->childcnt[3];
				now->childcnt[3]++;

				now->childs[3][id] = new Chip;

				now->childs[3][id]->parent = now;

				now->childs[3][id]->bone = new Bone(0, 0, 0, 1, 0, 0, 0, 0, 0, now->bone);
				now = now->childs[3][id];

				now->type = rc_getchiptype(tp + 2);

				float vx2[3];

				RotateVecPrepare(vx2, now->type, 90);

				now->dir = 3;

				mr = -1;

				ResetChilds(now);

				memcpy(now->vx2, vx2, sizeof(vx2));
			}
			int an;
			int ann = rc_numarg(ln);
			int col = 0xffffff;
			float angle = 0;
			float power = 0;
			int op = 0;

			now->angle_val = 0;
			now->color_val = 0;
			now->power_val = 0;


			for(an = 0;an < ann; an++) {
				char *ag = rc_getargstr(ln,an);
				while (*ag == ' ' || *ag == '\t') ag++;
				if(strncmp(ag,"angle=",6) == 0) {
					char *p = ag+6;
					if (isdigit(*p) || (*p == '-' && isdigit(*(p + 1)))) {
						angle = strtod(p, &p);
					}
					else if (*p == '#') {
						p++;
						angle = strtol(p, &p, 16);
					}
					else {
						now->angle_vm = 1;
						if (*p == '-') {
							now->angle_vm = -1;
							p++;
						}
						now->angle_val = rc_searchval(p);
						if (now->angle_val) angle = now->angle_val->def;
					}
				}
				if (strncmp(ag, "power=", 6) == 0) {
					char *p = ag + 6;
					if (isdigit(*p) || (*p == '-' && isdigit(*(p+1)))) {
						power = strtod(p, &p);
					}
					else if (*p == '#') {
						p++;
						power = strtol(p, &p, 16);
					}
					else {
						now->power_vm = 1;
						if (*p == '-') {
							now->power_vm = -1;
							p++;
						}
						now->power_val = rc_searchval(p);
						if (now->power_val) power = now->power_val->def;
					}
				}
				if(strncmp(ag,"option=",7) == 0) {
					char *p = ag+7;
					op = strtol(p,&p,0);
				}
				if(strncmp(ag,"color=",6) == 0) {
					char *p = ag+6;
					if(*p == '#') {
						p++;
						col = strtol(p,&p,16);
					}
					else if (isdigit(*p) || (*p == '-' && isdigit(*(p + 1)))) {
						col = strtol(p, &p, 0);
					}
					else {
						now->color_vm = 1;
						if (*p == '-') {
							now->color_vm = -1;
							p++;
						}
						now->color_val = rc_searchval(p);
						if (now->color_val) col = now->color_val->def;
					}
				}
			}

			float *vx2 = now->vx2;

			if (now->parent) {
				int pdir = now->parent->dir;

				float mx = 0, my = 0, mz = 0;
				float mx2 = 0, my2 = 0, mz2 = 0;

				float mm = diffmul;

				switch (now->dir) {
				case 0:
					mz = -1;
					if (pdir == 2) mz2 = 0.5, mx2 = 0.5;
					if (pdir == 3) mz2 = 0.5, mx2 = -0.5;
					if (pdir == 1) mz2 = 1;
					break;
				case 1:
					mz = 1;
					if (pdir == 2) mz2 = -0.5, mx2 = 0.5;
					if (pdir == 3) mz2 = -0.5, mx2 = -0.5;
					if (pdir == 0) mz2 = -1;
					break;
				case 2:
					mx = 1;
					if (pdir == 0) mz2 = -0.5, mx2 = -0.5;
					if (pdir == 1) mz2 = 0.5, mx2 = -0.5;
					if (pdir == 3) mx2 = -1;
					break;
				case 3:
					mx = -1;
					if (pdir == 0) mz2 = -0.5, mx2 = 0.5;
					if (pdir == 1) mz2 = 0.5, mx2 = 0.5;
					if (pdir == 2) mx2 = 1;
					break;
				}

				now->bone->setPosition(DIFF_CHIP * mx * mm + mx2 * DIFF_CHIP, DIFF_CHIP * my * mm + my2 * DIFF_CHIP, DIFF_CHIP * mz * mm + mz2 * DIFF_CHIP, 1);
			}

			float rangle = (angle * mr);

			now->bone->setRotation(vx2[0], vx2[1], vx2[2], rangle);

			SetDrawMove(now);



			now->color = col;
			now->option = op;
			now->angle = angle;
			now->mr = mr;
			now->power = power;

			if (now->type >= RC_COWL0 && now->type <= RC_COWL5) now->type = (RC_CHIP_TYPE)(now->type + (RC_CHIP_TYPE)op);

			char *e = &ln[strlen(ln) - 1];
			while (*e == 0x0d || *e == 0x0a) e--;

			if (*e == '}') {
				now = now->parent;
				if (!now) break;
			}
		skip_bodyf:
			;
		}
	skip:

		if (strcmp(ln,"Body\n") == 0) { bodyf = true; fgets(ln,1024,fp); }
		if (strcmp(ln,"Body {\n") == 0) bodyf = true;
		if (strcmp(ln, "Val\n") == 0) { valf = true; fgets(ln, 1024, fp); }
		if (strcmp(ln, "Val {\n") == 0) valf = true;
		if (strcmp(ln, "Key\n") == 0) { keyf = true; fgets(ln, 1024, fp); }
		if (strcmp(ln, "Key {\n") == 0) keyf = true;
	} while(!feof(fp));

	viewMatrix.loadIdentity();
	viewMatrix.lookat(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	projectionMatrix.loadIdentity();

	fclose(fp);
}

btQuaternion MatrixToQuaternion(Matrix mat) {
	//btQuaternion q(0,0,0,1);

	const float *m = mat.get();
	float x, y, z, w;

	float s;
	float tr = m[0 * 4 + 0] + m[1 * 4 + 1] + m[2 * 4 + 2] + 1.0f;
	if (tr >= 1.0f) {
		s = 0.5f / sqrt(tr);
		w = 0.25f / s;
		x = (m[1 * 4 + 2] - m[2 * 4 + 1]) * s;
		y = (m[2 * 4 + 0] - m[0 * 4 + 2]) * s;
		z = (m[0 * 4 + 1] - m[1 * 4 + 0]) * s;
		return btQuaternion(x,y,z,w);
	}
	else {
		float max;
		if (m[1 * 4 + 1] > m[2 * 4 + 2]) {
			max = m[1 * 4 + 1];
		}
		else {
			max = m[2 * 4 + 2];
		}

		if (max < m[0 * 4 + 0]) {
			s = sqrt(m[0 * 4 + 0] - (m[1 * 4 + 1] + m[2 * 4 + 2]) + 1.0f);
			float x = s * 0.5f;
			s = 0.5f / s;
			x = x;
			y = (m[0 * 4 + 1] + m[1 * 4 + 0]) * s;
			z = (m[2 * 4 + 0] + m[0 * 4 + 2]) * s;
			w = (m[1 * 4 + 2] - m[2 * 4 + 1]) * s;
			return btQuaternion(x, y, z, w);
		}
		else if (max == m[1 * 4 + 1]) {
			s = sqrt(m[1 * 4 + 1] - (m[2 * 4 + 2] + m[0 * 4 + 0]) + 1.0f);
			float y = s * 0.5f;
			s = 0.5f / s;
			x = (m[0 * 4 + 1] + m[1 * 4 + 0]) * s;
			y = y;
			z = (m[1 * 4 + 2] + m[2 * 4 + 1]) * s;
			w = (m[2 * 4 + 0] - m[0 * 4 + 2]) * s;
			return btQuaternion(x, y, z, w);
		}
		else {
			s = sqrt(m[2 * 4 + 2] - (m[0 * 4 + 0] + m[1 * 4 + 1]) + 1.0f);
			float z = s * 0.5f;
			s = 0.5f / s;
			x = (m[2 * 4 + 0] + m[0 * 4 + 2]) * s;
			y = (m[1 * 4 + 2] + m[2 * 4 + 1]) * s;
			z = z;
			w = (m[0 * 4 + 1] - m[1 * 4 + 0]) * s;
			return btQuaternion(x, y, z, w);
		}
	}
}

#define RADIAN( n ) ( ( n ) * ( 3.1415926535897932384626 / 180.0 ) )

void VectorVertex(float *v, float *vecX, float *vecY, float *vecZ, float *ofs)
{
	float x = v[0];
	float y = v[1];
	float z = v[2];

	float xx = vecX[0];
	float xy = vecX[1];
	float xz = vecX[2];

	float yx = vecY[0];
	float yy = vecY[1];
	float yz = vecY[2];
	
	float zx = vecZ[0];
	float zy = vecZ[1];
	float zz = vecZ[2];

	float a = (xx * x + yx * y + zx * z) + ofs[0];
	float b = (xy * x + yy * y + zy * z) + ofs[1];
	float c = (xz * x + yz * y + zz * z) + ofs[2];

	v[0] = a;
	v[1] = b;
	v[2] = c;
}

void VectorVertexInvert(float *v, float *vecX, float *vecY, float *vecZ, float *ofs)
{
	float x = v[0];
	float y = v[1];
	float z = v[2];

	float xx = -vecX[0];
	float xy = -vecX[1];
	float xz = -vecX[2];

	float yx = -vecY[0];
	float yy = -vecY[1];
	float yz = -vecY[2];

	float zx = -vecZ[0];
	float zy = -vecZ[1];
	float zz = -vecZ[2];

	float a = (xx * x + yx * y + zx * z) - ofs[0];
	float b = (xy * x + yy * y + zy * z) - ofs[1];
	float c = (xz * x + yz * y + zz * z) - ofs[2];

	v[0] = a;
	v[1] = b;
	v[2] = c;
}

Matrix CalcBone(const Bone *b, float *bottom, float *vecX, float *vecY, float *vecZ)
{
	/* ボーンの図形データ */
	static const GLfloat boneVertex[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const GLfloat boneVertexX[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	static const GLfloat boneVertexY[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	static const GLfloat boneVertexZ[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	/* ボーンの長さに合わせてスケーリングする変換行列 */
	Matrix scale;
	scale.loadScale(b->getLength(), b->getLength(), b->getLength());

	/* ボーンを初期位置に配置する変換行列とアニメーション後の変換行列 */
	Matrix initial, animated, ident, rotate;
	initial.loadIdentity();
	animated.loadIdentity();
	rotate.loadIdentity();

	/* 現在の視野変換行列をかけておく */
	do {
		Matrix temp, temp2;
		temp.loadTranslate(b->getPosition());
		temp.rotate(b->getRotation());
		initial = temp * initial;
		temp.multiply(b->getAnimation());
		animated = temp * animated;
		temp2.rotate(b->getRotation());
		temp2.multiply(b->getAnimation());
		rotate = temp * rotate;
	} while ((b = b->getParent()) != 0);

	/* ボーンの初期位置における根元と先端の位置を求める */
	initial = viewMatrix * initial;
	animated = viewMatrix * animated;

	/* ボーンの初期位置における根元の位置を求める */
	animated.projection(bottom, boneVertex);
	animated.projection(vecX, boneVertexX);
	animated.projection(vecY, boneVertexY);
	animated.projection(vecZ, boneVertexZ);

	vecX[0] -= bottom[0];
	vecX[1] -= bottom[1];
	vecX[2] -= bottom[2];

	vecY[0] -= bottom[0];
	vecY[1] -= bottom[1];
	vecY[2] -= bottom[2];

	vecZ[0] -= bottom[0];
	vecZ[1] -= bottom[1];
	vecZ[2] -= bottom[2];

	return rotate;
}

typedef struct {
	double x, y, z, w;
} Quaternion;

#define PI 3.141593

btVector3 quatToEuler(btQuaternion quat)
{
	float  heading, attitude, bank;
	Quaternion q1;
	q1.x = quat.getX();
	q1.y = quat.getY();
	q1.z = quat.getZ();
	q1.w = quat.getW();
	double test = q1.x*q1.y + q1.z*q1.w;
	if (test > 0.499) { // singularity at north pole
		heading = 2 * atan2(q1.x, q1.w);
		attitude = PI / 2;
		bank = 0;
		return btVector3(0, 0, 0);
	}
	if (test < -0.499) { // singularity at south pole
		heading = -2 * atan2(q1.x, q1.w);
		attitude = -PI / 2;
		bank = 0;
		return  btVector3(0, 0, 0);
	}
	double sqx = q1.x*q1.x;
	double sqy = q1.y*q1.y;
	double sqz = q1.z*q1.z;
	heading = atan2(2 * q1.y*q1.w - 2 * q1.x*q1.z, 1 - 2 * sqy - 2 * sqz);
	attitude = asin(2 * test);
	bank = atan2(2 * q1.x*q1.w - 2 * q1.y*q1.z, 1 - 2 * sqx - 2 * sqz);
	btVector3 vec(bank, heading, attitude);
	return vec;
}

void DrawChipGraphic(Chip *now)
{
	GLfloat vs[] = {
		-DIFF_CHIP / 2.0, 0, -DIFF_CHIP / 2.0, 1,
		DIFF_CHIP / 2.0, 0, -DIFF_CHIP / 2.0, 1,
		DIFF_CHIP / 2.0, 0,  DIFF_CHIP / 2.0, 1,
		-DIFF_CHIP / 2.0, 0,  DIFF_CHIP / 2.0, 1,
	};

	if ((now->type == RC_FRAME || now->type == RC_TRIMF || now->type == RC_RUDDERF) && now->option == 1) return;
	//if (now->type >= RC_COWL0 && now->type <= RC_COWL5) return;

	GLfloat vs2[sizeof(vs) / sizeof(GLfloat)];

	memcpy(vs2, vs, sizeof(vs));

	/*for (int j = 0; j < 6; j++) VectorVertex(vs + j * 4, bonevecX, bonevecY, bonevecZ, bonepos);*/

	btScalar m[16];
	btMatrix3x3	rot;

	btRigidBody* body = now->phy;

	if (body) {
		btDefaultMotionState* ms = (btDefaultMotionState*)body->getMotionState();
		//ms->m_graphicsWorldTrans.getOpenGLMatrix(m);
		rot = ms->m_graphicsWorldTrans.getBasis();

		btTransform transform;
		ms->getWorldTransform(transform);
		transform.getOpenGLMatrix(m);

		glPushMatrix();

		btScalar m2[16];

		rot.getOpenGLSubMatrix(m2);

		glMultMatrixf(m);

		if (now->type == RC_RLW || now->type == RC_WHEEL) {
			btScalar angle = -now->joint->getHingeAngle();

			for (int j = 0; j < 4; j++) {
				rc_rotate3d(&(vs + j * 4)[0], &(vs + j * 4)[1], &(vs + j * 4)[2], (vs + j * 4)[0], (vs + j * 4)[1], (vs + j * 4)[2], angle / 3.14159f * 180.0f, 1);
			}
		}
	}
	else {
		float bonepos[4], bonevecX[4], bonevecY[4], bonevecZ[4];
		CalcBone(now->bone, bonepos, bonevecX, bonevecY, bonevecZ);

		for (int j = 0; j < 4; j++) {
			(vs + j * 4)[0] += now->dm[0];
			(vs + j * 4)[1] += now->dm[1];
			(vs + j * 4)[2] += now->dm[2];
		}

		for (int j = 0; j < 4; j++) VectorVertex(vs + j * 4, bonevecX, bonevecY, bonevecZ, bonepos);

		glPushMatrix();
	}

	/*if (now->joint) {

		for (int i = 0; i < 2; i++) {
			float bonepos[3];
			bonepos[0] = now->pivot[i].getX();
			bonepos[1] = now->pivot[i].getY();
			bonepos[2] = now->pivot[i].getZ();

			glBegin(GL_LINES);

			glVertex3f(bonepos[0] - 0.06, bonepos[1], bonepos[2]);
			glVertex3f(bonepos[0] + 0.06, bonepos[1], bonepos[2]);
			glVertex3f(bonepos[0], bonepos[1] - 0.06, bonepos[2]);
			glVertex3f(bonepos[0], bonepos[1] + 0.06, bonepos[2]);
			glVertex3f(bonepos[0], bonepos[1], bonepos[2] - 0.06);
			glVertex3f(bonepos[0], bonepos[1], bonepos[2] + 0.06);

			glEnd();
		}
	}*/

	glBindTexture(GL_TEXTURE_2D, chip_graphic[now->type]);
	
	glBegin(GL_POLYGON);

	//glColor3i((now->color >> 16) & 0xff, (now->color >> 8) & 0xff, (now->color >> 0) & 0xff);
	GLfloat color[4];
	GLfloat colorhalf[4];
	int r, g, b;
	rc_fromcolor(now->color, &r, &g, &b);
	color[0] = r / 255.0;
	color[1] = g / 255.0;
	color[2] = b / 255.0;
	color[3] = 1;

	colorhalf[0] = r / 255.0 / 2.0;
	colorhalf[1] = g / 255.0 / 2.0;
	colorhalf[2] = b / 255.0 / 2.0;
	colorhalf[3] = 1;


	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, colorhalf);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);

	float anglist[] = { 0, 180, 90, 270 };

	for (int j = 0; j < 4; j++) {
		float u = (vs2 + j * 4)[0];
		float v = (vs2 + j * 4)[2];
		float d = 0;

		if(now->dir >= 0) rc_rotate3d(&u, &d, &v, u, d, v, anglist[now->dir], 1);

		u = (u + DIFF_CHIP / 2) / DIFF_CHIP;
		v = (v + DIFF_CHIP / 2) / DIFF_CHIP;

		//u = (now->dir & 1) ? u : 1 - u;
		//v = (now->dir & 1) ? 1 - v : v;

		glTexCoord2f(u, v);
		glVertex4fv(vs + j * 4);
	}

	glEnd();

	glPopMatrix();
}

void ApplyBonePos(Chip *now)
{
	if (!now->parent) {
		if (now->phy) {
			btRigidBody *body = now->phy;
			btDefaultMotionState* ms = (btDefaultMotionState*)body->getMotionState();
			btVector3 pos = ms->m_graphicsWorldTrans.getOrigin();
			btTransform transform;
			body->getMotionState()->getWorldTransform(transform);

			btQuaternion q = transform.getRotation();

			btVector3 axis = q.getAxis();
			btScalar angle = q.getAngle();

			now->bone->setPosition(pos.getX(), pos.getY(), pos.getZ());

			now->bone->setRotation(axis.getX(), axis.getY(), axis.getZ(), angle / 3.14159 * 180.0);
		}
	}
	else {
		if (now->phy) {
			/*btRigidBody *body = now->phy;
			btDefaultMotionState* ms = (btDefaultMotionState*)body->getMotionState();
			btTransform transform;
			body->getMotionState()->getWorldTransform(transform);

			btQuaternion q = transform.getRotation();

			btVector3 axis = q.getAxis();
			btScalar angle = q.getAngle();

			now->bone->setRotation(axis.getX(), axis.getY(), axis.getZ(), angle / 3.14159 * 180.0);*/
			btHingeConstraint *hinge = now->joint;
			btScalar angle = -hinge->getHingeAngle() / 3.14159 * 180;

			now->bone->setRotation(now->vx2[0], now->vx2[1], now->vx2[2], angle);
		}
	}
	for (int i = 0; i < 4; i++) for (int j = 0; j < now->childcnt[i]; j++) ApplyBonePos(now->childs[i][j]);
}

void DrawChip(Chip *now)
{
	
	DrawChipGraphic(now);
	for (int i = 0; i < 4; i++) for (int j = 0; j < now->childcnt[i]; j++) DrawChip(now->childs[i][j]);
}

void DrawModel(void)
{
	GLfloat amb[] = { 0.5,0.5,0.5 };
	GLfloat dif[] = { 0.5,0.5,0.5 };
	GLfloat spe[] = { 1.0,1.0,1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 5);

	ApplyBonePos(Chips);

	DrawChip(Chips);
}

#define COWLMASS 1e-20

void RegisterOneChip(Chip *now)
{
	const float mass_list[] = {
		25.200002670288086,
		25.200002670288086,
		12.600001335144043,
		25.200002670288086,
		25.200002670288086,
		50.400005340576172,
		36.756637573242188,
		36.756637573242188,
		36.756637573242188,
		100.800010681152340,
		COWLMASS,COWLMASS,COWLMASS,COWLMASS,COWLMASS,COWLMASS,
		12.600001335144043,
		12.600001335144043
	};

	now->phy = 0;
	now->joint = 0;
	now->joint2 = 0;

	//if ((now->type == RC_FRAME || now->type == RC_TRIMF || now->type == RC_RUDDERF) && now->option == 1) return;
	if (now->type >= RC_COWL0 && now->type <= RC_COWL5) return;


	btCollisionShape *shape;

	if (now->type == RC_RLW || now->type == RC_WHEEL || now->type == RC_JET) {
		shape = new btCylinderShape(btVector3(DIFF_CHIP / 2.0, DIFF_CHIP / 5.0, DIFF_CHIP / 2.0));
	}
	else {
		shape = new btBoxShape(btVector3(DIFF_CHIP / 2.0, DIFF_CHIP / 5.0, DIFF_CHIP / 2.0));
	}

	float bonepos[4], bonevecX[4], bonevecY[4], bonevecZ[4];
	Matrix rot = CalcBone(now->bone, bonepos, bonevecX, bonevecY, bonevecZ);

	float dmv[4];
	float ofs[] = { 0,0,0,0 };
	dmv[0] = now->dm[0];
	dmv[1] = now->dm[1];
	dmv[2] = now->dm[2];
	dmv[3] = 1;

	VectorVertex(dmv, bonevecX, bonevecY, bonevecZ, ofs);

	btQuaternion qrot = MatrixToQuaternion(rot);
	btVector3 pos(bonepos[0] + dmv[0], bonepos[1] + dmv[1], bonepos[2] + dmv[2]);

	btDefaultMotionState* motion_state = new btDefaultMotionState(btTransform(qrot, pos));

	btVector3 inertia(0, 0, 0);
	shape->calculateLocalInertia(mass_list[now->type], inertia);

	now->phy = new btRigidBody(mass_list[now->type], motion_state, shape, inertia);

	g_pDynamicsWorld->addRigidBody(now->phy, RX_COL_CHIP, RX_COL_GROUND);

	if (now->parent) {
		float nbone[3] = { 0,0,0 };

		switch (now->dir) {
		case 0: //-
			nbone[2] = -DIFF_CHIP / 2;
			break;
		case 1: //+
			nbone[2] = DIFF_CHIP / 2;
			break;
		case 2: //+
			nbone[0] = DIFF_CHIP / 2;
			break;
		case 3: //-
			nbone[0] = -DIFF_CHIP / 2;
			break;
		}

		btVector3 pivot1(nbone[0], nbone[1], nbone[2]);
		btVector3 pivot2(-nbone[0], -nbone[1], -nbone[2]);

		now->pivot[0] = pivot1;
		now->pivot[1] = pivot2;

		btTransform frameInA;
		frameInA = btTransform::getIdentity();
		frameInA.setOrigin(pivot1);

		btTransform frameInB;
		frameInB = btTransform::getIdentity();
		frameInB.setOrigin(pivot2);

		btVector3 axis = btVector3(now->vx2[0], now->vx2[1], now->vx2[2]);

		float angle = -RADIAN(now->mr * now->angle);

		if (now->type == RC_RLW || now->type == RC_WHEEL) {
			btCylinderShape	*shapeR = new btCylinderShape(btVector3(DIFF_CHIP / 4.0, DIFF_CHIP / 100.0, DIFF_CHIP / 4.0));
			btVector3 inertiaR(0, 0, 0);
			shapeR->calculateLocalInertia(42.411502838134766, inertiaR);

			btRigidBody *rim = new btRigidBody(42.411502838134766, motion_state, shapeR, inertiaR);
			g_pDynamicsWorld->addRigidBody(rim, RX_COL_NOTHING, RX_COL_GROUND);
			now->joint2 = new btHingeConstraint(*now->parent->phy, *rim, pivot1, pivot2, axis, axis);

			now->joint2->setLimit(angle - 0.0001, angle + 0.0001);

			g_pDynamicsWorld->addConstraint(now->joint2);

			dmv[0] = 0;
			dmv[1] = 1;
			dmv[2] = 0;
			dmv[3] = 1;

			//VectorVertex(dmv, bonevecX, bonevecY, bonevecZ, ofs);

			btVector3 pivot1R(0, 0, 0);
			btVector3 pivot2R(0, 0, 0);
			btVector3 axisR = btVector3(dmv[0], dmv[1], dmv[2]);

			now->joint = new btHingeConstraint(*rim, *now->phy, pivot1R, pivot2R, axisR, axisR);

			g_pDynamicsWorld->addConstraint(now->joint);

			//now->joint->enableAngularMotor(true, btRadians(180 * 30), now->power / 100.0f);
			btRigidBody *body = now->phy;
			btVector3 m = body->getWorldTransform().getBasis()*btVector3(0, -now->power / 1000.0f, 0);
			body->applyTorqueImpulse(m);
		}
		else {
			now->joint = new btHingeConstraint(*now->parent->phy, *now->phy, pivot1, pivot2, axis, axis);

			now->joint->setLimit(angle - 0.0001, angle + 0.0001);
			now->joint->setParam(BT_CONSTRAINT_CFM, 0);
			now->joint->setParam(BT_CONSTRAINT_STOP_CFM, 0);
			//now->joint->setParam(BT_CONSTRAINT_STOP_ERP, 1);
		}

		g_pDynamicsWorld->addConstraint(now->joint, true);
	}
}

void RegisterChip(Chip *now)
{
	RegisterOneChip(now);
	for (int i = 0; i < 4; i++) for (int j = 0; j < now->childcnt[i]; j++) RegisterChip(now->childs[i][j]);
}

void RegisterModel(void)
{
	RegisterChip(Chips);
}