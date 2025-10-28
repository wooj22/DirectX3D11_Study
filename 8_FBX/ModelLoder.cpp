#include "ModelLoder.h"
#include "StaticMesh.h"
#include "RigidMesh.h"
#include "Material.h"
#include "AnimationClip.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;
using namespace DirectX;

// static member init
Importer ModelLoder::importer;
unsigned int ModelLoder::staticImportFlags =
    aiProcess_Triangulate |  // vertex 삼각형 으로 출력
    aiProcess_GenNormals |                              // normal 
    aiProcess_GenUVCoords |                             // uv
    aiProcess_CalcTangentSpace |                        // tangent vector
    aiProcess_ConvertToLeftHanded |                     // DX용 왼손좌표계 변환
    aiProcess_PreTransformVertices;                     // 노드의 변환행렬을 적용한 버텍스 생성한다.  *StaticMesh로 처리할때만

unsigned int ModelLoder::skeletalImportFlags =
aiProcess_Triangulate |  // vertex 삼각형 으로 출력
aiProcess_GenNormals |                              // normal 
aiProcess_GenUVCoords |                             // uv
aiProcess_CalcTangentSpace |                        // tangent vector
aiProcess_ConvertToLeftHanded;                     // DX용 왼손좌표계 변환



// Static Mash Load
StaticMesh* ModelLoder::LoadStaticMesh(const string& modelPath)
{
	const aiScene* scene = importer.ReadFile(modelPath, staticImportFlags);

    StaticMesh* staticMesh = new StaticMesh();
	ProcessStaticNode(scene->mRootNode, scene, staticMesh);

    return staticMesh;
}

// Rigid Mesh Load
RigidMesh* ModelLoder::LoadRigidMesh(const string& modelPath)
{
	const aiScene* scene = importer.ReadFile(modelPath, skeletalImportFlags);

	RigidMesh* rigidMesh = new RigidMesh();
	ProcessRigidNode(scene->mRootNode, scene, rigidMesh, -1);
	ProcessRigidAnimation(scene, rigidMesh);

	return rigidMesh;
}

/*-------------------  Static Mesh ---------------------------*/
// Node 순회
void ModelLoder::ProcessStaticNode(aiNode* node, const aiScene* scene, StaticMesh* staticMesh)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* mesh = scene->mMeshes[meshIndex];
        ProcessStaticMesh(mesh, scene, staticMesh);
    }

    // 자식 노드 재귀 탐색
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessStaticNode(node->mChildren[i], scene, staticMesh);
    }
}


// Mesh
void ModelLoder::ProcessStaticMesh(aiMesh* mesh, const aiScene* scene, StaticMesh* staticMesh)
{
    StaticSubMesh submesh;

    // vertex
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex v;
        v.position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        v.tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        v.bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        v.texcoord = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

        submesh.vertices.push_back(move(v));
    }

    // index
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            submesh.indices.push_back(move(face.mIndices[j]));
    }

    // material
    submesh.materialIndex = mesh->mMaterialIndex;
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    ProcessStaticMaterial(material, scene, staticMesh);

    // buffer create
    submesh.Create();

    // push sub mesh
    staticMesh->subMeshes.push_back(move(submesh));
}


// Material
void ModelLoder::ProcessStaticMaterial(aiMaterial* material, const aiScene* scene, StaticMesh* staticMesh)
{
	Material mat = ProcessMaterial(material, scene);
    staticMesh->materials.push_back(move(mat));
}


/*-------------------  Rigid Mesh ---------------------------*/
// Node 순회 (초기 parent index = -1)
void ModelLoder::ProcessRigidNode(aiNode* node, const aiScene* scene, RigidMesh* rigidMesh, int parentIndex)
{
    string currentNodeName = node->mName.C_Str();

	// SubMesh 처리
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* mesh = scene->mMeshes[meshIndex];
        ProcessRigidMesh(mesh, scene, rigidMesh, currentNodeName, parentIndex);
    }

    // 자식 노드 재귀 탐색
    int myIndex = rigidMesh->subMeshes.size();
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessRigidNode(node->mChildren[i], scene, rigidMesh, myIndex);
    }
}

// Mesh
void ModelLoder::ProcessRigidMesh(aiMesh* mesh, const aiScene* scene, RigidMesh* rigidMesh, const string& nodeName, int parentIndex)
{
    // node name
    RigidSubMesh submesh;
	submesh.nodeName = nodeName;
	submesh.parentIndex = parentIndex -1;
    
	OutputDebugStringA((nodeName + "\n").c_str());
	OutputDebugStringA(("Parent Index : " + std::to_string(parentIndex-1) + "\n").c_str());

    // vertex
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex v;
        v.position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        v.tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        v.bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        v.texcoord = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

        submesh.vertices.push_back(move(v));
    }

    // index
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            submesh.indices.push_back(move(face.mIndices[j]));
    }

    // material
    submesh.materialIndex = mesh->mMaterialIndex;
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    ProcessRigidMaterial(material, scene, rigidMesh);

    // buffer create
    submesh.CreateBuffer();

    // push sub mesh
    rigidMesh->subMeshes.push_back(move(submesh));
}

// Material
void ModelLoder::ProcessRigidMaterial(aiMaterial* material, const aiScene* scene, RigidMesh* rigidMesh)
{
    Material mat = ProcessMaterial(material, scene);
    rigidMesh->materials.push_back(move(mat));
}

