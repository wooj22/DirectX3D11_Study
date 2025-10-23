#include "ModelLoder.h"
#include "StaticMesh.h"
#include "StaticSubMesh.h"
#include "Material.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;
using namespace DirectX;

// static member init
Importer ModelLoder::importer;
unsigned int ModelLoder::importFlags =
    aiProcess_Triangulate |  // vertex 삼각형 으로 출력
    aiProcess_GenNormals |                              // normal 
    aiProcess_GenUVCoords |                             // uv
    aiProcess_CalcTangentSpace |                        // tangent vector
    aiProcess_ConvertToLeftHanded |                     // DX용 왼손좌표계 변환
    aiProcess_PreTransformVertices;                     // 노드의 변환행렬을 적용한 버텍스 생성한다.  *StaticMesh로 처리할때만


// Model Load
StaticMesh* ModelLoder::LoadStaticMesh(const string& modelPath)
{
	const aiScene* scene = importer.ReadFile(modelPath, importFlags);

    StaticMesh* staticMesh = new StaticMesh();
	ProcessNode(scene->mRootNode, scene, staticMesh);

    return staticMesh;
}


// Node 순회
void ModelLoder::ProcessNode(aiNode* node, const aiScene* scene, StaticMesh* staticMesh)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* mesh = scene->mMeshes[meshIndex];
        ProcessMesh(mesh, scene, staticMesh);
    }

    // 자식 노드 재귀 탐색
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, staticMesh);
    }
}


// Mesh 처리
void ModelLoder::ProcessMesh(aiMesh* mesh, const aiScene* scene, StaticMesh* staticMesh)
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
    ProcessMaterial(material, scene, staticMesh);

    // buffer create
    submesh.Create();

    // push sub mesh
    staticMesh->subMeshes.push_back(move(submesh));
}


// Material 처리
void ModelLoder::ProcessMaterial(aiMaterial* material, const aiScene* scene, StaticMesh* staticMesh)
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
    mat.Create();

    // push
    staticMesh->materials.push_back(move(mat));
}

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