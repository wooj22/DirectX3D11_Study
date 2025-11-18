#pragma once
#include <string>
#include <sstream>
#include <iomanip>
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
*  메모리 사용량을 체크해주는 클래스입니다.
* 
* - VRAM Used : GPU가 실제로 사용 중인 VRAM
* - VRAM Budget : 프로세스가 안정적으로 쓸 수 있는 VRAM 한도
* - DRAM Used : CPU RAM 사용량
* - PageFile Used : SSD/HDD로 스왑된 메모리 (PageFile 사용량 증가 현상이 보이면 DRAM이 부족하다는 의미로 성능 최악상태임)
*/

class MemoryDebugger
{
private: 
    ComPtr<IDXGIDevice3>  dxgiDevice;       // trim
    ComPtr<IDXGIAdapter3> dxgiAdapter;      // memory cheak

    std::wstring memoryReport;
    std::wstring FormatBytes(UINT64 bytes);

public:
    void Init(); 
    void CheakMemoryUsage();
    void Trim();
    const std::wstring& GetMemoryUsageWstring() const { return memoryReport; }
};

