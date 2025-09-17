#include "Camera.h"
#include <DirectXMath.h>
using namespace DirectX;
namespace Math = DirectX::SimpleMath;

Camera::Camera()
{
	Reset();
}

Vector3 Camera::GetForward()
{
	return -world.Forward();
}

Vector3 Camera::GetRight()
{
	return world.Right();
}

void Camera::Reset()
{
	world = Matrix::Identity;
	rotation = Vector3(0.0f, 0.0f, 0.0f);
	position = Vector3(0.0f, 0.0f, -5.0f);
}

void Camera::Update()
{
	OnInputProcess();

	if (inputVector.Length() > 0.0f)
	{
		position += inputVector * moveSpeed * Time::GetDeltaTime();
		inputVector = Vector3(0.0f, 0.0f, 0.0f);
	}

	world = Matrix::CreateFromYawPitchRoll(rotation) *
		Matrix::CreateTranslation(position);
}

void Camera::GetViewMatrix(Matrix& out)
{
	Vector3 eye = world.Translation();
	Vector3 target = world.Translation() + GetForward();
	Vector3 up = world.Up();
	out = XMMatrixLookAtLH(eye, target, up);
}

void Camera::AddInputVector(const Math::Vector3& input)
{
	inputVector += input;
	inputVector.Normalize();
}

void Camera::AddPitch(float value)
{
	rotation.x += value;
	if (rotation.x > XM_PI)
	{
		rotation.x -= XM_2PI;
	}
	else if (rotation.x < -XM_PI)
	{
		rotation.x += XM_2PI;
	}
}

void Camera::AddYaw(float value)
{
	rotation.y += value;
	if (rotation.y > XM_PI)
	{
		rotation.y -= XM_2PI;
	}
	else if (rotation.y < -XM_PI)
	{
		rotation.y += XM_2PI;
	}
}


void Camera::OnInputProcess()
{
	Vector3 forward = GetForward();
	Vector3 right = GetRight();

	if (Input::GetKey('R'))
	{
		Reset();
	}

	if (Input::GetKey('W'))
	{
		AddInputVector(forward);
	}
	else if (Input::GetKey('S'))
	{
		AddInputVector(-forward);
	}

	if (Input::GetKey('A'))
	{
		AddInputVector(-right);
	}
	else if (Input::GetKey('D'))
	{
		AddInputVector(right);
	}

	if (Input::GetKey('E'))
	{
		AddInputVector(world.Up());
	}
	else if (Input::GetKey('Q'))
	{
		AddInputVector(-world.Up());
	}

	// 오른쪽 버튼 누르면 상대 모드, 아니면 절대 모드
	Input::SetMouseMode(Input::GetMouseButton(1)); // 1 = 오른쪽 버튼

	// 상대 모드라면 마우스 이동량으로 회전 적용
	POINT delta = Input::GetMouseDelta();
	if (Input::GetMouseButton(1)) // 상대 모드일 때만
	{
		Vector3 rotationDelta = Vector3(float(delta.y), float(delta.x), 0.f) * rotationSpeed;
		AddPitch(rotationDelta.x);  // Y축 회전
		AddYaw(rotationDelta.y);    // X축 회전
	}
}