// Animation
void ModelLoder::ProcessRigidAnimation(const aiScene* scene, RigidMesh* rigidMesh)
{
    // 애니메이션이 없다면 return
    if (scene->mNumAnimations == 0) return;

    for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
    {
        aiAnimation* aiAnim = scene->mAnimations[i];
        AnimationClip clip;

        // animation clip info
		clip.name = aiAnim->mName.C_Str();
		clip.duration = static_cast<float>(aiAnim->mDuration / aiAnim->mTicksPerSecond);
		clip.ticksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond);

		// node animation
		for (unsigned int j = 0; j < aiAnim->mNumChannels; ++j)
		{
			aiNodeAnim* aiNodeAnim = aiAnim->mChannels[j];
			NodeAnimation nodeAnim;
			nodeAnim.nodeName = aiNodeAnim->mNodeName.C_Str();

			// keyframe
			// position
			for (unsigned int k = 0; k < aiNodeAnim->mNumPositionKeys; ++k)
			{
                VectorKey posKey;
				posKey.time = static_cast<float>(aiNodeAnim->mPositionKeys[k].mTime / aiAnim->mTicksPerSecond);
				posKey.value = Vector3(
					aiNodeAnim->mPositionKeys[k].mValue.x,
					aiNodeAnim->mPositionKeys[k].mValue.y,
					aiNodeAnim->mPositionKeys[k].mValue.z);
				nodeAnim.positionKeys.push_back(move(posKey));
			}

            // rotation
            for (unsigned int k = 0; k < aiNodeAnim->mNumRotationKeys; ++k)
            {
                QuatKey rotKey;
                rotKey.time = static_cast<float>(aiNodeAnim->mRotationKeys[k].mTime / aiAnim->mTicksPerSecond);
                rotKey.value = Quaternion(
                    aiNodeAnim->mRotationKeys[k].mValue.x,
                    aiNodeAnim->mRotationKeys[k].mValue.y,
                    aiNodeAnim->mRotationKeys[k].mValue.z,
                    aiNodeAnim->mRotationKeys[k].mValue.w);
                nodeAnim.rotationKeys.push_back(move(rotKey));
            }

            // scale
            for (unsigned int k = 0; k < aiNodeAnim->mNumScalingKeys; ++k)
            {
                VectorKey scaleKey;
                scaleKey.time = static_cast<float>(aiNodeAnim->mScalingKeys[k].mTime / aiAnim->mTicksPerSecond);
                scaleKey.value = Vector3(
                    aiNodeAnim->mScalingKeys[k].mValue.x,
                    aiNodeAnim->mScalingKeys[k].mValue.y,
                    aiNodeAnim->mScalingKeys[k].mValue.z);
                nodeAnim.scaleKeys.push_back(move(scaleKey));
            }

			// node animation push
			clip.nodeAnimations.push_back(move(nodeAnim));
		}

		// animation clip push
		rigidMesh->animationClips.push_back(move(clip));
    }
}

// Material 처리
Material ModelLoder::ProcessMaterial(aiMaterial* material, const aiScene* scene)
{
    Material mat;
    aiString filepath;
    string directory = "../Resource/";

    // file name save
    // diffuse
    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &filepath) == AI_SUCCESS)
    {
        std::string filename = fs::path(filepath.C_Str()).filename().string();
        SaveEmbeddedTextureIfExists(scene, directory, filename);

        mat.diffuse_filename = fs::path(filepath.C_Str()).filename().wstring();
        mat.textureFlags |= TEX_DIFFUSE;
    }

    // normal
    if (material->GetTexture(aiTextureType_NORMALS, 0, &filepath) == AI_SUCCESS)
    {
        std::string filename = fs::path(filepath.C_Str()).filename().string();
        SaveEmbeddedTextureIfExists(scene, directory, filename);

        mat.normal_filename = fs::path(filepath.C_Str()).filename().wstring();
        mat.textureFlags |= TEX_NORMAL;
    }

    // specular
    if (material->GetTexture(aiTextureType_SPECULAR, 0, &filepath) == AI_SUCCESS)
    {
        std::string filename = fs::path(filepath.C_Str()).filename().string();
        SaveEmbeddedTextureIfExists(scene, directory, filename);

        mat.specular_filename = fs::path(filepath.C_Str()).filename().wstring();
        mat.textureFlags |= TEX_SPECULAR;
    }

    // emissive
    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &filepath) == AI_SUCCESS)
    {
        std::string filename = fs::path(filepath.C_Str()).filename().string();
        SaveEmbeddedTextureIfExists(scene, directory, filename);

        mat.emissive_filename = fs::path(filepath.C_Str()).filename().wstring();
        mat.textureFlags |= TEX_EMISSIVE;
    }

    // texture create
    mat.CreateSRV();
    return move(mat);
}

// 내장된 텍스처 저장
void ModelLoder::SaveEmbeddedTextureIfExists(const aiScene* scene, const string& directory, const string& filename)
{
    const aiTexture* embedded = scene->GetEmbeddedTexture(filename.c_str());

    if (embedded&& embedded->mHeight >= 0)
    {
        std::string tmpPath = directory + filename;
        std::ofstream file(tmpPath, std::ios::binary);

        if (file.is_open())
        {
            file.write(reinterpret_cast<const char*>(embedded->pcData), embedded->mWidth);
            file.close();
        }
    }
}