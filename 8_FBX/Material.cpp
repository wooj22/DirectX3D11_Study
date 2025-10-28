#include "Material.h"
#include "../WinBase/Helper.h"

void Material::CreateSRV()
{
	// Texture load
	// Shader Resource View create
	if (textureFlags & TEX_DIFFUSE)
	{
		CreateTextureFromFile(D3D::device.Get(), (directory + diffuse_filename).c_str(), &diffuseSRV);
	}

	if (textureFlags & TEX_NORMAL)
	{
		CreateTextureFromFile(D3D::device.Get(), (directory + normal_filename).c_str(), &normalSRV);
	}

	if (textureFlags & TEX_SPECULAR)
	{
		CreateTextureFromFile(D3D::device.Get(), (directory + specular_filename).c_str(), &specualrSRV);
	}

	if (textureFlags & TEX_EMISSIVE)
	{
		CreateTextureFromFile(D3D::device.Get(), (directory + emissive_filename).c_str(), &emissiveSRV);
	}
}

void Material::SetDirectoryPath(wstring& path)
{
	directory = path;
}