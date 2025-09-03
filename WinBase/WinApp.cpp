#include "WinApp.h"
#include <dbghelp.h>
#include <minidumpapiset.h>

#pragma comment(lib, "Dbghelp.lib")

// static member init
WinApp* WinApp::Instance = nullptr;
HWND	 WinApp::hWnd = nullptr;


/*------------------------- 윈도우 메시지 프로시저 -------------------------*/
LRESULT CALLBACK DefaultWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return  WinApp::Instance->WindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WinApp::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ACTIVATEAPP:
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


/*------ 크래시 발생 시 자동으로 미니덤프(.dmp) 파일을 생성하는 시스템 -------*/
// dump file create
void CreateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
	wchar_t moduleFileName[MAX_PATH] = { 0, };
	std::wstring fileName(moduleFileName);
	if (GetModuleFileName(NULL, moduleFileName, MAX_PATH) == 0) {
		fileName = L"unknown_project.dmp"; // 예외 상황 처리
	}
	else
	{
		fileName = std::wstring(moduleFileName);
		size_t pos = fileName.find_last_of(L"\\/");
		if (pos != std::wstring::npos) {
			fileName = fileName.substr(pos + 1); // 파일 이름 추출
		}

		pos = fileName.find_last_of(L'.');
		if (pos != std::wstring::npos) {
			fileName = fileName.substr(0, pos); // 확장자 제거
		}
		fileName += L".dmp";
	}

	HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return;

	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ExceptionPointers = pExceptionPointers;
	dumpInfo.ClientPointers = TRUE;

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

	CloseHandle(hFile);
}

// exception handler
LONG WINAPI CustomExceptionHandler(EXCEPTION_POINTERS* pExceptionPointers)
{
	int msgResult = MessageBox(
		NULL,
		L"Should Create Dump ?",
		L"Exception",
		MB_YESNO | MB_ICONQUESTION
	);

	if (msgResult == IDYES) {
		CreateDump(pExceptionPointers);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}



/*----------------------- window process ---------------------------*/
WinApp::WinApp()
{
	WinApp::Instance = this;
}

WinApp::~WinApp()
{
	WinApp::Instance = nullptr;
}

bool WinApp::Init()
{
	// exception handler set
	SetUnhandledExceptionFilter(CustomExceptionHandler);

	// module path save
	char szPath[MAX_PATH] = { 0, };
	GetModuleFileNameA(NULL, szPath, MAX_PATH);
	modulePath = szPath;
	
	// directory path save
	GetCurrentDirectoryA(MAX_PATH, szPath);
	workingPath = szPath;

	// get program handle
	hInstance = GetModuleHandle(NULL);

	// window class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = DefaultWindowProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;
	wc.lpszClassName = winClassName.c_str();
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	RegisterClassEx(&wc);

	SIZE clientSize = { screenWidth, screenHeight };
	RECT clientRect = { 0, 0, clientSize.cx, clientSize.cy };

	// window create
	hWnd = CreateWindowEx(
		0,
		winClassName.c_str(),
		titleName.c_str(),
		WS_OVERLAPPEDWINDOW,		// WS_POPUP/ WS_OVERLAPPEDWINDOW
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		NULL,
		NULL,
		hInstance,
		this
	);

	// show window
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// util init
	input.Init(hWnd);
	time.Init();

	if (!OnInit()) return false;
	return true;
}

void WinApp::Uninit()
{
	OnUninit();
}

bool WinApp::Run(HINSTANCE hInstance)
{
	wc.hInstance = hInstance;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefaultWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = winClassName.c_str();

	try
	{
		// init
		if (!Init()) return false;

		// Loop
		while (TRUE)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					break;

				// 윈도우 메시지 처리 
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				Update();
				Render();
			}
		}
	}
	catch (const std::exception& e)
	{
		::MessageBoxA(NULL, e.what(), "Exception", MB_OK);
	}

	// uninit
	Uninit();
	return true;
}

void WinApp::Update()
{
	time.Update();
	input.Update();
	OnUpdate();
}

void WinApp::Render()
{
	OnRender();
}