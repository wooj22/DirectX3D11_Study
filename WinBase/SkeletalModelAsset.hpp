#include "SkeletalSubMesh.h"
#include "Skeleton.hpp"
#include "Material.h"
#include "AnimationClip.h"

// SkeletalMeshModel의 읽기 전용 공유 Asset

class SkeletalModelAsset
{
public:
    // sub mesh, material, animation
    vector<SkeletalSubMesh> subMeshes;
    Skeleton skeleton;
    vector<Material> materials;
    vector<AnimationClip> animationClips;
};