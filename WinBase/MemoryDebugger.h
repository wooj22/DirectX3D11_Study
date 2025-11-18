#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <wincodec.h>
#include <dxgi1_6.h>
#include <psapi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib,"windowscodecs.lib")
using namespace Microsoft::WRL;


/*
* [ Memory Debugger ]
*  
*/

class MemoryDebugger
{
private:
    // d3d memory    
    ComPtr<IDXGIDevice3> dxgiDevice;
    ComPtr<IDXGIAdapter3> dxgiAdapter;

public:
    // singleton instance getter
    static MemoryDebugger& Instance()
    {
        static MemoryDebugger instance;
        return instance;
    }
};

