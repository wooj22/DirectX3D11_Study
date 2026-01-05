#include "DirectionalShadowCamera.h"
#include "Camera.h"


void DirectionalShadowCamera::Udate(Camera& camera, const Vector3 lightDir, const ShadowOrthoDesc& desc)
{
    Vector3 sceneCenter = camera.position + camera.GetForward() * desc.lookPointDist;
    Vector3 lightPos = sceneCenter - lightDir * desc.shadowLightDist;
    view = XMMatrixLookAtLH(lightPos, sceneCenter, Vector3::Up);
    projection = XMMatrixOrthographicLH(desc.shadowWidth, desc.shadowHeight, desc.shadowNear, desc.shadowFar);
}