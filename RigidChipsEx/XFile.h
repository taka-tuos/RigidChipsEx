#include <GL/glut.h>
#include <vector>

#include <btBulletDynamicsCommon.h>

using namespace std;

#ifndef __XFILE_H__
#define __XFILE_H__

class TEXTURE {
public:
	TEXTURE();
	TEXTURE(const char* FileName);//�R���X�g���N�^
	void LOAD_PNG(const char* FileName);//PNG�ǂݍ���
	unsigned char* image;//�C���[�W�|�C���^
	unsigned int Width, Height;//�摜�T�C�Y
};

//3�̃x�N�g��
struct Vector3f {
	float x;
	float y;
	float z;
};

//4�̃x�N�g��
struct Vector4f {
	float x;
	float y;
	float z;
	float w;
};
//4�̃J���[
struct Color4 {
	float r;
	float g;
	float b;
	float a;
};
//4�̔���
struct Reflection4 {
	Color4 diffuse;
	Color4 ambient;
	Color4 emission;
	Color4 specular;
};
//UV���W
struct UV {
	float u;//u�l
	float v;//v�l
};
//�|���S���f�[�^
struct Triangle {
	Vector3f TriVer;
	Vector3f TriNor;
	UV TriUV;
};
//�|���S���f�[�^
struct Quadrangle {
	Vector3f QuadVer;
	Vector3f QuadNor;
	UV QuadUV;
};
//�}�e���A���\����
struct MATERIAL {
	string MaterialName;//�}�e���A����
	Reflection4 MaterialColor;//����
	float Shininess;//shininess
	string TextureName;//�e�N�X�`����
	int TexNo;//�e�N�X�`��NO.
	vector <Triangle> Tridata;//�O�p�ʃf�[�^
	vector <Quadrangle> Quaddata;//�l�p�ʃf�[�^
};
//���f���N���X
class MODEL {
protected:
	vector <MATERIAL> Material;//�}�e���A��
	vector <TEXTURE*> TexData;//�e�N�X�`���f�[�^
	vector<GLuint> TexID;//�e�N�X�`��ID
	GLuint TexID2;//����p
	TEXTURE* tex;
	btTriangleMesh* Mesh;
public:
	MODEL();
	MODEL(char* FileName, float size);//�R���X�g���N�^
	bool XFILE_Load(char* FileName, float size);//���[�h
	void Draw();
	void Register();
};

#endif