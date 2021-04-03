#pragma once
#include "System.h"

struct RenderCanvasSystem : public System {
	bool showDebugTools = false;
	bool showDebugBar = true;
	bool showMenuBar = true;
	bool showImGuiDemoWindow = false;
	bool showDebugLayer = true;
	
	//this can be done better
	bool ConsoleHovFlag = false;
	
	void DebugBar();
	void DebugTools();
	void MenuBar();
	void DebugLayer();
	
	void Init(EntityAdmin* admin) override;
	void Update() override;
	void DrawUI(void);
};