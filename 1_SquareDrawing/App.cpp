#include "App.h"
#include "../WinBase/Helper.h"

#include <directxtk/simplemath.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE 1

App::App()
{
	
}

App::~App()
{

}

bool App::OnInit()
{
	return true;
}

void App::OnUninit()
{

}

void App::OnUpdate()
{

}

void App::OnRender()
{
	
}