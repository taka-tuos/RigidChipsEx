#include "RCEX.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "XFile.h"

MODEL *model;

char *chipnames[] = {
	"Resources/chip.bmp",
	"Resources/core.bmp",
	"Resources/frame.bmp",
	"Resources/trim.bmp",
	"Resources/rudder.bmp",
	"Resources/arm.bmp",
	"Resources/wheel.bmp",
	"Resources/rlw.bmp",
	"Resources/jet.bmp",
	"Resources/weight.bmp",
	"Resources/cowl0.bmp",
	"Resources/cowl1.bmp",
	"Resources/cowl2.bmp",
	"Resources/cowl3.bmp",
	"Resources/cowl4.bmp",
	"Resources/cowl5.bmp",
	"Resources/trimf.bmp",
	"Resources/rudderf.bmp"
};

GLuint chip_graphic[RC_KINDN];

extern Chip *Chips;
extern Val Vals[256];
extern Key Keys[16];

int keypress[16];
int keytrig[16];

int ww = 800, wh = 600;

static GLfloat lightPosition[4] = { 20.0, 200.0, 100.0, 0.0 }; //光源の位置
static GLfloat lightDiffuse[3]  = { 1.0,   1.0, 1.0  }; //拡散光
static GLfloat lightAmbient[3]  = { 1.0, 1.0, 1.0 }; //環境光
static GLfloat lightSpecular[3] = { 1.0,   1.0, 1.0  }; //鏡面光

float ang = 0;

extern btDynamicsWorld* g_pDynamicsWorld;

btIDebugDraw *g_pDebugDraw;

void rc_rotate3d(float *ox, float *oy, float *oz, float ax, float ay, float az, double angle, int axis);

btIDebugDraw* CreateDebugDraw() {

	class DebugDraw : public btIDebugDraw
	{
	private:
		//デバッグモード
		int m_DebugMode;
	public:
		//コストラクタ
		DebugDraw()
		{}

		//ラインの描画
		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
		{
			glDisable(GL_LIGHTING);

			glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

			glColor4f(color.x(), color.y(), color.z(), 1.0f);
			glBegin(GL_LINES);
			glVertex3fv(from);
			glVertex3fv(to);
			glEnd();

			glPopAttrib();

			glEnable(GL_LIGHTING);
		}

		//ラインの描画
		void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
		{
			glDisable(GL_LIGHTING);

			glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

			glBegin(GL_LINES);
			glColor4f(fromColor.x(), fromColor.y(), fromColor.z(), 1.0f);
			glVertex3fv(from);
			glColor4f(toColor.x(), toColor.y(), toColor.z(), 1.0f);
			glVertex3fv(to);
			glEnd();

			glPopAttrib();

			glEnable(GL_LIGHTING);
		}

		//接触点の描画
		void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
		{
			glDisable(GL_LIGHTING);

			glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

			btVector3 to = PointOnB + normalOnB*distance;
			const btVector3&from = PointOnB;
			glColor4f(color.getX(), color.getY(), color.getZ(), 1.f);

			glBegin(GL_LINES);
			glVertex3d(from.getX(), from.getY(), from.getZ());
			glVertex3d(to.getX(), to.getY(), to.getZ());
			glEnd();

			glPopAttrib();

			glEnable(GL_LIGHTING);
		}

		//エラーの表示
		void reportErrorWarning(const char* warningString)
		{
			puts(warningString);
		}

		//３Ｄテキストの描画
		void draw3dText(const btVector3& location, const char* textString)
		{}

		//デバッグモードの設定
		void setDebugMode(int debugMode)
		{
			m_DebugMode = debugMode;
		}

		//デバッグモードの取得
		int getDebugMode() const
		{
			return m_DebugMode;
		}
	};

	return new DebugDraw;
}

