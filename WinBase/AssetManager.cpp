#include "AssetManager.h"
#include "ModelLoader.h"
#include "StaticModel.h"
#include "RigidModel.h"
#include "SkeletalModel.h"

void DebugPrintSkeletalVertex(const SkeletalSubMesh& mesh)
{
    if (mesh.vertices.empty())
    {
        OutputDebugStringA("❌ Skeletal Mesh has NO vertices!\n");
        return;
    }

    const BoneWeightVertex& v = mesh.vertices[0];

    char buf[512];

    sprintf_s(buf, sizeof(buf),
        "===== Skeletal Vertex Debug =====\n"
        "pos        = %.4f, %.4f, %.4f\n"
        "normal     = %.4f, %.4f, %.4f\n"
        "tangent    = %.4f, %.4f, %.4f\n"
        "bitangent  = %.4f, %.4f, %.4f\n"
        "texcoord   = %.4f, %.4f\n"
        "boneIndex  = %d, %d, %d, %d\n"
        "boneWeight = %.4f, %.4f, %.4f, %.4f\n"
        "==================================\n",
        v.position.x, v.position.y, v.position.z,
        v.normal.x, v.normal.y, v.normal.z,
        v.tangent.x, v.tangent.y, v.tangent.z,
        v.bitangent.x, v.bitangent.y, v.bitangent.z,
        v.texcoord.x, v.texcoord.y,
        v.boneIndices[0], v.boneIndices[1], v.boneIndices[2], v.boneIndices[3],
        v.boneWeights[0], v.boneWeights[1], v.boneWeights[2], v.boneWeights[3]
    );

    OutputDebugStringA(buf);
}

void AssetManager::LoadStaticModelAsset(StaticModel* model, string filePath)
{
    string key = filePath;
    auto it = asset_staticmodel.find(key);

    // asset이 있을 경우
    if (it != asset_staticmodel.end())
    {
        // 인스턴스가 살아있다면 asset 넘겨주기
        if (!it->second.expired()) {
            model->model_data = it->second.lock();
            return;
        }
        else asset_staticmodel.erase(it);
    }

    // asset이 없을 경우 생성
    ModelLoader::LoadStaticMesh(model, filePath);
    asset_staticmodel[filePath] = model->model_data;
}


void AssetManager::LoadRigidModelAsset(RigidModel* model, string filePath)
{
    string key = filePath;
    auto it = asset_rigidmodel.find(key);

    // asset이 있을 경우
    if (it != asset_rigidmodel.end())
    {
        // 인스턴스가 살아있다면 asset 넘겨주기
        if (!it->second.expired()) {
            model->model_data = it->second.lock();
            model->submesh_localMatrices.resize(model->model_data->subMeshes.size());
            model->submesh_modelMatrices.resize(model->model_data->subMeshes.size());
            return;
        }
        else asset_rigidmodel.erase(it);
    }

    // asset이 없을 경우 생성
    ModelLoader::LoadRigidMesh(model, filePath);
    asset_rigidmodel[filePath] = model->model_data;
}


void AssetManager::LoadSkeletalModelAsset(SkeletalModel* model, string filePath)
{
    string key = filePath;
    auto it = asset_skeletalmodel.find(key);

    // asset이 있을 경우
    if (it != asset_skeletalmodel.end())
    {
        // 인스턴스가 살아있다면 asset 넘겨주기
        if (!it->second.expired()) {
            model->model_data = it->second.lock();
            model->localMatrix.resize(model->model_data->skeleton.bones.size());
            model->poseMatrix.resize(model->model_data->skeleton.bones.size());
            return;
        }
        else asset_skeletalmodel.erase(it);
    }

    // asset이 없을 경우 생성
    ModelLoader::LoadSkeletalMesh(model, filePath);

    // 디버그 출력
    for (auto& subMesh : model->model_data->subMeshes)
    {
        DebugPrintSkeletalVertex(subMesh);
    }
    OutputDebugStringA(">>> LoadSkeletalModelAsset CALLED\n");
    asset_skeletalmodel[filePath] = model->model_data;
}