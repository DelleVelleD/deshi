#include "window.h"
#include "input.h"
#include "console.h"
#include "assets.h"
#include "../utils/debug.h"

#include "../external/stb/stb_image.h"

#if defined(_MSC_VER)
#pragma comment(lib,"glfw3.lib")
#endif
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

void glfwError(int id, const char* description){
	std::cout << description << std::endl;
}

//thanks: https://github.com/OneLoneCoder/olcPixelGameEngine/pull/181
void Window::Init(Input* input, i32 width, i32 height, i32 x, i32 y, DisplayMode displayMode){
	glfwSetErrorCallback(&glfwError);
	if (!glfwInit()){ return; }
	Window::input = input;
	
	//TODO(delle,Wi) maybe we should not allow the window to be resizable in-game, but in-engine is fine
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width, height, "deshi", NULL, NULL);
	monitor = glfwGetPrimaryMonitor();
	if(!window) { glfwTerminate(); return; }
	if(!monitor) { glfwTerminate(); return; }
	
	//load and set icon
	GLFWimage icon; int icon_channels;
	icon.pixels = stbi_load("data/textures/deshi_icon.png", &icon.width, &icon.height, &icon_channels, STBI_rgb_alpha);
	if(icon.pixels){
		glfwSetWindowIcon(window, 1, &icon);
	}else{
		PRINTLN("Failed to load texture: deshi_icon.png; Using default window icon");
	}
	
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int xpos, ypos;
	glfwGetWindowPos(window, &xpos, &ypos);
	this->x = xpos; this->y = ypos;
	this->width = width; this->height = height;
	this->restoreX = x; this->restoreY = y;
	this->restoreW = width; this->restoreH = height;
	this->screenWidth = mode->width; this->screenHeight = mode->height;
	this->screenRefreshRate = mode->refreshRate;
	this->displayMode = displayMode;
	this->cursorMode = CursorMode::DEFAULT;
	this->dimensions = Vector2(width, height);
	
	this->rawInput = false;
	UpdateRawInput(true); //sets raw input to true if supported
	this->resizable = true;
	this->closeWindow = false;
	
	UpdateDisplayMode(displayMode);
	
	//keyboard mappings
	input->mapKeys[0x00] = Key::NONE;
	input->mapKeys['A'] = Key::A; input->mapKeys['B'] = Key::B; input->mapKeys['C'] = Key::C;
	input->mapKeys['D'] = Key::D; input->mapKeys['E'] = Key::E; input->mapKeys['F'] = Key::F;
	input->mapKeys['G'] = Key::G; input->mapKeys['H'] = Key::H; input->mapKeys['I'] = Key::I;
	input->mapKeys['J'] = Key::J; input->mapKeys['K'] = Key::K; input->mapKeys['L'] = Key::L; 
	input->mapKeys['M'] = Key::M; input->mapKeys['N'] = Key::N; input->mapKeys['O'] = Key::O;
	input->mapKeys['P'] = Key::P; input->mapKeys['Q'] = Key::Q; input->mapKeys['R'] = Key::R;
	input->mapKeys['S'] = Key::S; input->mapKeys['T'] = Key::T; input->mapKeys['U'] = Key::U;
	input->mapKeys['V'] = Key::V; input->mapKeys['W'] = Key::W; input->mapKeys['X'] = Key::X;
	input->mapKeys['Y'] = Key::Y; input->mapKeys['Z'] = Key::Z;
	
	input->mapKeys[GLFW_KEY_0] = Key::K0; input->mapKeys[GLFW_KEY_1] = Key::K1;
	input->mapKeys[GLFW_KEY_2] = Key::K2; input->mapKeys[GLFW_KEY_3] = Key::K3; 
	input->mapKeys[GLFW_KEY_4] = Key::K4; input->mapKeys[GLFW_KEY_5] = Key::K5;
	input->mapKeys[GLFW_KEY_6] = Key::K6; input->mapKeys[GLFW_KEY_7] = Key::K7; 
	input->mapKeys[GLFW_KEY_8] = Key::K8; input->mapKeys[GLFW_KEY_9] = Key::K9;
	
	input->mapKeys[GLFW_KEY_F1]  = Key::F1;  input->mapKeys[GLFW_KEY_F2]  = Key::F2; 
	input->mapKeys[GLFW_KEY_F3]  = Key::F3;  input->mapKeys[GLFW_KEY_F4]  = Key::F4; 
	input->mapKeys[GLFW_KEY_F5]  = Key::F5;  input->mapKeys[GLFW_KEY_F6]  = Key::F6;
	input->mapKeys[GLFW_KEY_F7]  = Key::F7;  input->mapKeys[GLFW_KEY_F8]  = Key::F8; 
	input->mapKeys[GLFW_KEY_F9]  = Key::F9;  input->mapKeys[GLFW_KEY_F10] = Key::F10;
	input->mapKeys[GLFW_KEY_F11] = Key::F11; input->mapKeys[GLFW_KEY_F12] = Key::F12;
	
	input->mapKeys[GLFW_KEY_DOWN] = Key::DOWN; input->mapKeys[GLFW_KEY_UP]    = Key::UP;
	input->mapKeys[GLFW_KEY_LEFT] = Key::LEFT; input->mapKeys[GLFW_KEY_RIGHT] = Key::RIGHT;
	
	input->mapKeys[GLFW_KEY_ESCAPE]        = Key::ESCAPE; 
	input->mapKeys[GLFW_KEY_GRAVE_ACCENT]  = Key::TILDE;
	input->mapKeys[GLFW_KEY_TAB]           = Key::TAB; 
	input->mapKeys[GLFW_KEY_CAPS_LOCK]     = Key::CAPSLOCK;
	input->mapKeys[GLFW_KEY_LEFT_SHIFT]    = Key::LSHIFT; 
	input->mapKeys[GLFW_KEY_RIGHT_SHIFT]   = Key::RSHIFT;
	input->mapKeys[GLFW_KEY_LEFT_CONTROL]  = Key::LCTRL; 
	input->mapKeys[GLFW_KEY_RIGHT_CONTROL] = Key::RCTRL;
	input->mapKeys[GLFW_KEY_LEFT_ALT]      = Key::LALT; 
	input->mapKeys[GLFW_KEY_RIGHT_ALT]     = Key::RALT; 
	input->mapKeys[GLFW_KEY_BACKSPACE]     = Key::BACKSPACE; 
	input->mapKeys[GLFW_KEY_ENTER]         = Key::ENTER;
	input->mapKeys[GLFW_KEY_MINUS]         = Key::MINUS; 
	input->mapKeys[GLFW_KEY_EQUAL]         = Key::EQUALS;
	input->mapKeys[GLFW_KEY_LEFT_BRACKET]  = Key::LBRACKET; 
	input->mapKeys[GLFW_KEY_RIGHT_BRACKET] = Key::RBRACKET;
	input->mapKeys[GLFW_KEY_SLASH]         = Key::SLASH; 
	input->mapKeys[GLFW_KEY_SEMICOLON]     = Key::SEMICOLON;
	input->mapKeys[GLFW_KEY_APOSTROPHE]    = Key::APOSTROPHE; 
	input->mapKeys[GLFW_KEY_COMMA]         = Key::COMMA;
	input->mapKeys[GLFW_KEY_PERIOD]        = Key::PERIOD;
	input->mapKeys[GLFW_KEY_BACKSLASH]     = Key::BACKSLASH;
	input->mapKeys[GLFW_KEY_SPACE]         = Key::SPACE;
	
	input->mapKeys[GLFW_KEY_INSERT]  = Key::INSERT; input->mapKeys[GLFW_KEY_DELETE]      = Key::DELETE;
	input->mapKeys[GLFW_KEY_HOME]    = Key::HOME;   input->mapKeys[GLFW_KEY_END]         = Key::END;
	input->mapKeys[GLFW_KEY_PAGE_UP] = Key::PAGEUP; input->mapKeys[GLFW_KEY_PAGE_DOWN]   = Key::PAGEDOWN;
	input->mapKeys[GLFW_KEY_PAUSE]   = Key::PAUSE;  input->mapKeys[GLFW_KEY_SCROLL_LOCK] = Key::SCROLL;
	
	input->mapKeys[GLFW_KEY_KP_0] = Key::NUMPAD0; input->mapKeys[GLFW_KEY_KP_1] = Key::NUMPAD1;
	input->mapKeys[GLFW_KEY_KP_2] = Key::NUMPAD2; input->mapKeys[GLFW_KEY_KP_3] = Key::NUMPAD3;
	input->mapKeys[GLFW_KEY_KP_4] = Key::NUMPAD4; input->mapKeys[GLFW_KEY_KP_5] = Key::NUMPAD5;
	input->mapKeys[GLFW_KEY_KP_6] = Key::NUMPAD6; input->mapKeys[GLFW_KEY_KP_7] = Key::NUMPAD7;
	input->mapKeys[GLFW_KEY_KP_8] = Key::NUMPAD8; input->mapKeys[GLFW_KEY_KP_9] = Key::NUMPAD9;
	
	input->mapKeys[GLFW_KEY_KP_MULTIPLY] = Key::NUMPADMULTIPLY;
	input->mapKeys[GLFW_KEY_KP_DIVIDE]   = Key::NUMPADDIVIDE;
	input->mapKeys[GLFW_KEY_KP_ADD]      = Key::NUMPADPLUS; 
	input->mapKeys[GLFW_KEY_KP_SUBTRACT] = Key::NUMPADMINUS;
	input->mapKeys[GLFW_KEY_KP_DECIMAL]  = Key::NUMPADPERIOD; 
	input->mapKeys[GLFW_KEY_KP_ENTER]    = Key::NUMPADENTER;
	input->mapKeys[GLFW_KEY_NUM_LOCK]    = Key::NUMLOCK;
	
	//mouse mappings
	input->mapMouse[GLFW_MOUSE_BUTTON_1] = MouseButton::LEFT;
	input->mapMouse[GLFW_MOUSE_BUTTON_2] = MouseButton::RIGHT;
	input->mapMouse[GLFW_MOUSE_BUTTON_3] = MouseButton::MIDDLE;
	input->mapMouse[GLFW_MOUSE_BUTTON_4] = MouseButton::FOUR;
	input->mapMouse[GLFW_MOUSE_BUTTON_5] = MouseButton::FIVE;
	input->mapMouse[GLFW_MOUSE_BUTTON_6] = MouseButton::SIX;
	input->mapMouse[GLFW_MOUSE_BUTTON_7] = MouseButton::SEVEN;
	input->mapMouse[GLFW_MOUSE_BUTTON_8] = MouseButton::EIGHT;
	
	//event callbacks
	//void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	glfwSetMouseButtonCallback(window, 
							   [](GLFWwindow* w, int button, int action, int mods)->void{
								   std::map<size_t, u8>::iterator it = Window::input->mapMouse.find(button);
								   if(it != Window::input->mapMouse.end()){
									   if(action == GLFW_PRESS){
										   Window::input->realKeyState[it->second] = true;
										   Window::input->checkbinds = true;
										   if(Window::input->logInput) { LOG("{m", it->second, "|", mods,"}"); }
									   }else if(action == GLFW_RELEASE){
										   Window::input->realKeyState[it->second] = false;
									   }
								   }
							   });
	
	//static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	glfwSetCursorPosCallback(window,
							 [](GLFWwindow* window, double xpos, double ypos)->void{
								 //mouse coords come in screen space, must convert to window space
								 GLFWmonitor* monitor = glfwGetPrimaryMonitor();
								 int x, y, width, height;
								 glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
								 Window::input->mouseFocus = true;
								 Window::input->realMouseX = xpos - (double)x;
								 Window::input->realMouseY = ypos - (double)y;
								 Window::input->realScreenMouseX = xpos;
								 Window::input->realScreenMouseY = ypos;
							 });
	
	//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	glfwSetKeyCallback(window,
					   [](GLFWwindow* window, int key, int scancode, int action, int mods)->void{
						   std::map<size_t, u8>::iterator it = Window::input->mapKeys.find(key);
						   if(it != Window::input->mapKeys.end()){
							   if(action == GLFW_PRESS){
								   Window::input->realKeyState[it->second] = true;
								   Window::input->checkbinds = true;
								   if(Window::input->logInput) { LOG("{k", it->second, "|", mods,"}"); }
							   }else if(action == GLFW_RELEASE){
								   Window::input->realKeyState[it->second] = false;
							   }
						   }
					   });
	
	//void cursor_enter_callback(GLFWwindow* window, int entered)
	glfwSetCursorEnterCallback(window, 
							   [](GLFWwindow* w, int entered)->void{
								   Window::input->keyFocus = entered;
							   });
	
	//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	glfwSetScrollCallback(window, 
						  [](GLFWwindow* window, double xoffset, double yoffset)->void{
							  Window::input->realScrollX = xoffset;
							  Window::input->realScrollY = yoffset;
						  });
	
}//Init

