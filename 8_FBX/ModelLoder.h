#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace Assimp;
using std::string;
using std::wstring;

class StaticMesh;
class StaticSubMesh;
class Material;

/*
* [ Model Loder ]
* - Static Mesh를 생성하여 포인터를 반환해주는 3D 모델 임포터
*/
class ModelLoder
{
public:
	static Importer importer;
	static unsigned int importFlags;

	// Model Load
	static StaticMesh* LoadStaticMesh(const string& modelPath);
    static void SetImportFlags(unsigned int flags) { importFlags = flags; }

private:
	static void ProcessNode(aiNode* node, const aiScene* scene, StaticMesh* staticMesh);
	static void ProcessMesh(aiMesh* mesh, const aiScene* scene, StaticMesh* staticMesh);
	static void ProcessMaterial(aiMaterial* material, const aiScene* scene, StaticMesh* staticMesh);
	static void SaveEmbeddedTextureIfExists(const aiScene* scene, const string& directory, const string& filename);
};

