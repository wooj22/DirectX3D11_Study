#pragma once
#include <directxtk/SimpleMath.h>
#include "Input.h"
#include "Time.h"
using namespace DirectX::SimpleMath;

class Camera
{
public:
	Camera();
	Vector3 rotation;
	Vector3 position;

	float FovY = 90.0f;
	float Near = 0.01f;
	float Far = 100.0f;

	Matrix world;
	Vector3 inputVector;

	float moveSpeed = 20.f;
	float rotationSpeed = 0.004f;

	Vector3 GetForward();
	Vector3 GetRight();

	void Reset();
	void Update();
	void GetViewMatrix(Matrix& out);
	void AddInputVector(const Vector3& input);
	void SetSpeed(float speed) { moveSpeed = speed; }
	void AddPitch(float value);
	void AddYaw(float value);

	virtual void OnInputProcess();
};

