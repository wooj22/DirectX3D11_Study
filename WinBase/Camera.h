#pragma once
#include <directxtk/SimpleMath.h>
#include "Input.h"
#include "Time.h"
using namespace DirectX::SimpleMath;

class Camera
{
private:
    Matrix view;
    Matrix projection;

public:
	Camera();
	Vector3 rotation;
	Vector3 position;

	float FovY = 45.0f;
	float Near = 0.1f;
	float Far = 5000.0f;

	Matrix world;
	Vector3 inputVector;

	float moveSpeed = 20.f;
	float rotationSpeed = 0.004f;

	Vector3 GetForward();
	Vector3 GetRight();

    Matrix GetView() const { return view; }
    Matrix GetProjection() const { return projection; }

	void Reset();
	void Update(Vector2 screenSize);
	void GetViewMatrix(Matrix& out);
	void GetProjectionMatrix(Matrix& out, Vector2 screenSize);
	void AddInputVector(const Vector3& input);
	void SetSpeed(float speed) { moveSpeed = speed; }
	void AddPitch(float value);
	void AddYaw(float value);

	virtual void OnInputProcess();
};

