#pragma once
#ifndef DESHI_INPUT_H
#define DESHI_INPUT_H

#include "console.h"
#include "../utils/defines.h"
#include "../utils/debug.h"
#include "../math/Vector.h"

#include <map>
#include <iostream>

//constants
#define MAX_KEYBOARD_KEYS 256
#define MAX_MOUSE_BUTTONS 7

namespace Key {
	typedef enum KeyBits {
		NONE,
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		K0, K1, K2, K3, K4, K5, K6, K7, K8, K9,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		UP, DOWN, LEFT, RIGHT,
		ESCAPE, TILDE, TAB, CAPSLOCK, LSHIFT, LCTRL, LALT,
		BACKSPACE, ENTER, RSHIFT, RCTRL, RALT, MINUS, EQUALS, LBRACKET, RBRACKET,
		SLASH, SEMICOLON, APOSTROPHE, COMMA, PERIOD, BACKSLASH, SPACE,
		INSERT, DELETE, HOME, END, PAGEUP, PAGEDOWN, PAUSE, SCROLL,
		NUMPAD0, NUMPAD1, NUMPAD2, NUMPAD3, NUMPAD4, NUMPAD5, NUMPAD6, NUMPAD7, NUMPAD8, NUMPAD9,
		NUMPADMULTIPLY, NUMPADDIVIDE, NUMPADPLUS, NUMPADMINUS, NUMPADPERIOD, NUMPADENTER, NUMLOCK,
		MBLEFT, MBRIGHT, MBMIDDLE, MBFOUR, MBFIVE, MBSIX, MBSEVEN, MBEIGHT, MBSCROLLDOWN, MBSCROLLUP
	} KeyBits;
	typedef u32 Key;
};

namespace MouseButton{
	typedef enum MouseButtonBits{
		LEFT = Key::MBLEFT, RIGHT = Key::MBRIGHT, MIDDLE = Key::MBMIDDLE, 
		FOUR = Key::MBFOUR, FIVE = Key::MBFIVE, 
		SIX = Key::MBSIX, SEVEN = Key::MBSEVEN, 
		EIGHT = Key::MBEIGHT,
		SCROLLDOWN = Key::MBSCROLLDOWN, SCROLLUP = Key::MBSCROLLUP
	} MouseButtonBits;
	typedef u32 MouseButton;
}

//TODO(sushi, In) add right and left differenciation and a middle term for both
typedef enum InputModFlagBits{
	INPUTMOD_NONE          = 0,
	INPUTMOD_ANY           = 256,
	INPUTMOD_CTRL          = 512,
	INPUTMOD_SHIFT         = 1024,
	INPUTMOD_ALT           = 2048,
	INPUTMOD_CTRLSHIFT     = 1536,
	INPUTMOD_CTRLALT       = 2560,
	INPUTMOD_SHIFTALT      = 3072,
	INPUTMOD_CTRLSHIFTALT  = 3584,
} InputModFlagBits;
typedef u32 InputModFlags;

struct Input{
	std::map<size_t, u8> mapKeys;
	std::map<size_t, u8> mapMouse;
	
	//TODO(delle,OpIn) look into storing these as vector<bool> instead
	bool oldKeyState[MAX_KEYBOARD_KEYS]   = {0};
	bool newKeyState[MAX_KEYBOARD_KEYS]   = {0};
	double mouseX,       mouseY;
	double screenMouseX, screenMouseY;
	double scrollX,      scrollY;
	Vector2 mousePos;
	
	//NOTE sushi: I was going to put this on keybinds, but I wanted it to only check binds if some input occured, and it seems easiest to do that here
	//for console command binding
	std::vector<std::pair<std::string, Key::Key>> binds;
	bool checkbinds = false; //needed bc glfw callbacks would call the function too early
	
	//real values are updated through GLFW callbacks
	bool realKeyState[MAX_KEYBOARD_KEYS]   = {0};
	double realMouseX,       realMouseY;
	double realScreenMouseX, realScreenMouseY;
	double realScrollX,      realScrollY;
	bool keyFocus, mouseFocus;
	