void ApplyVals(Chip *now)
{
	if (now->angle_val) {
		now->angle = now->angle_val->now * now->angle_vm;
		float angle = -RADIAN(now->mr * now->angle);
		if (now->joint2) {
			now->joint2->setLimit(angle - 0.0001, angle + 0.0001);
		}
		else {
			now->joint->setLimit(angle - 0.0001, angle + 0.0001);
		}
	}

	if (now->color_val) {
		now->color = now->color_val->now * now->color_vm;
	}

	if (now->power_val) {
		if (now->type == RC_RLW || now->type == RC_WHEEL) {
			now->power = now->power_val->now * now->power_vm;
			//now->joint->enableAngularMotor(true, btRadians(180 * 30), now->power / 100.0f);
			//now->phy->applyTorqueImpulse(now->phy->getWorldTransform()* now->power);
			btRigidBody *body = now->phy;
			btVector3 m = body->getWorldTransform().getBasis()*btVector3(0, -now->power / 1000.0f, 0);
			body->applyTorqueImpulse(m);
		}

		if (now->type == RC_JET) {
			now->power = now->power_val->now * now->power_vm;
			btRigidBody *body = now->phy;
			btVector3 m = body->getWorldTransform().getBasis()*btVector3(0, now->power / 100.0f, 0);
			body->applyCentralImpulse(m);
		}
	}

	for (int i = 0; i < 4; i++) for (int j = 0; j < now->childcnt[i]; j++) ApplyVals(now->childs[i][j]);
}

void keydown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'z':
		keytrig[4] = 1;
		keypress[4] = 1;
		break;
	case 'x':
		keytrig[5] = 1;
		keypress[5] = 1;
		break;
	case 'c':
		keytrig[6] = 1;
		keypress[6] = 1;
		break;
	case 'a':
		keytrig[7] = 1;
		keypress[7] = 1;
		break;
	case 's':
		keytrig[8] = 1;
		keypress[8] = 1;
		break;
	case 'd':
		keytrig[9] = 1;
		keypress[9] = 1;
		break;
	case 'v':
		keytrig[10] = 1;
		keypress[10] = 1;
		break;
	case 'b':
		keytrig[11] = 1;
		keypress[11] = 1;
		break;
	case 'f':
		keytrig[12] = 1;
		keypress[12] = 1;
		break;
	case 'g':
		keytrig[13] = 1;
		keypress[13] = 1;
		break;
	case 'q':
		keytrig[14] = 1;
		keypress[14] = 1;
		break;
	case 'w':
		keytrig[15] = 1;
		keypress[15] = 1;
		break;
	case 'e':
		keytrig[16] = 1;
		keypress[16] = 1;
		break;
	}
}

void keydownSP(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		keytrig[0] = 1;
		keypress[0] = 1;
		break;
	case GLUT_KEY_DOWN:
		keytrig[1] = 1;
		keypress[1] = 1;
		break;
	case GLUT_KEY_LEFT:
		keytrig[2] = 1;
		keypress[2] = 1;
		break;
	case GLUT_KEY_RIGHT:
		keytrig[3] = 1;
		keypress[3] = 1;
		break;
	}
}

void keyup(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'z':
		keypress[4] = 0;
		break;
	case 'x':
		keypress[5] = 0;
		break;
	case 'c':
		keypress[6] = 0;
		break;
	case 'a':
		keypress[7] = 0;
		break;
	case 's':
		keypress[8] = 0;
		break;
	case 'd':
		keypress[9] = 0;
		break;
	case 'v':
		keypress[10] = 0;
		break;
	case 'b':
		keypress[11] = 0;
		break;
	case 'f':
		keypress[12] = 0;
		break;
	case 'g':
		keypress[13] = 0;
		break;
	case 'q':
		keypress[14] = 0;
		break;
	case 'w':
		keypress[15] = 0;
		break;
	case 'e':
		keypress[16] = 0;
		break;
	}
}

void keyupSP(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		keypress[0] = 0;
		break;
	case GLUT_KEY_DOWN:
		keypress[1] = 0;
		break;
	case GLUT_KEY_LEFT:
		keypress[2] = 0;
		break;
	case GLUT_KEY_RIGHT:
		keypress[3] = 0;
		break;
	}
}

