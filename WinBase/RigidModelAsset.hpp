#include "RigidSubMesh.h"
#include "Material.h"
#include "AnimationClip.h"

// RigidMeshModel의 읽기 전용  공유 Asset

class RigidModelAsset
{
public:
    vector<RigidSubMesh> subMeshes;
    vector<Material> materials;
    vector<AnimationClip> animationClips;
};