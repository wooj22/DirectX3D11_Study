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
   Mesh 단위로 생성되는 Bone의 data를 담는 구조체
*/
struct Bone
{
    string name;                // Bone 이름
    Matrix offsetMatrix;        // 바인드 포즈 역행렬 (aiBone::mOffsetMatrix)
};


/* 매트릭스 팔레드 안쓸거라 일단 x
    [Skeleton Info]
    하나의 스켈레탈 메시가 가지는 Bone을 관리하는 구조체
    Bone 배열을 저장하고, 매핑 테이블을 제공한다.
*/
//struct SkeletonInfo
//{
//    vector<Bone> bones;
//    map<string, int> boneMappingTable;      // boneName -> boneIndex
//    map<string, int> meshMappingTable;      // meshName -> boneInde
// 
//    // mapping funcs
//};