void Window::Update() {
	ZoneScoped;

	int xpos, ypos;
	glfwGetWindowPos(window, &xpos, &ypos);
	x = xpos; y = ypos;
	
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	width = w; height = h;
	minimized = (w <= 0 || h <= 0) ? true : false;
	
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	screenWidth = mode->width; screenHeight = mode->height;
	screenRefreshRate = mode->refreshRate; 
	
	centerX = w/2; centerY = h/2;
	this->dimensions = Vector2(width, height);
	
	glfwGetCursorPos(window, &Window::input->mouseX, &Window::input->mouseY);
	if(cursorMode == CursorMode::FIRSTPERSON){ glfwSetCursorPos(window, w/2, h/2); }
}

void Window::Cleanup(){
	glfwTerminate();
}

void Window::UpdateDisplayMode(DisplayMode displayMode){
	if(displayMode == this->displayMode){return;}
	if(this->displayMode == DisplayMode::WINDOWED){
		restoreX = x;  restoreY = y; 
		restoreW = width; restoreH = height;
	}
	
	this->displayMode = displayMode;
	
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	switch(displayMode){
		case(DisplayMode::FULLSCREEN):{
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}break;
		case(DisplayMode::BORDERLESS):{
			glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
			glfwSetWindowMonitor(window, 0, 0, 0, mode->width, mode->height, mode->refreshRate);
		}break;
		case(DisplayMode::WINDOWED):default:{
			glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
			glfwSetWindowMonitor(window, 0, restoreX, restoreY, restoreW, restoreH, GLFW_DONT_CARE);
		}break;
	}
}

