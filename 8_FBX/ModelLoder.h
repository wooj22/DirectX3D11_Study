#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace Assimp;
using std::string;
using std::wstring;

class StaticMesh;
class RigidMesh;
class Material;

/*
* [ Model Loder ]
* - Static Mesh를 생성하여 포인터를 반환해주는 3D 모델 임포터
*/
class ModelLoder
{
public:
	static Importer importer;
	static unsigned int staticImportFlags;
	static unsigned int skeletalImportFlags;

	// model loading
	static StaticMesh* LoadStaticMesh(const string& modelPath);
	static RigidMesh* LoadRigidMesh(const string& modelPath);

	// flag setting
    static void SetImportFlags(unsigned int flags) { staticImportFlags = flags; }

private:
	static void ProcessStaticNode(aiNode* node, const aiScene* scene, StaticMesh* staticMesh);
	static void ProcessStaticMesh(aiMesh* mesh, const aiScene* scene, StaticMesh* staticMesh);
	static void ProcessStaticMaterial(aiMaterial* material, const aiScene* scene, StaticMesh* staticMesh);
	
	static void ProcessRigidNode(aiNode* node, const aiScene* scene, RigidMesh* rigidMesh, int parentIndex);
	static void ProcessRigidMesh(aiMesh* mesh, const aiScene* scene, RigidMesh* rigidMesh, aiNode* node, int parentIndex);
	static void ProcessRigidMaterial(aiMaterial* material, const aiScene* scene, RigidMesh* rigidMesh);
	static void ProcessRigidAnimation(const aiScene* scene, RigidMesh* rigidMesh);
	
	static Material ProcessMaterial(aiMaterial* material, const aiScene* scene);
	static void SaveEmbeddedTextureIfExists(const aiScene* scene, const string& directory, const string& filename);
};

