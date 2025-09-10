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
	static WinApp*	 Instance;		// �̱���
	static HWND		 hWnd; 			// ������ �ڵ�
	int			  	 screenWidth= 1280;
	int				 screenHeight = 720;

protected:
	HINSTANCE hInstance;			// �������� ���α׷� �ν��Ͻ� �ڵ�
	WNDCLASSEX wc; 					// ������ Ŭ����	
	wstring			winClassName = L"DirectX3D11";
	wstring			titleName = L"DirectX3D11";
	string			modulePath;
	string			workingPath;

	MSG msg;						// ������ �޽���

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
	// ������ ������Ʈ���� override�ؼ� ���
	virtual bool OnInit() { return true; };
	virtual void OnRender() {};
	virtual void OnUpdate() {};
	virtual void OnUninit() {};

	// screen size
	void SetScreenize(UINT width, UINT height) { this->screenWidth = width; this->screenHeight = height; }
};

