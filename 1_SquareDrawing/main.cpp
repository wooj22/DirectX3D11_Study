#include "App.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,	_In_ int       nCmdShow)
{
	App App;
	return App.Run(hInstance);
}
