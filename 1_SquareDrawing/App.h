#pragma once
#include "../WinBase/WinApp.h"
#include <d3d11.h>


class App : public WinApp
{
public:
	App();
	~App() override;

	// override
	virtual bool OnInit() override;
	virtual void OnUninit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;
};

