#include "StaticSubMesh.h"
#include "Material.h"

// StaticMeshModel의 읽기 전용 공유 Asset

class StaticModelAsset
{
public:
    vector<StaticSubMesh> subMeshes;
    vector<Material> materials;
};