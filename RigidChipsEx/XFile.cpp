#include "stb_image.h"
#include "XFile.h"

extern btDynamicsWorld* g_pDynamicsWorld;

static struct UV vec2d;
static struct Triangle Tri;
static struct Quadrangle Quad;
static struct MATERIAL mtl;
static struct Vector4f vec4d;
static struct Vector3f vec3d;

Vector3f & operator*(Vector3f &v, float size) {
	v.x *= size;
	v.y *= size;
	v.z *= size;
	return v;
}

MODEL::MODEL() {
}
MODEL::MODEL(char* FileName, float size) {
	XFILE_Load(FileName, size);
}
//描画
void MODEL::Draw() {
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	for (int i = 0; i<(signed)Material.size(); i++) {
		glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (const GLfloat *)&Material[i].MaterialColor.ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (const GLfloat *)&Material[i].MaterialColor.diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (const GLfloat *)&Material[i].MaterialColor.specular);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Material[i].Shininess);
		if (Material[i].TexNo>0) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, TexID[Material[i].TexNo - 1]);
		}
		else {
			glDisable(GL_TEXTURE_2D);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		if (Material[i].Tridata.size()>1) {
			glVertexPointer(3, GL_FLOAT, sizeof(Tri), &Material[i].Tridata[0].TriVer.x);
			glNormalPointer(GL_FLOAT, sizeof(Tri), &Material[i].Tridata[0].TriNor.x);
			if (Material[i].TexNo>0)glTexCoordPointer(2, GL_FLOAT, sizeof(Tri), &Material[i].Tridata[0].TriUV.u);
			glDrawArrays(GL_TRIANGLES, 0, Material[i].Tridata.size());
		}
		if (Material[i].Quaddata.size()>1) {
			glVertexPointer(3, GL_FLOAT, sizeof(Quad), &Material[i].Quaddata[0].QuadVer.x);
			glNormalPointer(GL_FLOAT, sizeof(Quad), &Material[i].Quaddata[0].QuadNor.x);
			if (Material[i].TexNo>0)glTexCoordPointer(2, GL_FLOAT, sizeof(Quad), &Material[i].Quaddata[0].QuadUV.u);
			glDrawArrays(GL_QUADS, 0, Material[i].Quaddata.size());
		}
		glPopMatrix();
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

void MODEL::Register() {
	Mesh = new btTriangleMesh();

	for (int i = 0; i<(signed)Material.size(); i++) {
		if (Material[i].Tridata.size()>1) {
			for (int j = 0; j < Material[i].Tridata.size(); j+=3) {
				btVector3 vertex[3];
				for (int k = 0; k < 3; k++) vertex[k] = btVector3(Material[i].Tridata[j + k].TriVer.x, Material[i].Tridata[j + k].TriVer.y, Material[i].Tridata[j + k].TriVer.z);
				Mesh->addTriangle(vertex[0], vertex[1], vertex[2]);
			}
		}
		if (Material[i].Quaddata.size()>1) {
			for (int j = 0; j < Material[i].Quaddata.size(); j+=4) {
				btVector3 vertex[4];
				for (int k = 0; k < 4; k++) vertex[k] = btVector3(Material[i].Quaddata[j + k].QuadVer.x, Material[i].Quaddata[j + k].QuadVer.y, Material[i].Quaddata[j + k].QuadVer.z);
				Mesh->addTriangle(vertex[0], vertex[1], vertex[2]);
				Mesh->addTriangle(vertex[0], vertex[3], vertex[2]);
			}
		}
	}

	btCollisionShape* collisionShapeTerrain = new btBvhTriangleMeshShape(Mesh, true);

	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

	btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructionInfo(0.0f, motionState, collisionShapeTerrain, btVector3(0, 0, 0));
	btRigidBody* rigidBodyTerrain = new btRigidBody(rigidBodyConstructionInfo);
	rigidBodyTerrain->setFriction(btScalar(0.9));

	g_pDynamicsWorld->addRigidBody(rigidBodyTerrain, 1, 3);
}

//Xファイル読み込み
bool MODEL::XFILE_Load(char* FileName, float size) {
	vector <Vector3f> Vertex;//頂点
	vector <Vector3f> Normal;//法線
	vector <UV> uv;//UV
	vector <int> VertexIndex;
	vector <int> NormalIndex;
	vector <int> MaterialIndex;
	vector <int> FaceIndex;

	char key[255];
	char buf[255];
	//Xファイルを開いて内容を読み込む
	FILE* fp = NULL;
	fopen_s(&fp, FileName, "rt");

	int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
	int Count = 0;
	bool flag = false;
	string str = "";

	//読み込み 
	fseek(fp, SEEK_SET, 0);
	while (!feof(fp))
	{
		//キーワード 読み込み
		ZeroMemory(key, sizeof(key));
		fscanf_s(fp, "%s ", key, sizeof(key));

		//テンプレート読み飛ばし
		if (strcmp(key, "template") == 0) {
			while (strcmp(key, "}")) {
				fscanf_s(fp, "%s ", key, sizeof(key));
			}
			continue;
		}

		//マテリアル読み込み
		if (strcmp(key, "Material") == 0)
		{
			if (flag)Material.push_back(mtl);
			flag = true;
			fgets(buf, sizeof(buf), fp);//直後の行にあると推定　改行する
										//ディフューズ
			fscanf_s(fp, "%f;%f;%f;%f;;", &vec4d.x, &vec4d.y, &vec4d.z, &vec4d.w);
			mtl.MaterialColor.diffuse = (const Color4 &)vec4d;
			//SHININESS  
			fscanf_s(fp, "%f;", &mtl.Shininess);
			//スペキュラー 
			fscanf_s(fp, "%f;%f;%f;;", &vec4d.x, &vec4d.y, &vec4d.z);
			mtl.MaterialColor.specular = (const Color4 &)vec4d;
			//エミッシブ 
			fscanf_s(fp, "%f;%f;%f;;", &vec4d.x, &vec4d.y, &vec4d.z);
			mtl.MaterialColor.ambient = (const Color4 &)vec4d;

			//テクスチャー
			fscanf_s(fp, "%s ", key, sizeof(key));
			if (strcmp(key, "TextureFilename") == 0)
			{
				fgets(buf, sizeof(buf), fp);//直後の行にあると推定　改行する
											//map_Kd　テクスチャー 
				fscanf_s(fp, "%s", buf, 255);
				mtl.TextureName = buf;
				str = mtl.TextureName;
				mtl.TextureName = str.substr(str.find_first_of('\"', 0) + 1, str.find_last_of('\"', 255) - 1);
				//テクスチャを作成
				TexData.push_back(tex);
				TexData[TexData.size() - 1] = new TEXTURE(mtl.TextureName.c_str());;
				mtl.TexNo = TexData.size();
				TexID.push_back(TexID2);
				glGenTextures(1, (GLuint *)&TexID[TexData.size() - 1]);
				glBindTexture(GL_TEXTURE_2D, TexID[TexData.size() - 1]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				glEnable(GL_TEXTURE_2D);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TexData[TexData.size() - 1]->Width, TexData[TexData.size() - 1]->Height,
					0, GL_RGBA, GL_UNSIGNED_BYTE, TexData[TexData.size() - 1]->image);
				glDisable(GL_TEXTURE_2D);
			}
		}
		//頂点 読み込み
		if (strcmp(key, "Mesh") == 0)
		{
			fgets(buf, sizeof(buf), fp);//データは2行下にあると推定　改行する
			fgets(buf, sizeof(buf), fp);
			Count = atoi(buf);
			for (int i = 0; i<Count; i++)
			{
				fscanf_s(fp, "%f;%f;%f;,", &vec3d.x, &vec3d.y, &vec3d.z);
				Vertex.push_back(vec3d*(float)size);
			}
			//頂点インデックス読み込み  
			fgets(buf, sizeof(buf), fp);//データは2行下にあると推定　改行する
			fgets(buf, sizeof(buf), fp);
			while (strchr(buf, ';') == NULL) { fgets(buf, sizeof(buf), fp); }//空行対策
			Count = atoi(buf);
			for (int i = 0; i<Count; i++)
			{
				int dammy = 0;
				fgets(buf, sizeof(buf), fp);
				str = buf;
				string::size_type first = str.find_first_not_of(" ");
				string::size_type index = str.find("3;");
				if (index - first == 0) {
					sscanf_s(buf, "%d;%d,%d,%d;,", &dammy, &v1, &v2, &v3);
					VertexIndex.push_back(v1);
					VertexIndex.push_back(v2);
					VertexIndex.push_back(v3);
				}
				if ((index == -1) || (index - first>1)) {
					sscanf_s(buf, "%d;%d,%d,%d,%d;,", &dammy, &v1, &v2, &v3, &v4);
					VertexIndex.push_back(v1);
					VertexIndex.push_back(v2);
					VertexIndex.push_back(v3);
					VertexIndex.push_back(v4);
				}
				FaceIndex.push_back(dammy);
			}
		}

		//法線 読み込み
		if (strcmp(key, "MeshNormals") == 0)
		{
			fgets(buf, sizeof(buf), fp);//データは2行下にあると推定　改行する
			fgets(buf, sizeof(buf), fp);
			Count = atoi(buf);
			for (int i = 0; i<Count; i++)
			{
				fscanf_s(fp, "%f;%f;%f;,", &vec3d.x, &vec3d.y, &vec3d.z);
				Normal.push_back(vec3d);
			}
			//法線インデックス読み込み  
			fgets(buf, sizeof(buf), fp);//データは2行下にあると推定　改行する
			fgets(buf, sizeof(buf), fp);
			while (strchr(buf, ';') == NULL) { fgets(buf, sizeof(buf), fp); }//空行対策
			Count = atoi(buf);
			for (int i = 0; i<Count; i++)
			{
				int dammy = 0;
				fgets(buf, sizeof(buf), fp);
				str = buf;
				string::size_type first = str.find_first_not_of(" ");
				string::size_type index = str.find("3;");
				if (index - first == 0) {
					sscanf_s(buf, "%d;%d,%d,%d;,", &dammy, &v1, &v2, &v3);
					NormalIndex.push_back(v1);
					NormalIndex.push_back(v2);
					NormalIndex.push_back(v3);
				}
				if ((index == -1) || (index - first>1)) {
					sscanf_s(buf, "%d;%d,%d,%d,%d;,", &dammy, &v1, &v2, &v3, &v4);
					NormalIndex.push_back(v1);
					NormalIndex.push_back(v2);
					NormalIndex.push_back(v3);
					NormalIndex.push_back(v4);
				}
			}
		}

		//テクスチャー座標 読み込み
		if (strcmp(key, "MeshTextureCoords") == 0)
		{
			fgets(buf, sizeof(buf), fp);//データは2行下にあると推定　改行する
			fgets(buf, sizeof(buf), fp);
			while (strchr(buf, ';') == NULL) { fgets(buf, sizeof(buf), fp); }//空行対策
			Count = atoi(buf);
			for (int i = 0; i<Count; i++)
			{
				fscanf_s(fp, "%f;%f;,", &vec2d.u, &vec2d.v);
				uv.push_back(vec2d);
			}
		}
		//マテリアルリスト
		if (strcmp(key, "MeshMaterialList") == 0)
		{
			fgets(buf, sizeof(buf), fp);//空改行
			fgets(buf, sizeof(buf), fp);//マテリアル数
			fgets(buf, sizeof(buf), fp);//リスト要素数
			Count = atoi(buf);
			for (int i = 0; i<Count; i++)
			{
				fgets(buf, sizeof(buf), fp);
				int test = atoi(buf);
				MaterialIndex.push_back(test);
			}
		}
	}
	if (flag)Material.push_back(mtl);

	fclose(fp);

	Count = 0;
	//マテリアル毎のデータを作成
	for (int i = 0; i<(signed)MaterialIndex.size(); i++) {
		if (FaceIndex[i] == 3) {
			for (int j = 0; j<3; j++) {
				Tri.TriVer = Vertex[VertexIndex[Count + j]];
				Tri.TriNor = Normal[NormalIndex[Count + j]];
				Tri.TriUV = uv[VertexIndex[Count + j]];
				Material[MaterialIndex[i]].Tridata.push_back(Tri);
			}
			Count += 3;
		}
		else {
			for (int j = 0; j<4; j++) {
				Quad.QuadVer = Vertex[VertexIndex[Count + j]];
				Quad.QuadNor = Normal[NormalIndex[Count + j]];
				Quad.QuadUV = uv[VertexIndex[Count + j]];
				Material[MaterialIndex[i]].Quaddata.push_back(Quad);
			}
			Count += 4;
		}
	}

	Vertex.clear();
	Normal.clear();
	uv.clear();
	VertexIndex.clear();
	NormalIndex.clear();
	MaterialIndex.clear();
	FaceIndex.clear();

	return true;
}

TEXTURE::TEXTURE() {
}

TEXTURE::TEXTURE(const char* FileName) {
	LOAD_PNG(FileName);
}

void TEXTURE::LOAD_PNG(const char* FileName) {
	int bpp;
	image = stbi_load(FileName, (int *)&Width, (int *)&Height, &bpp, 4);
}