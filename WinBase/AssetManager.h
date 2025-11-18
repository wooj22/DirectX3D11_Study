#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>

#include <d3d11.h>
#include <wrl.h>
#include <wincodec.h>
#include <dxgi1_6.h>
#include <psapi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib,"windowscodecs.lib")

using std::unordered_map;
using std::vector;
using std::string;
using std::weak_ptr;
using std::shared_ptr;
using namespace Microsoft::WRL;

class StaticModel;
class RigidModel;
class SkeletalModel;
class StaticModelAsset;
class RigidModelAsset;
class SkeletalModelAsset;

/*
* [ AssetManager Manager ]
*  공유 자원을 중복으로 메모리에 로드하지 않도록 해시테이블로 관리하는 클래스
*  key값으로 weak_ptr을 내부에서 관리를 하며, 외부 요청이 들어왔을 경우 shared_ptr을 넘겨준다.
*  아직 없는 에셋이라면 ModelLoader를 통해 새로 생성한다.
* 
*  - StaticMeshAsset, RigidMeshAsset, SkeletalMeshAsset
*/

class AssetManager
{
public:
    // singleton instance getter
    static AssetManager& Instance()
    {
        static AssetManager instance;
        return instance;
    }

private:
    // d3d memory    
    ComPtr<IDXGIDevice3> dxgiDevice;
    ComPtr<IDXGIAdapter3> dxgiAdapter;

    // asseet data
    unordered_map<string, weak_ptr<StaticModelAsset>> asset_staticmodel;
    unordered_map<string, weak_ptr<RigidModelAsset>> asset_rigidmodel;
    unordered_map<string, weak_ptr<SkeletalModelAsset>> asset_skeletalmodel;

public:
    // asset managing
    void LoadStaticModelAsset(StaticModel* model, string filePath);
    void LoadRigidModelAsset(RigidModel* model, string filePath);
    void LoadSkeletalModelAsset(SkeletalModel* model, string filePath);


private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;
};

