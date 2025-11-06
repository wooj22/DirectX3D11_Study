#include "ModelLoder.h"
#include "StaticMesh.h"
#include "RigidMesh.h"
#include "SkeletalMesh.h"
#include "Material.h"
#include "AnimationClip.h"
#include "Bone.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;
using namespace DirectX;

// static member init
Importer ModelLoder::importer;
unsigned int ModelLoder::staticImportFlags =
aiProcess_Triangulate |                             // vertex 삼각형 으로 출력
aiProcess_GenNormals |                              // normal 
aiProcess_GenUVCoords |                             // uv
aiProcess_CalcTangentSpace |                        // tangent vector
aiProcess_ConvertToLeftHanded |                     // DX용 왼손좌표계 변환
aiProcess_PreTransformVertices;                     // 노드의 변환행렬을 적용한 버텍스 생성한다.  *StaticMesh로 처리할때만

unsigned int ModelLoder::skeletalImportFlags =
aiProcess_Triangulate |                             // vertex 삼각형 으로 출력
aiProcess_GenNormals |                              // normal 
aiProcess_GenUVCoords |                             // uv
aiProcess_CalcTangentSpace |                        // tangent vector
aiProcess_LimitBoneWeights |                        // 하나의 정점이 영향을 받는 Bone의 개수를 최대 4개로 제한
aiProcess_ConvertToLeftHanded;                      // DX용 왼손좌표계 변환


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

// Skeletal Mesh Load
SkeletalMesh* ModelLoder::LoadSkeletalMesh(const string& modelPath)
{
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
    const aiScene* scene = importer.ReadFile(modelPath, skeletalImportFlags);

    SkeletalMesh* skeletalMesh = new SkeletalMesh();
    ProcessSkeletalNode(scene->mRootNode, scene, skeletalMesh, -1);
    ProcessSkeletalAnimation(scene, skeletalMesh);

    return skeletalMesh;
}


/*-------------------  Static Mesh ---------------------------*/
// Node 순회
void ModelLoder::ProcessStaticNode(aiNode* node, const aiScene* scene, StaticMesh* staticMesh)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        StaticSubMesh subMesh;
        Material material;

        // submesh
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* aiMesh = scene->mMeshes[meshIndex];
        ProcessStaticMesh(aiMesh, scene, &subMesh);      // aiMesh -> subMesh data save
        subMesh.CreateBuffer();                          // vertex, index buffer create
        staticMesh->subMeshes.push_back(move(subMesh));

        // material
        subMesh.materialIndex = aiMesh->mMaterialIndex;
        aiMaterial* aiMaterial = scene->mMaterials[aiMesh->mMaterialIndex];
        ProcessMaterial(aiMaterial, scene, &material);    // aiMaterial -> material data save
        material.CreateSRV();                             // shader resource view create
        staticMesh->materials.push_back(move(material));
        
    }

    // child node
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessStaticNode(node->mChildren[i], scene, staticMesh);
    }
}

// Mesh
void ModelLoder::ProcessStaticMesh(aiMesh* mesh, const aiScene* scene, StaticSubMesh* submesh)
{
    // vertex
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex v;
        v.position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        v.tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        v.bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        v.texcoord = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

        submesh->vertices.push_back(move(v));
    }

    // index
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            submesh->indices.push_back(move(face.mIndices[j]));
    }
}


/*-------------------  Rigid Skeletal Mesh ---------------------------*/
// Node 순회 (초기 parent index = -1)
void ModelLoder::ProcessRigidNode(aiNode* node, const aiScene* scene, RigidMesh* rigidMesh, int parentIndex)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        RigidSubMesh subMesh;
        Material material;

        // submesh
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* aiMesh = scene->mMeshes[meshIndex];
        subMesh.nodeName = node->mName.C_Str();
        subMesh.bindMatrix = XMMatrixTranspose(XMLoadFloat4x4((XMFLOAT4X4*)&node->mTransformation));
        subMesh.parentIndex = parentIndex - 1;
        ProcessRigidMesh(aiMesh, scene, &subMesh);       // aiMesh -> subMesh data save
        subMesh.CreateBuffer();                          // vertex, index buffer create
        rigidMesh->subMeshes.push_back(move(subMesh));

        // material
        subMesh.materialIndex = aiMesh->mMaterialIndex;
        aiMaterial* aiMaterial = scene->mMaterials[aiMesh->mMaterialIndex];
        ProcessMaterial(aiMaterial, scene, &material);    // aiMaterial -> material data save
        material.CreateSRV();                             // shader resource view create
        rigidMesh->materials.push_back(move(material));
    }

    // child node
    int myIndex = rigidMesh->subMeshes.size();
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessRigidNode(node->mChildren[i], scene, rigidMesh, myIndex);
    }
}

// Mesh
void ModelLoder::ProcessRigidMesh(aiMesh* mesh, const aiScene* scene, RigidSubMesh* subMesh)
{
    // vertex
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex v;
        v.position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        v.tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        v.bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        v.texcoord = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

        subMesh->vertices.push_back(move(v));
    }

    // index
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            subMesh->indices.push_back(move(face.mIndices[j]));
    }
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
			//OutputDebugStringA((nodeAnim.nodeName + "\n").c_str());

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



/*------------------- Skinned Skeletal Mesh ---------------------------*/

