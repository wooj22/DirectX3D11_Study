#include "ModelLoader.h"
#include "StaticModel.h"
#include "RigidModel.h"
#include "SkeletalModel.h"
#include "Material.h"
#include "AnimationClip.h"
#include "Skeleton.hpp"
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;
using namespace DirectX;

// static member init
Importer ModelLoader::importer;
unsigned int ModelLoader::staticImportFlags =
aiProcess_Triangulate |                             // vertex 삼각형 으로 출력
aiProcess_GenNormals |                              // normal 
aiProcess_GenUVCoords |                             // uv
aiProcess_CalcTangentSpace |                        // tangent vector
aiProcess_ConvertToLeftHanded |                     // DX용 왼손좌표계 변환
aiProcess_PreTransformVertices;                     // 노드의 변환행렬을 적용한 버텍스 생성한다.  *StaticMesh로 처리할때만

unsigned int ModelLoader::skeletalImportFlags =
aiProcess_Triangulate |                             // vertex 삼각형 으로 출력
aiProcess_GenNormals |                              // normal 
aiProcess_GenUVCoords |                             // uv
aiProcess_CalcTangentSpace |                        // tangent vector
aiProcess_LimitBoneWeights |                        // 하나의 정점이 영향을 받는 Bone의 개수를 최대 4개로 제한
aiProcess_ConvertToLeftHanded;                      // DX용 왼손좌표계 변환

// Static Mash Load
StaticModel* ModelLoader::LoadStaticMesh(const string& modelPath)
{
    const aiScene* scene = importer.ReadFile(modelPath, staticImportFlags);

    StaticModel* staticMesh = new StaticModel();
    ProcessStaticNode(scene->mRootNode, scene, staticMesh);

    return staticMesh;
}

// Rigid Mesh Load
RigidModel* ModelLoader::LoadRigidMesh(const string& modelPath)
{
    const aiScene* scene = importer.ReadFile(modelPath, skeletalImportFlags);

    RigidModel* rigidMesh = new RigidModel();
    ProcessRigidNode(scene->mRootNode, scene, rigidMesh, -1);
    ProcessRigidAnimation(scene, rigidMesh);

    return rigidMesh;
}

// Skeletal Mesh Load
SkeletalModel* ModelLoader::LoadSkeletalMesh(const string& modelPath)
{
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
    const aiScene* scene = importer.ReadFile(modelPath, skeletalImportFlags);

    SkeletalModel* skeletalMesh = new SkeletalModel();
    ProcessSkeleton(scene, skeletalMesh);                               // bone
    ProcessSkeletalNode(scene->mRootNode, scene, skeletalMesh, -1);     // node(bone, mesh)
    ProcessSkeletalAnimation(scene, skeletalMesh);

    return skeletalMesh;
}


/*-------------------  Static Mesh ---------------------------*/
// Node 순회
void ModelLoader::ProcessStaticNode(aiNode* node, const aiScene* scene, StaticModel* staticMesh)
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
void ModelLoader::ProcessStaticMesh(aiMesh* mesh, const aiScene* scene, StaticSubMesh* submesh)
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
void ModelLoader::ProcessRigidNode(aiNode* node, const aiScene* scene, RigidModel* rigidMesh, int parentIndex)
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
void ModelLoader::ProcessRigidMesh(aiMesh* mesh, const aiScene* scene, RigidSubMesh* subMesh)
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
void ModelLoader::ProcessRigidAnimation(const aiScene* scene, RigidModel* rigidMesh)
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



/*------------------- Skinned Skeletal Mesh ---------------------------*/

void ModelLoader::ProcessSkeleton(const aiScene* scene, SkeletalModel* skeletalMesh)
{
    skeletalMesh->skeleton.bones.clear();

    // scene의 mesh 전체를 순회하며 aiBone 저장
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            // bone 중복 skip
            std::string name = mesh->mBones[b]->mName.C_Str();
            if (skeletalMesh->skeleton.nameToIndex.find(name) != skeletalMesh->skeleton.nameToIndex.end())
                continue;

            // skeleton에 bone 추가
            // parent index는 node 순회때 저장
            Bone bone;
            bone.name = name;
            bone.offsetMatrix = XMMatrixTranspose(XMLoadFloat4x4((XMFLOAT4X4*)&mesh->mBones[b]->mOffsetMatrix));
            skeletalMesh->skeleton.bones.push_back(std::move(bone));
            skeletalMesh->skeleton.nameToIndex[name] = skeletalMesh->skeleton.bones.size() - 1;
        }
    }
}

