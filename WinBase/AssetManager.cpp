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

        // 인스턴스가 소멸된상태라면 map에서 제거
        else asset_staticmodel.erase(it);
    }

    // asset이 없을 경우
    // TODO :: ModelLoader asset 생성 구조로 바꾸기
    //model->model = TODO
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
            return;
        }

        // 인스턴스가 소멸된상태라면 map에서 제거
        else asset_rigidmodel.erase(it);
    }

    // asset이 없을 경우
    // TODO :: ModelLoader asset 생성 구조로 바꾸기
    //model->model = TODO
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
            return;
        }

        // 인스턴스가 소멸된상태라면 map에서 제거
        else asset_skeletalmodel.erase(it);
    }

    // asset이 없을 경우
    // TODO :: ModelLoader asset 생성 구조로 바꾸기
    //model->model = TODO
}