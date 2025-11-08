#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace Assimp;
using std::string;
using std::wstring;

class StaticMesh;
class StaticSubMesh;
class RigidMesh;
class RigidSubMesh;
class SkeletalMesh;
class SkeletalSubMesh;
class Material;
class Skeleton;

/*
* [ Model Loder ]
* - Static Mesh, Skeletal Mesh를 생성하여 포인터를 반환해주는 3D 모델 임포터
*/
class ModelLoader
{
public:
	static Importer importer;
	static unsigned int staticImportFlags;
	static unsigned int skeletalImportFlags;

	// model loading
	static StaticMesh*	 LoadStaticMesh(const string& modelPath);
	static RigidMesh*    LoadRigidMesh(const string& modelPath);
	static SkeletalMesh* LoadSkeletalMesh(const string& modelPath);

	// flag setting
    static void SetImportFlags(unsigned int flags) { staticImportFlags = flags; }

private:
	// static mesh
	static void ProcessStaticNode(aiNode* node, const aiScene* scene, StaticMesh* staticMesh);
	static void ProcessStaticMesh(aiMesh* mesh, const aiScene* scene, StaticSubMesh* subMesh);
	
	// rigid skeletal mesh
	static void ProcessRigidNode(aiNode* node, const aiScene* scene, RigidMesh* rigidMesh, int parentIndex);
	static void ProcessRigidMesh(aiMesh* mesh, const aiScene* scene, RigidSubMesh* subMesh);
	static void ProcessRigidAnimation(const aiScene* scene, RigidMesh* rigidMesh);

    // skinned skeletal mesh
    static void ProcessSkeleton(const aiScene* scene, SkeletalMesh* skeletalMesh);
    static void ProcessSkeletalNode(aiNode* node, const aiScene* scene, SkeletalMesh* skeletalMesh, int parentIndex);
    static void ProcessSkeletalMesh(aiMesh* mesh, const aiScene* scene, SkeletalSubMesh* subMesh, Skeleton& skeleton);
    static void ProcessSkeletalAnimation(const aiScene* scene, SkeletalMesh* skeletalMesh);

	static void SaveEmbeddedTextureIfExists(const aiScene* scene, const string& directory, const string& filename);
	static void ProcessMaterial(aiMaterial* aiMaterial, const aiScene* scene, Material* material);
};

