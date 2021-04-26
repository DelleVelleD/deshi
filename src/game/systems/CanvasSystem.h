#pragma once
#ifndef SYSTEM_CANVAS_H
#define SYSTEM_CANVAS_H

#include "System.h"

struct Renderer;
struct Vector2;
struct Vector3;
struct Color;
struct Camera;

namespace ImGui {
	void BeginDebugLayer();
	void EndDebugLayer(); //not necessary, but I'm adding it for clarity in code 
	void DebugDrawCircle(Vector2 pos, float radius, Color color = Color::WHITE);
	void DebugDrawCircle3(Vector3 pos, float radius, Camera* c, Vector2 windimen, Color color = Color::WHITE);
	void DebugDrawLine(Vector2 pos1, Vector2 pos2, Color color = Color::WHITE);
	void DebugDrawLine3(Vector3 pos1, Vector3 pos2, Camera* c, Vector2 windimen, Color color = Color::WHITE);
	void DebugDrawText(const char* text, Vector2 pos, Color color = Color::WHITE);
	void DebugDrawText3(const char* text, Vector3 pos, Camera* c, Vector2 windimen, Color color = Color::WHITE, Vector2 twoDoffset = Vector2::ZERO);
	void DebugDrawGraphFloat(Vector2 pos, float inval, float sizex = 100, float sizey = 100);
}

struct CanvasSystem : public System {
	bool showDebugTools = true;
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

#endif //SYSTEM_CANVAS_H