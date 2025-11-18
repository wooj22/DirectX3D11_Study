#include "AssetManager.h"
#include "ModelLoader.h"
#include "StaticModel.h"
#include "RigidModel.h"
#include "SkeletalModel.h"

void AssetManager::LoadStaticModelAsset(StaticModel* model, string filePath)
{
    string key = filePath;
    auto it = asset_staticmodel.find(key);

    // asset이 있을 경우
    if (it != asset_staticmodel.end())
    {
        // 인스턴스가 살아있다면 asset 넘겨주기
        if (!it->second.expired()) {
            model->model = it->second.lock();
            return;
        }
        else asset_staticmodel.erase(it);
    }

    // asset이 없을 경우 생성
    ModelLoader::LoadStaticMesh(model, filePath);
    asset_staticmodel[filePath] = model->model;
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
            model->model = it->second.lock();
            model->submesh_localMatrices.resize(model->model->subMeshes.size());
            model->submesh_modelMatrices.resize(model->model->subMeshes.size());
            return;
        }
        else asset_rigidmodel.erase(it);
    }

    // asset이 없을 경우 생성
    ModelLoader::LoadRigidMesh(model, filePath);
    asset_rigidmodel[filePath] = model->model;
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
            model->model = it->second.lock();
            model->localMatrix.resize(model->model->skeleton.bones.size());
            model->poseMatrix.resize(model->model->skeleton.bones.size());
            return;
        }
        else asset_skeletalmodel.erase(it);
    }

    // asset이 없을 경우 생성
    ModelLoader::LoadSkeletalMesh(model, filePath);
    asset_skeletalmodel[filePath] = model->model;
}