void ModelLoder::ProcessSkeletalNode(aiNode* node, const aiScene* scene, SkeletalMesh* rigidMesh, int parentIndex)
{
    OutputDebugStringA("Node Name : ");
    OutputDebugStringA(node->mName.C_Str());
    OutputDebugStringA("\n");
    OutputDebugStringA(to_string(node->mNumMeshes).c_str());
    OutputDebugStringA("\n");

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        SkeletalSubMesh subMesh;
        Material material;

        // submesh
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* aiMesh = scene->mMeshes[meshIndex];
        subMesh.nodeName = node->mName.C_Str();
        subMesh.bindMatrix = XMMatrixTranspose(XMLoadFloat4x4((XMFLOAT4X4*)&node->mTransformation));
        subMesh.parentIndex = parentIndex - 1;

        // Node Name debug
        /*OutputDebugStringA("Node Name : ");
        OutputDebugStringA(node->mName.C_Str());
        OutputDebugStringA("\n");
        OutputDebugStringA("Sub Mesh Node Name : ");
        OutputDebugStringA(subMesh.nodeName.c_str());
        OutputDebugStringA("\n");*/

        // vertex, index
        ProcessSkeletalMesh(aiMesh, scene, &subMesh);    // aiMesh -> subMesh data save
        subMesh.CreateBuffer();                          // vertex, index buffer create

        // bone
        subMesh.boneCount = aiMesh->mNumBones;
        for (int j = 0; j < subMesh.boneCount; j++)
        {
            aiBone* aiBone = aiMesh->mBones[j];
            Bone bone;
            bone.name = aiBone->mName.C_Str();
            //OutputDebugStringA(bone.name.c_str());
            bone.offsetMatrix = XMMatrixTranspose(XMLoadFloat4x4((XMFLOAT4X4*)&aiBone->mOffsetMatrix));
            subMesh.bones.push_back(move(bone));

            OutputDebugStringA("Bone Name : ");
            OutputDebugStringA(aiBone->mName.C_Str());
            OutputDebugStringA("\n");
        }
        rigidMesh->subMeshes.push_back(move(subMesh));

        // material
        subMesh.materialIndex = aiMesh->mMaterialIndex;
        aiMaterial* aiMaterial = scene->mMaterials[aiMesh->mMaterialIndex];
        ProcessMaterial(aiMaterial, scene, &material);    // aiMaterial -> material data save
        material.CreateSRV();                             // shader resource view create
        rigidMesh->materials.push_back(move(material));
    }

    // child node
    int myIndex = rigidMesh->subMeshes.size();
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessSkeletalNode(node->mChildren[i], scene, rigidMesh, myIndex);
    }
}

void ModelLoder::ProcessSkeletalMesh(aiMesh* mesh, const aiScene* scene, SkeletalSubMesh* subMesh)
{
    // vertex
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        BoneWeightVertex v;
        v.position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        v.tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        v.bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        v.texcoord = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        
        // bone, weight
        for (int j = 0; j < mesh->mNumBones; j++)
        {
            aiBone* aiBone = mesh->mBones[j];
            for (unsigned int k = 0; k < aiBone->mNumWeights; ++k)
            {
                if (aiBone->mWeights[k].mVertexId == i)
                {
                    v.AddBoneData(j, aiBone->mWeights[k].mWeight);
                    break;
                }
            }
        }

        subMesh->vertices.push_back(move(v));
    }

    // index
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            subMesh->indices.push_back(move(face.mIndices[j]));
    }
}

void ModelLoder::ProcessSkeletalAnimation(const aiScene* scene, SkeletalMesh* rigidMesh)
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
            //OutputDebugStringA((nodeAnim.nodeName + "\n").c_str());

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


/*-------------------  Material & Embedded Texture ---------------------------*/
// 내장된 텍스처 저장
void ModelLoder::SaveEmbeddedTextureIfExists(const aiScene* scene, const string& directory, const string& filename)
{
    const aiTexture* embedded = scene->GetEmbeddedTexture(filename.c_str());

    if (embedded && embedded->mHeight == 0)
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

// Material
void ModelLoder::ProcessMaterial(aiMaterial* aiMaterial, const aiScene* scene, Material* material)
{
    aiString filepath;
    string directory = "../Resource/";

    // file name save
    // diffuse
    if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &filepath) == AI_SUCCESS)
    {
        std::string filename = fs::path(filepath.C_Str()).filename().string();
        SaveEmbeddedTextureIfExists(scene, directory, filename);

        material->diffuse_filename = fs::path(filepath.C_Str()).filename().wstring();
        material->textureFlags |= TEX_DIFFUSE;
    }

    // normal
    if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &filepath) == AI_SUCCESS)
    {
        std::string filename = fs::path(filepath.C_Str()).filename().string();
        SaveEmbeddedTextureIfExists(scene, directory, filename);

        material->normal_filename = fs::path(filepath.C_Str()).filename().wstring();
        material->textureFlags |= TEX_NORMAL;
    }

    // specular
    if (aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &filepath) == AI_SUCCESS)
    {
        std::string filename = fs::path(filepath.C_Str()).filename().string();
        SaveEmbeddedTextureIfExists(scene, directory, filename);

        material->specular_filename = fs::path(filepath.C_Str()).filename().wstring();
        material->textureFlags |= TEX_SPECULAR;
    }

    // emissive
    if (aiMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &filepath) == AI_SUCCESS)
    {
        std::string filename = fs::path(filepath.C_Str()).filename().string();
        SaveEmbeddedTextureIfExists(scene, directory, filename);

        material->emissive_filename = fs::path(filepath.C_Str()).filename().wstring();
        material->textureFlags |= TEX_EMISSIVE;
    }
}