#include "MemoryDebugger.h"
#include "D3D.h"

std::wstring MemoryDebugger::FormatBytes(UINT64 bytes)
{
    constexpr double KB = 1024.0;
    constexpr double MB = KB * 1024.0;
    constexpr double GB = MB * 1024.0;

    std::wostringstream oss;
    oss << std::fixed << std::setprecision(2);

    if (bytes >= GB)
        oss << (bytes / GB) << L" GB";
    else if (bytes >= MB)
        oss << (bytes / MB) << L" MB";
    else if (bytes >= KB)
        oss << (bytes / KB) << L" KB";
    else
        oss << bytes << L" B";

    return oss.str();
}

void MemoryDebugger::Init()
{
    HRESULT hr;

    // IDXGIDevice 인터페이스 얻기
    ComPtr<IDXGIDevice> dxgiDeviceBase;
    hr = D3D::device ->QueryInterface(dxgiDeviceBase.GetAddressOf());
    if (FAILED(hr)) {

    }

    // IDXGIDeviceBase -> IDXGIDevice3 변환
    hr = dxgiDeviceBase.As(&dxgiDevice);
    if (FAILED(hr))
    {

    }

    // IDXGIAdapter 얻기
    ComPtr<IDXGIAdapter> dxgiAdapterBase;
    hr = dxgiDevice->GetAdapter(dxgiAdapterBase.GetAddressOf());
    if (FAILED(hr))
    {

    }

    // IDXGIAdapter -> IDXGIAdapter3 변환
    hr = dxgiAdapterBase.As(&dxgiAdapter);
    if (FAILED(hr)) {

    }
}

void MemoryDebugger::CheakMemoryUsage()
{
    if (!dxgiAdapter)
    {
        memoryReport = L"[MemoryDebugger] Adapter Not Initialized.\n";
        return;
    }

    DXGI_QUERY_VIDEO_MEMORY_INFO memInfo = {};
    HRESULT hr = dxgiAdapter->QueryVideoMemoryInfo(
        0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);

    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc;
    pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);

    std::wostringstream oss;
    oss << L"====== Memory Debugger ======\n";

    if (SUCCEEDED(hr))
    {
        oss << L"VRAM Used     : "
            << FormatBytes(memInfo.CurrentUsage) << L"\n";
        oss << L"VRAM Budget   : "
            << FormatBytes(memInfo.Budget) << L"\n";
    }
    else
    {
        oss << L"VRAM 조회 실패!\n";
    }

    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
    {
        oss << L"DRAM Used     : "
            << FormatBytes(pmc.WorkingSetSize) << L"\n";

        UINT64 pageUsed = pmc.PagefileUsage - pmc.WorkingSetSize;
        oss << L"PageFile Used : "
            << FormatBytes(pageUsed) << L"\n";
    }
    else
    {
        oss << L"프로세스 메모리 조회 실패\n";
    }

    oss << L"=============================\n";

    memoryReport = oss.str();
}

void MemoryDebugger::Trim() 
{
    dxgiDevice->Trim();
}