	b32 logInput = false;
	
	//caches values so they are consistent thru the frame
	void Update(){
		ZoneScoped;

		memcpy(&oldKeyState, &newKeyState, sizeof(bool) * MAX_KEYBOARD_KEYS);
		memcpy(&newKeyState, &realKeyState, sizeof(bool) * MAX_KEYBOARD_KEYS);
		//mouseX = realMouseX; mouseY = realMouseY; //NOTE this doesnt work, idk why
		mousePos.x = mouseX; mousePos.y = mouseY;
		screenMouseY = realScreenMouseX; screenMouseY = realScreenMouseY;
		//bit scuffed mouse wheel stuff
		if      (realScrollY < 0) newKeyState[MouseButton::SCROLLDOWN] = 1;
		else if (realScrollY > 0) newKeyState[MouseButton::SCROLLUP] = 1;
		else    { newKeyState[MouseButton::SCROLLDOWN] = 0; newKeyState[MouseButton::SCROLLUP] = 0; }
		realScrollY = 0;
	}
	
	/////////////////////////////////
	//// input helper functions /////
	/////////////////////////////////
	
	inline bool CtrlDown(){ return newKeyState[Key::LCTRL] || newKeyState[Key::RCTRL]; }
	inline bool ShiftDown(){ return newKeyState[Key::LSHIFT] || newKeyState[Key::RSHIFT]; }
	inline bool AltDown(){ return newKeyState[Key::LALT] || newKeyState[Key::RALT]; }
	
	bool ModsDown(u32 mods){
		switch(mods){
			case(INPUTMOD_ANY):          return true;
			case(INPUTMOD_NONE):         return !CtrlDown() && !ShiftDown() && !AltDown();
			case(INPUTMOD_CTRL):         return  CtrlDown() && !ShiftDown() && !AltDown();
			case(INPUTMOD_SHIFT):        return !CtrlDown() &&  ShiftDown() && !AltDown();
			case(INPUTMOD_ALT):          return !CtrlDown() && !ShiftDown() &&  AltDown();
			case(INPUTMOD_CTRLSHIFT):    return  CtrlDown() &&  ShiftDown() && !AltDown();
			case(INPUTMOD_CTRLALT):      return  CtrlDown() && !ShiftDown() &&  AltDown();
			case(INPUTMOD_SHIFTALT):     return !CtrlDown() &&  ShiftDown() &&  AltDown();
			case(INPUTMOD_CTRLSHIFTALT): return  CtrlDown() &&  ShiftDown() &&  AltDown();
			default: ERROR("[ERROR] ModsDown called with invalid mod: ", mods, "; defaulting to any mod"); return true;
		}
	}
	
	/////////////////////////////
	//// keyboard keys input ////
	/////////////////////////////
	
	//mod_key & 0x000000FF extract the key
	//mod_key & 0xFFFFFF00 extract the mods
	
	inline bool KeyDownAnyMod(u32 mod_key){ return newKeyState[mod_key & 0x000000FF]; }
	
	inline bool KeyDown(u32 mod_key) { 
		u32 key = mod_key & 0x000000FF;
		return newKeyState[key] && ModsDown(mod_key & 0xFFFFFF00); 
	}
	
	inline bool KeyUp(u32 mod_key) {
		u32 key = mod_key & 0x000000FF;
		return newKeyState[key] && ModsDown(mod_key & 0xFFFFFF00);
	}
	
	inline bool KeyPressed(u32 mod_key) {
		u32 key = mod_key & 0x000000FF;
		return newKeyState[key] && !oldKeyState[key] && ModsDown(mod_key & 0xFFFFFF00);
	}
	
	inline bool KeyReleased(u32 mod_key) {
		u32 key = mod_key & 0x000000FF;
		return !newKeyState[key] && oldKeyState[key] && ModsDown(mod_key & 0xFFFFFF00);
	}
	
	inline void AddBind(std::string command, Key::Key key) {
		
	}
	
	
	
};

//global input pointer
extern Input* g_input;
#define DengInput g_input


#endif //DESHI_INPUT_H