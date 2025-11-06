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
   
*/
struct Bone
{
    string name;
    int parentIndex;
        
    Matrix offsetMatrix = Matrix::Identity;      // bind pose inverse transform
    Matrix bindMatrix =  Matrix::Identity;       // bind
    Matrix localMatrix = Matrix::Identity;       // animation
    Matrix worldMatrix = Matrix::Identity;       // parent * local
};


/* 
    [Skeleton]
    하나의 스켈레탈 메시가 가지는 Bone을 관리하는 구조체
*/
struct Skeleton
{
    vector<Bone> bones;
    map<string, int> nameToIndex;     // name -> index 매핑
    int boneCount;
    
    // bone name -> index
    int GetBoneIndex(const string& name) const
    {
        auto it = nameToIndex.find(name);
        if (it != nameToIndex.end())
            return it->second;
        return -1;
    }

    // world matrix udpate
    void UpdateBoneWorld()
    {
        for (int i = 0; i < bones.size(); ++i)
        {
            if (bones[i].parentIndex == -1)
                bones[i].worldMatrix = bones[i].localMatrix;
            else
                bones[i].worldMatrix = bones[i].localMatrix * bones[bones[i].parentIndex].worldMatrix;
        }
    }
};
