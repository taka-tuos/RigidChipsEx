#include <GL/glut.h>
#include <vector>

#include <btBulletDynamicsCommon.h>

using namespace std;

#ifndef __XFILE_H__
#define __XFILE_H__

class TEXTURE {
public:
	TEXTURE();
	TEXTURE(const char* FileName);//コンストラクタ
	void LOAD_PNG(const char* FileName);//PNG読み込み
	unsigned char* image;//イメージポインタ
	unsigned int Width, Height;//画像サイズ
};

//3つのベクトル
struct Vector3f {
	float x;
	float y;
	float z;
};

//4つのベクトル
struct Vector4f {
	float x;
	float y;
	float z;
	float w;
};
//4つのカラー
struct Color4 {
	float r;
	float g;
	float b;
	float a;
};
//4つの反射
struct Reflection4 {
	Color4 diffuse;
	Color4 ambient;
	Color4 emission;
	Color4 specular;
};
//UV座標
struct UV {
	float u;//u値
	float v;//v値
};
//ポリゴンデータ
struct Triangle {
	Vector3f TriVer;
	Vector3f TriNor;
	UV TriUV;
};
//ポリゴンデータ
struct Quadrangle {
	Vector3f QuadVer;
	Vector3f QuadNor;
	UV QuadUV;
};
//マテリアル構造体
struct MATERIAL {
	string MaterialName;//マテリアル名
	Reflection4 MaterialColor;//反射
	float Shininess;//shininess
	string TextureName;//テクスチャ名
	int TexNo;//テクスチャNO.
	vector <Triangle> Tridata;//三角面データ
	vector <Quadrangle> Quaddata;//四角面データ
};
//モデルクラス
class MODEL {
protected:
	vector <MATERIAL> Material;//マテリアル
	vector <TEXTURE*> TexData;//テクスチャデータ
	vector<GLuint> TexID;//テクスチャID
	GLuint TexID2;//代入用
	TEXTURE* tex;
	btTriangleMesh* Mesh;
public:
	MODEL();
	MODEL(char* FileName, float size);//コンストラクタ
	bool XFILE_Load(char* FileName, float size);//ロード
	void Draw();
	void Register();
};

#endif