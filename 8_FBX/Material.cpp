#include "Material.h"
#include "../WinBase/Helper.h"

void Material::Create()
{
	// Texture load
	// Shader Resource View create
	if (!diffuse_filename.empty()) 
	{
		CreateTextureFromFile(D3D::device.Get(), (directory + diffuse_filename).c_str(), &diffuseSRV);
	}

	if (!normal_filename.empty())
	{
		CreateTextureFromFile(D3D::device.Get(), (directory + normal_filename).c_str(), &normalSRV);
	}

	if (!specular_filename.empty())
	{
		CreateTextureFromFile(D3D::device.Get(), (directory + specular_filename).c_str(), &specualrSRV);
	}

	if (!emissive_filename.empty())
	{
		CreateTextureFromFile(D3D::device.Get(), (directory + emissive_filename).c_str(), &emissiveSRV);
	}

}

void Material::SetDirectoryPath(wstring& path)
{
	directory = path;
}