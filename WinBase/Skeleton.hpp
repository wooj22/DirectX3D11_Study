#pragma once
#include <string>
#include <vector>
#include <map>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;
using std::string;
using std::vector;
using std::map;

/*
   [Bone]
   Skeletal Model을 이루는 각각의 Bone
*/
struct Bone
{
    string name;                // bone name
    int    parentIndex;         // skeleton에 저장된 bone vector의 부모 bone index

    Matrix offsetMatrix = Matrix::Identity;      // bind pose inverse transform
    Matrix bindMatrix   = Matrix::Identity;      // bind pose transform
};


/*
    [Skeleton]
    하나의 Skeletal Model이 가지는 Bone의 모음
*/
struct Skeleton
{
    vector<Bone> bones;
    map<string, int> nameToIndex;

    // bone name -> index
    int GetBoneIndex(const string& name) const
    {
        auto it = nameToIndex.find(name);
        if (it != nameToIndex.end())
            return it->second;
        return -1;
    }
};