void timer(int) {
	glutPostRedisplay();
	glutTimerFunc(1000 / 30, timer, 0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, ww, wh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//視野角,アスペクト比(ウィンドウの幅/高さ),描画する範囲(最も近い距離,最も遠い距離)
	gluPerspective(30.0, (double)ww / (double)wh, 1.0, 10000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//視点の設定
	static float a=-0,b=50,c=10;

	btRigidBody* body = Chips->phy;

	btDefaultMotionState* ms = (btDefaultMotionState*)body->getMotionState();

	btTransform trans;

	ms->getWorldTransform(trans);

	btVector3 pos = trans.getOrigin();

	gluLookAt(a,b,c, //カメラの座標
		pos.getX(), pos.getY(), pos.getZ(), // 注視点の座標
		0.0,1.0,0.0); // 画面の上方向を指すベクトル*/

	//ライトの設定
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	glEnable(GL_TEXTURE_2D);
	DrawModel();
	model->Draw();
	//g_pDynamicsWorld->debugDrawWorld();
	glutSwapBuffers();
	ang += RADIAN(10);

	for (int i = 0; i < 25; i++) g_pDynamicsWorld->stepSimulation(0.0333, 1, 0.00333/2.5);
	
	for (int i = 0; i < 256; i++) {
		Val *now = &Vals[i];
		if (now->now != now->def && !now->inmove) {
			if (now->now > now->def) {
				now->now -= fabs(now->step);
				if (now->now < now->def) now->now = now->def;
			}

			if (now->now < now->def) {
				now->now += fabs(now->step);
				if (now->now > now->def) now->now = now->def;
			}
		}
	}

	int in_inmove[256];

	memset(in_inmove, 0, sizeof(in_inmove));

	for (int i = 0; i < 16; i++) {
		Key *now = &Keys[i];

		if (keypress[i]) {
			for (int j = 0; j < now->valcnt; j++) {
				Val *nowv = now->val[j];
				nowv->now += now->step[j];
				nowv->inmove = 1;
				in_inmove[nowv->ind] = 1;
				if (now->step[j] < 0) {
					if (nowv->now < nowv->min) nowv->now = nowv->min;
				}
				else {
					if (nowv->now > nowv->max) nowv->now = nowv->max;
				}
			}
		}
		else {
			for (int j = 0; j < now->valcnt; j++) {
				Val *nowv = now->val[j];
				if(!in_inmove[nowv->ind]) nowv->inmove = 0;
			}
		}
	}

	memset(keytrig, 0, sizeof(keytrig));

	ApplyVals(Chips);
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, (double)w / (double)h, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	ww = w, wh = h;
}

void init(void)
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glDisable(GL_CULL_FACE);

	glAlphaFunc(GL_GREATER, 0.5);

	glGenTextures(RC_KINDN, chip_graphic);
	for (int i = 0; i < RC_KINDN; i++) {
		int w, h, bpp;
		stbi_uc *bit = stbi_load(chipnames[i], &w, &h, &bpp, 4);
		unsigned int *p = (unsigned int *)bit;

		for (int j = 0; j < w*h; j++) p[j] = (p[j] & 0xffffff) == 0x008000 ? 0 : p[j];

		glBindTexture(GL_TEXTURE_2D, chip_graphic[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bit);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	float ofs[] = { 0,50,0 };

	ReadModel("basic.txt", ofs);

	model = new MODEL("Land.x", 1);

	// 衝突検出方法の選択(デフォルトを選択)
	btDefaultCollisionConfiguration *config = new btDefaultCollisionConfiguration();
	btCollisionDispatcher *dispatcher = new btCollisionDispatcher(config);

	// ブロードフェーズ法の設定(Dynamic AABB tree method)
	btDbvtBroadphase *broadphase = new btDbvtBroadphase();

	// 拘束(剛体間リンク)のソルバ設定
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

	// Bulletのワールド作成
	g_pDynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);
	//g_pDynamicsWorld->setGravity(btVector3(0, 0, 0));

	RegisterModel();

	model->Register();

	g_pDebugDraw = CreateDebugDraw();

	g_pDynamicsWorld->setDebugDrawer(g_pDebugDraw);
}

void idle(void)
{
	//glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInitWindowSize(800,600);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutCreateWindow("Rigidchips EX");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);

	timer(0);

	glutSpecialFunc(keydownSP);
	glutKeyboardFunc(keydown);

	glutSpecialUpFunc(keyupSP);
	glutKeyboardUpFunc(keyup);

	init();
	glutMainLoop();
	return 0;
}