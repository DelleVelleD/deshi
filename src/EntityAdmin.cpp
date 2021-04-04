/* //TODO(sushi) rewrite this list eventually
------Tick Order---||--------Components-----||-------------------------------
  olcPixelGameEngine      || Input							|| N/A
  TimeSystem              || Time							 || N/A
  ScreenSystem            || Screen						   || N/A
  CommandSystem           || Input, Keybinds, Canvas		  || N/A
  SimpleMovementSystem    || Camera						   || Input, Keybinds, MovementState, Time
  PhysicsSystem           || Time, Transform, Physics		 || Camera, Screen
  CameraSystem            || Camera						   || Screen
  RenderCanvasSystem      || Canvas						   || Screen
  WorldSystem             || World, Entity					|| N/A
  TriggeredCommandSystem  || N/A							  || N/A
  DebugSystem             || ALL							  || ALL
*/

#include "EntityAdmin.h"
#include "core.h"

#include "utils/PhysicsWorld.h"
#include "utils/Command.h"
#include "utils/defines.h"
#include "scene/Scene.h"

#include "game/Keybinds.h"
#include "game/components/Component.h"
#include "game/components/Camera.h"
#include "game/components/Controller.h"
#include "game/components/AudioListener.h"
#include "game/components/Transform.h"
#include "game/components/MeshComp.h"
#include "game/components/Orb.h"

#include "scene/Scene.h"

#include "game/systems/System.h"
#include "game/systems/PhysicsSystem.h"
#include "game/systems/RenderCanvasSystem.h"
#include "game/systems/WorldSystem.h"
#include "game/systems/SoundSystem.h"

//// EntityAdmin ////

void EntityAdmin::Init(Input* i, Window* w, Time* t, Renderer* r, Console* c) {
	time = t;
	input = i;
	window = w;
	console = c;
	renderer = r;
	
	systems = std::vector<System*>();
	entities = std::map<EntityID, Entity*>();
	components = std::vector<Component*>();
	physicsWorld = new PhysicsWorld();
	
	//reserve complayers
	for (int i = 0; i < 8; i++) {
		freeCompLayers.push_back(ContainerManager<Component*>());
	}
	
	//systems initialization
	switch (physicsWorld->integrationMode) {
		default: /* Semi-Implicit Euler */ {
			physics = new PhysicsSystem();
		}
	}
	canvas = new RenderCanvasSystem();
	world = new WorldSystem();
	sound = new SoundSystem();
	
	physics->Init(this);
	canvas->Init(this);
	world->Init(this);
	sound->Init(this);
	
	//singleton initialization
	scene = new Scene();
	mainCamera = new Camera(this, 90.f, .01f, 1000.01f, true);
	mainCamera->layer_index = freeCompLayers[mainCamera->layer].add(mainCamera);
	keybinds = new Keybinds(this);
	controller = new Controller(this);
	controller->layer_index = freeCompLayers[controller->layer].add(controller);
	
	
	
	/*
	//orb testing
	Entity* orbtest = world->CreateEntity(admin);
	orbtest->name = "orbtest";
	orbtest->admin = this;
	Mesh mesh = Mesh::CreateMeshFromOBJ("sphere.obj", "sphere");
	Texture tex("lambertian.jpg");
	admin->renderer->LoadTexture(tex);
	mesh.batchArray[0].textureArray.push_back(tex);
	mesh.batchArray[0].textureCount = 1;
	mesh.batchArray[0].shader = Shader::PBR;
	Mesh* m = new Mesh(mesh);
	OrbManager* om = new OrbManager(m, this, orbtest);
	admin->world->AddAComponentToEntity(admin, orbtest, om);
	*/
}

void EntityAdmin::Cleanup() {
	//cleanup collections
	//for(System* s : systems)       { delete s; }           systems.clear();
	for (auto pair : entities) { delete pair.second; } entities.clear();
	for (Component* c : components) { delete c; }           components.clear();
	
	delete physicsWorld;
	
	//clean up singletons
	delete world;
	delete canvas;
	delete sound;
	delete physics;
	delete mainCamera;
	delete keybinds;
	delete controller;
	delete tempCanvas;
}

void UpdateLayer(ContainerManager<Component*> cl) {
	for (int i = 0; i < cl.size(); i++) {
		if (cl[i]) {
			cl[i].get()->Update();
		}
	}
}

void EntityAdmin::Update() {
	controller->Update();
	mainCamera->Update();
	
	if (!pause_phys && !paused)    UpdateLayer(freeCompLayers[CL0_PHYSICS]);
	if (!pause_phys && !paused)    physics->Update();
	if (!pause_canvas)	           UpdateLayer(freeCompLayers[CL1_RENDCANVAS]);
	if (!pause_canvas)	           canvas->Update();
	if (!pause_console)            UpdateLayer(freeCompLayers[CL2_WORLD]);
	if (!pause_world && !paused)   world->Update();
	if (!pause_sound && !paused)   UpdateLayer(freeCompLayers[CL3_SOUND]);
	if (!pause_sound && !paused)   sound->Update();
	if (!pause_last && !paused)    UpdateLayer(freeCompLayers[CL4_LAST]);
	
	for (Component* c : components) {
		c->Update();
	}
}

void EntityAdmin::AddSystem(System* system) {
	systems.push_back(system);
	system->Init(this);
}

void EntityAdmin::RemoveSystem(System* system) {
	for (int i = 0; i < systems.size(); ++i) {
		if (systems[i] == system) {
			systems.erase(systems.begin() + i);
		}
	}
}

void EntityAdmin::AddComponent(Component* component) {
	components.push_back(component);
	component->entity = 0;
}

void EntityAdmin::RemoveComponent(Component* component) {
	for (int i = 0; i < components.size(); ++i) {
		if (components[i] == component) {
			components.erase(components.begin() + i);
		}
	}
}

Command* EntityAdmin::GetCommand(std::string command) {
	try {
		return console->commands.at(command);
	} catch (std::exception e) {
		//ERROR("Command \"", command, "\" does not exist");
		this->console->PushConsole(TOSTRING("\n[c:red]", "Command \"", command, "\" does not exist", "[c]"));
		return 0;
	}
}

bool EntityAdmin::ExecCommand(std::string command) {
	try {
		console->commands.at(command)->Exec(this);
		return true;
	} catch (std::exception e) {
		//ERROR("Command \"", command, "\" does not exist");
		this->console->PushConsole(TOSTRING("\n[c:red]", "Command \"", command, "\" does not exist", "[c]"));
		return false;
	}
}

bool EntityAdmin::ExecCommand(std::string command, std::string args) {
	try {
		console->commands.at(command)->Exec(admin, args);
		return true;
	} catch (std::exception e) {
		this->console->PushConsole(TOSTRING("\n[c:red]", "Command \"", command, "\" does not exist", "[c]"));
		return false;
	}
}

//// Entity ////

u32 Entity::AddComponent(Component* component) {
	components.push_back(component);
	component->entity = this;
	return components.size() - 1;
}

u32 Entity::AddComponents(std::vector<Component*> comps) {
	u32 value = this->components.size();
	for (auto& c : comps) {
		this->components.push_back(c);
		c->entity = this;
	}
	return value;
}

Entity::Entity() {
	transform = new Transform();
}

Entity::~Entity() {
	for (Component* c : components) delete c;
}