void ModelLoader::ProcessSkeletalNode(aiNode* node, const aiScene* scene, SkeletalModel* skeletalMesh, int parentIndex)
{
    // Skeleton
    int boneIndex = skeletalMesh->skeleton.GetBoneIndex(node->mName.C_Str());
    if (boneIndex != -1)
    {
        Bone& bone = skeletalMesh->skeleton.bones[boneIndex];
        bone.bindMatrix = XMMatrixTranspose(XMLoadFloat4x4((XMFLOAT4X4*)&node->mTransformation));
        bone.parentIndex = parentIndex;
    }

    // SubMesh
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        SkeletalSubMesh subMesh;
        Material material;

        // submesh
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* aiMesh = scene->mMeshes[meshIndex];
        subMesh.nodeName = node->mName.C_Str();

        // vertex, index
        ProcessSkeletalMesh(aiMesh, scene, &subMesh, skeletalMesh->skeleton);    // aiMesh -> subMesh data save
        subMesh.CreateBuffer();                          // vertex, index buffer create
        skeletalMesh->subMeshes.push_back(move(subMesh));

        // material
        subMesh.materialIndex = aiMesh->mMaterialIndex;
        aiMaterial* aiMaterial = scene->mMaterials[aiMesh->mMaterialIndex];
        ProcessMaterial(aiMaterial, scene, &material);    // aiMaterial -> material data save
        material.CreateSRV();                             // shader resource view create
        skeletalMesh->materials.push_back(move(material));
    }

    // child node
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessSkeletalNode(node->mChildren[i], scene, skeletalMesh, boneIndex);
    }
}

void ModelLoader::ProcessSkeletalMesh(aiMesh* mesh, const aiScene* scene, SkeletalSubMesh* subMesh, Skeleton& skeleton)
{
    // Vertex
    subMesh->vertices.resize(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        BoneWeightVertex& v = subMesh->vertices[i];
        v.position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        v.tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        v.bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        v.texcoord = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    }

    // bone index lookup table
    // mesh->mNumBones에 맞게 skeleton의 bones 배열 index 저장
    std::vector<int> boneToSkeletonIndex(mesh->mNumBones);
    for (int j = 0; j < mesh->mNumBones; ++j)
    {
        boneToSkeletonIndex[j] = skeleton.GetBoneIndex(mesh->mBones[j]->mName.C_Str());
    }

    // vertex - bone index, weight
    for (int j = 0; j < mesh->mNumBones; ++j)
    {
        int skeletonIndex = boneToSkeletonIndex[j];
        if (skeletonIndex == -1) continue;

        aiBone* aiBone = mesh->mBones[j];
        for (unsigned int k = 0; k < aiBone->mNumWeights; ++k)
        {
            int vertexId = aiBone->mWeights[k].mVertexId;
            float weight = aiBone->mWeights[k].mWeight;
            subMesh->vertices[vertexId].AddBoneData(skeletonIndex, weight);
        }
    }

    // Index
    subMesh->indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            subMesh->indices.push_back(face.mIndices[j]);
        }
    }
}

void ModelLoader::ProcessSkeletalAnimation(const aiScene* scene, SkeletalModel* skeletalMesh)
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
        skeletalMesh->animationClips.push_back(move(clip));
    }
}


/*-------------------  Material & Embedded Texture ---------------------------*/
// 내장된 텍스처 저장
void ModelLoader::SaveEmbeddedTextureIfExists(const aiScene* scene, const string& directory, const string& filename)
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
void ModelLoader::ProcessMaterial(aiMaterial* aiMaterial, const aiScene* scene, Material* material)
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