void Window::UpdateCursorMode(CursorMode mode){
	if(mode == this->cursorMode){return;}
	this->cursorMode = mode;
	
	switch(mode){
		case(CursorMode::DEFAULT):default:{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}break;
		case(CursorMode::FIRSTPERSON):{
			int w, h;
			glfwGetWindowSize(window, &w, &h);
			glfwSetCursorPos(window, w/2, h/2);
			glfwGetCursorPos(window, &Window::input->mouseX, &Window::input->mouseY);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}break;
		case(CursorMode::HIDDEN):{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}break;
	}
}

void Window::SetCursorPos(Vector2 pos){
	glfwSetCursorPos(window, pos.x, pos.y);
}

void Window::UpdateRawInput(bool rawInput){
	if (glfwRawMouseMotionSupported()){
		this->rawInput = rawInput;
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, rawInput);
	}
}

void Window::UpdateResizable(bool resizable){
	this->resizable = resizable;
	glfwSetWindowAttrib(this->window, GLFW_RESIZABLE, resizable);
}

void Window::Close() {
	closeWindow = true;
}

std::string Window::str(){
	std::string dispMode;
	switch (displayMode) {
		case(DisplayMode::WINDOWED):   { dispMode = "Windowed"; }break;
		case(DisplayMode::BORDERLESS): { dispMode = "Borderless Windowed"; }break;
		case(DisplayMode::FULLSCREEN): { dispMode = "Fullscreen"; }break;
	}
	std::string cursMode;
	switch (cursorMode) {
		case(CursorMode::DEFAULT):     { cursMode = "Default"; }break;
		case(CursorMode::FIRSTPERSON): { cursMode = "First Person"; }break;
		case(CursorMode::HIDDEN):      { cursMode = "Hidden"; }break;
	}
	return TOSTRING("Window Info"
					"\n    Window Position: ", x, ",", y,
					"\n    Window Dimensions: ", width, "x", height,
					"\n    Screen Dimensions: ", screenWidth, "x", screenHeight,
					"\n    Refresh Rate: ", refreshRate,
					"\n    Screen Refresh Rate: ", screenRefreshRate,
					"\n    Display Mode: ", dispMode,
					"\n    Cursor Mode: ", cursMode,
					"\n    Raw Input: ", rawInput,
					"\n    Resizable: ", resizable,
					"\n    Restores: ", restoreX, ",", restoreY, " ", restoreW, "x", restoreH);
}