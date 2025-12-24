#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace Assimp;
using std::string;
using std::wstring;

class StaticModel;
class StaticSubMesh;
class RigidModel;
class RigidSubMesh;
class SkeletalModel;
class SkeletalSubMesh;
class Material;
class Skeleton;

/*
* [ Model Loder ]
* - Static Mesh, Skeletal Mesh 데이터를 로드하고 파싱하여 저장하는 클래스
*/
class ModelLoader
{
public:
    static Importer importer;
    static unsigned int staticImportFlags;
    static unsigned int skeletalImportFlags;

    // model loading
    static void LoadStaticMesh(StaticModel* model, const string& modelPath);
    static void LoadRigidMesh(RigidModel* model, const string& modelPath);
    static void LoadSkeletalMesh(SkeletalModel* model, const string& modelPath);

    // flag setting
    static void SetImportFlags(unsigned int flags) { staticImportFlags = flags; }

private:
    // static mesh
    static void ProcessStaticNode(aiNode* node, const aiScene* scene, StaticModel* staticMesh);
    static void ProcessStaticMesh(aiMesh* mesh, const aiScene* scene, StaticSubMesh* subMesh);

    // rigid skeletal mesh
    static void ProcessRigidNode(aiNode* node, const aiScene* scene, RigidModel* rigidMesh, int parentIndex);
    static void ProcessRigidMesh(aiMesh* mesh, const aiScene* scene, RigidSubMesh* subMesh);
    static void ProcessRigidAnimation(const aiScene* scene, RigidModel* rigidMesh);

    // skinned skeletal mesh
    static void ProcessSkeleton(const aiScene* scene, SkeletalModel* skeletalMesh);
    static void ProcessSkeletalNode(aiNode* node, const aiScene* scene, SkeletalModel* skeletalMesh, int parentIndex);
    static void ProcessSkeletalMesh(aiMesh* mesh, const aiScene* scene, SkeletalSubMesh* subMesh, Skeleton& skeleton);
    static void ProcessSkeletalAnimation(const aiScene* scene, SkeletalModel* skeletalMesh);

    static void SaveEmbeddedTextureIfExists(const aiScene* scene, const string& directory, const string& filename);
    static void ProcessMaterial(aiMaterial* aiMaterial, const aiScene* scene, Material* material);

    // material texture debug
    static const wchar_t* TextureTypeToString(aiTextureType type);
    static void DebugMaterialTextures(aiMaterial* aiMaterial);

};

