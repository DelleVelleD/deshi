#pragma once
#ifndef DESHI_ENTITYADMIN_H
#define DESHI_ENTITYADMIN_H

#include "utils/defines.h"
#include "utils/debug.h"
#include "utils/ContainerManager.h"
#include "scene/scene.h"
#include "game/Transform.h"
#include "game/Keybinds.h"
#include "game/Controller.h"
#include "game/UndoManager.h"

#include <map>

#define DengKeys admin->keybinds

struct Entity;
struct System;
struct Component;
struct Command;
struct Camera;
struct PhysicsSystem;
struct CanvasSystem;
struct WorldSystem;
struct SoundSystem;

enum struct GameState{
	NONE, PLAY, PLAY_DEBUG, EDITOR, MENU
};

struct EntityAdmin {
	GameState state = GameState::NONE;
	
	std::vector<Entity> entities;
	//object_pool<Component>* components;
	
	//systems
	PhysicsSystem* physics;
	CanvasSystem*  canvas;
	WorldSystem*   world;
	SoundSystem*   sound;
	
	//admin singletons
	Scene       scene;
	Keybinds    keybinds;
	Controller  controller;
	UndoManager undoManager;
	
	Entity* player;
	Camera* mainCamera;
	
	Entity* selectedEntity = 0;
	
	//stores the components to be executed in between layers
	std::vector<ContainerManager<Component*>> freeCompLayers;
	
	//pause flags
	b32  skip = false;
	bool paused = false;
	bool pause_command = false;
	bool pause_phys = false;
	bool pause_canvas = false;
	bool pause_world = false;
	bool pause_console = false;
	bool pause_sound = false;
	bool pause_last = false;
	
	//timer related
	bool debugTimes = true;
	TIMER_START(t_a);
	
	void Init();
	void Cleanup();
	
	void Update();
	
	void Save();
	void Load(const char* filename);
};

struct Entity {
	EntityAdmin* admin; //reference to owning admin
	
	u32 id;
	char name[64];
	Transform transform;
	std::vector<Component*> components;
	
	Entity();
	Entity(EntityAdmin* admin, u32 id, Transform transform = Transform(), 
		   const char* name = 0, std::vector<Component*> components = {});
	~Entity();
	
	std::vector<char> Save();
	void Load(const char* filename);
	
	void SetName(const char* name);
	//adds a component to the end of the components vector
	//returns the position in the vector
	void AddComponent(Component* component);
	void AddComponents(std::vector<Component*> components);
	void RemoveComponent(Component* component);
	void RemoveComponents(std::vector<Component*> components);
	
	//returns a component pointer from the entity of provided type, nullptr otherwise
	template<class T>
		T* GetComponent() {
		T* t = nullptr;
		for (Component* c : components) { 
			if (T* temp = dynamic_cast<T*>(c)) { 
				t = temp; break; 
			} 
		}
		return t;
	}
};

//global admin pointer
extern EntityAdmin* g_admin;

#endif