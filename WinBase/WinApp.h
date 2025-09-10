#pragma once
#include "Windows.h"
#include "string"
#include "Input.h"
#include "Time.h"

using namespace std;

class WinApp
{
public:
	// window
	static WinApp*	 Instance;		// 싱글톤
	static HWND		 hWnd; 			// 윈도우 핸들
	int			  	 screenWidth= 1280;
	int				 screenHeight = 720;

protected:
	HINSTANCE hInstance;			// 실행중인 프로그램 인스턴스 핸들
	WNDCLASSEX wc; 					// 윈도우 클래스	
	wstring			winClassName = L"DirectX3D11";
	wstring			titleName = L"DirectX3D11";
	string			modulePath;
	string			workingPath;

	MSG msg;						// 윈도우 메시지

public:
	Input input;			// input
	Time  time;				// time

public:
	// main process
	WinApp();
	virtual ~WinApp();

	bool Init();
	void Uninit();
	bool Run(HINSTANCE hInstance);
	void Update();
	void Render();
	
	// window message
	virtual LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
public:
	// 콘텐츠 프로젝트에서 override해서 사용
	virtual bool OnInit() { return true; };
	virtual void OnRender() {};
	virtual void OnUpdate() {};
	virtual void OnUninit() {};

	// screen size
	void SetScreenize(UINT width, UINT height) { this->screenWidth = width; this->screenHeight = height; }
};

