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

#include "utils/Command.h"
#include "utils/defines.h"
#include "scene/Scene.h"

#include "game/Keybinds.h"
#include "game/Transform.h"
#include "game/Controller.h"
#include "game/components/Component.h"
#include "game/components/Collider.h"
#include "game/components/Camera.h"
#include "game/components/AudioListener.h"
#include "game/components/AudioSource.h"
#include "game/components/MeshComp.h"
#include "game/components/Orb.h"
#include "game/components/Light.h"
#include "game/components/Physics.h"
#include "game/systems/System.h"
#include "game/systems/PhysicsSystem.h"
#include "game/systems/CanvasSystem.h"
#include "game/systems/WorldSystem.h"
#include "game/systems/SoundSystem.h"

#include <iostream>
#include <fstream>

//// EntityAdmin ////

void EntityAdmin::Init() {
	state = GameState::EDITOR;
	entities.reserve(1000);
	
	//reserve complayers
	for (int i = 0; i < 8; i++) {
		freeCompLayers.push_back(ContainerManager<Component*>());
	}
	
	//systems initialization
	physics = new PhysicsSystem();
	canvas = new CanvasSystem();
	world  = new WorldSystem();
	sound  = new SoundSystem();
	physics->Init(this);
	canvas->Init(this);
	world->Init(this);
	sound->Init(this);
	
	scene.Init();
	DengRenderer->LoadScene(&scene);
	keybinds.Init();
	controller.Init(this);
	undoManager.Init();
	
	//singleton initialization
	mainCamera = new Camera(this, 90.f, .01f, 1000.01f, true);
	mainCamera->layer_index = freeCompLayers[mainCamera->layer].add(mainCamera);
	
	/*
	//orb testing
	Mesh* mesh = new Mesh(Mesh::CreateMeshFromOBJ("sphere.obj", "sphere.obj"));
	//Texture tex("default1024.png");
	//DengRenderer->LoadTexture(tex);
	//*mesh.batchArray[0].textureArray.push_back(tex);
	//*mesh.batchArray[0].textureCount = 1;
	mesh->batchArray[0].shader = Shader::WIREFRAME;
	OrbManager* om = new OrbManager(mesh, this);
	world->CreateEntity(admin, {om}, "orbtest");
*/
}

void EntityAdmin::Cleanup() {
	//cleanup collections
	entities.clear();
	freeCompLayers.clear();
	
	delete physics;
	delete canvas;
	delete world;
	delete sound;
	delete mainCamera;
}

void UpdateLayer(ContainerManager<Component*> cl) {
	ZoneScoped;

	for (int i = 0; i < cl.size(); i++) {
		if (cl[i].test()) {
			cl[i].value->Update();
		}
	}
}

void EntityAdmin::Update() {
	ZoneScoped;

	if(!skip) controller.Update();
	if(!skip) mainCamera->Update();
	
	TIMER_RESET(t_a); 
	if (!skip && !pause_phys && !paused)  { UpdateLayer(freeCompLayers[ComponentLayer_Physics]); }
	DengTime->physLyrTime =   TIMER_END(t_a); TIMER_RESET(t_a);
	if (!skip && !pause_phys && !paused)  { physics->Update(); }
	DengTime->physSysTime =   TIMER_END(t_a); TIMER_RESET(t_a);
	if (!skip && !pause_canvas)           { UpdateLayer(freeCompLayers[ComponentLayer_Canvas]); }
	DengTime->canvasLyrTime = TIMER_END(t_a); TIMER_RESET(t_a);
	if (!skip && !pause_canvas)           { canvas->Update(); }
	DengTime->canvasSysTime = TIMER_END(t_a); TIMER_RESET(t_a);
	if (!skip && !pause_console)          { UpdateLayer(freeCompLayers[ComponentLayer_World]); }
	DengTime->worldLyrTime =  TIMER_END(t_a); TIMER_RESET(t_a);
	if (!skip && !pause_world && !paused) { world->Update(); }
	DengTime->worldSysTime =  TIMER_END(t_a); TIMER_RESET(t_a);
	if (!skip && !pause_sound && !paused) { UpdateLayer(freeCompLayers[ComponentLayer_Sound]); }
	DengTime->sndLyrTime =    TIMER_END(t_a); TIMER_RESET(t_a);
	if (!skip && !pause_sound && !paused) { sound->Update(); }
	DengTime->sndSysTime =    TIMER_END(t_a); TIMER_RESET(t_a);
	if (!skip && !pause_last && !paused)  { UpdateLayer(freeCompLayers[ComponentLayer_LAST]); }
	DengTime->lastLyrTime =   TIMER_END(t_a);
	
	if (DengTime->physSysTime > 5000) {
		pause_phys = true;
		ERROR("Physics system took longer than 5 seconds to run, pausing.");
	}
	DengTime->paused = paused;
	DengTime->phys_pause = pause_phys;
	skip = false;
}

struct SaveHeader{
	u32 magic;
	u32 flags;
	//u32 cameraOffset;
	u32 entityCount;
	u32 entityArrayOffset;
	u32 meshCount;
	u32 meshArrayOffset;
	u32 componentTypeCount;
	u32 componentTypeHeaderArrayOffset;
};

void EntityAdmin::Save() {
	//std::vector<char> save_data(4096);
	
	//open file
	std::string filepath = deshi::dirData() + "save.desh";
	std::ofstream file(filepath, std::ios::out | std::ios::binary | std::ios::trunc);
	if(!file.is_open()){ ERROR("Failed to open file '", filepath, "' when trying to save"); return; }
	
	SaveHeader header{};
	file.write((const char*)&header, sizeof(SaveHeader)); //zero fill header
	header.magic = 1213416772; //DESH
	header.flags = 0;
	
	//// entities ////
	header.entityCount       = entities.size();
	header.entityArrayOffset = sizeof(SaveHeader);
	
	//store sorted components and write entities
	header.componentTypeCount = 8;
	std::vector<AudioListener*>  compsAudioListener;
	std::vector<AudioSource*>    compsAudioSource;
	std::vector<BoxCollider*>    compsColliderBox;
	std::vector<AABBCollider*>   compsColliderAABB;
	std::vector<SphereCollider*> compsColliderSphere;
	std::vector<Light*>          compsLight;
	std::vector<MeshComp*>       compsMeshComp;
	std::vector<Physics*>        compsPhysics;
	//TODO(delle,Cl) convert these vectors to char vectors and when iterating thru entities
	// and thier components, call the save function of an entity to add to the components
	// vector and then use the final size of that vector for type header offsets
	
	for(auto& e : entities) {
		//write entity
		file.write((const char*)&e.id,                 sizeof(u32));
		file.write(e.name,                             sizeof(char)*64);
		file.write((const char*)&e.transform.position, sizeof(Vector3));
		file.write((const char*)&e.transform.rotation, sizeof(Vector3));
		file.write((const char*)&e.transform.scale,    sizeof(Vector3));
		
		//sort components
		for(auto c : e.components) {
			if(dyncast(d, MeshComp, c)) {
				compsMeshComp.push_back(d);
			}else if(dyncast(d, Physics, c)) {
				compsPhysics.push_back(d);
			}else if(dyncast(col, Collider, c)){
				if (dyncast(d, BoxCollider, col)){
					compsColliderBox.push_back(d);
				}else if(dyncast(d, AABBCollider, col)){
					compsColliderAABB.push_back(d);
				}else if(dyncast(d, SphereCollider, col)){
					compsColliderSphere.push_back(d);
				}else{
					ERROR("Unhandled collider component '", c->name, "' found when attempting to save");
				}
			}else if(dyncast(d, AudioListener, c)){
				compsAudioListener.push_back(d);
			}else if(dyncast(d, AudioSource, c)){
				compsAudioSource.push_back(d);
			}else{
				ERROR("Unhandled component '", c->name, "' found when attempting to save");
			}
		}
	}
	
	//// write meshes ////
	header.meshCount = DengRenderer->meshes.size();
	header.meshArrayOffset = file.tellp();
	
	for(auto& m : DengRenderer->meshes){
		b32 base = m.base;
		file.write((const char*)&m.id, sizeof(u32));
		file.write((const char*)&base, sizeof(b32));
		file.write(m.name,             sizeof(char)*64);
	}
	
	//// write component type headers //// //TODO(delle) move these to thier respective files
	header.componentTypeHeaderArrayOffset = file.tellp();
	ComponentTypeHeader typeHeader;
	
	//audio listener
	typeHeader.type        = ComponentType_AudioListener;
	typeHeader.arrayOffset = header.componentTypeHeaderArrayOffset + sizeof(ComponentTypeHeader) * header.componentTypeCount;
	typeHeader.size        = sizeof(u32) + sizeof(Vector3)*3;
	typeHeader.count       = compsAudioListener.size();
	file.write((const char*)&typeHeader, sizeof(ComponentTypeHeader));
	
	//audio source
	typeHeader.type        = ComponentType_AudioSource;
	typeHeader.arrayOffset = typeHeader.arrayOffset + typeHeader.size * typeHeader.count;
	typeHeader.size        = sizeof(u32) + 0; //TODO(sushi) tell delle what data is important to save on a source
	typeHeader.count       = compsAudioSource.size();
	file.write((const char*)&typeHeader, sizeof(ComponentTypeHeader));
	
	//TODO(delle) update when colliders have triggers
	//collider box
	typeHeader.type        = ComponentType_ColliderBox;
	typeHeader.arrayOffset = typeHeader.arrayOffset + typeHeader.size * typeHeader.count;
	typeHeader.size        = sizeof(u32) + sizeof(u32) + sizeof(Matrix3) + sizeof(Vector3);
	typeHeader.count       = compsColliderBox.size();
	file.write((const char*)&typeHeader, sizeof(ComponentTypeHeader));
	
	//collider aabb
	typeHeader.type        = ComponentType_ColliderAABB;
	typeHeader.arrayOffset = typeHeader.arrayOffset + typeHeader.size * typeHeader.count;
	typeHeader.size        = sizeof(u32) + sizeof(u32) + sizeof(Matrix3) + sizeof(Vector3);
	typeHeader.count       = compsColliderAABB.size();
	file.write((const char*)&typeHeader, sizeof(ComponentTypeHeader));
	
	//collider sphere
	typeHeader.type        = ComponentType_ColliderSphere;
	typeHeader.arrayOffset = typeHeader.arrayOffset + typeHeader.size * typeHeader.count;
	typeHeader.size        = sizeof(u32) + sizeof(u32) + sizeof(Matrix3) + sizeof(float);
	typeHeader.count       = compsColliderSphere.size();
	file.write((const char*)&typeHeader, sizeof(ComponentTypeHeader));
	
	//light
	typeHeader.type        = ComponentType_Light;
	typeHeader.arrayOffset = typeHeader.arrayOffset + typeHeader.size * typeHeader.count;
	typeHeader.size        = sizeof(u32) + sizeof(Vector3)*2 + sizeof(float);
	typeHeader.count       = compsLight.size();
	file.write((const char*)&typeHeader, sizeof(ComponentTypeHeader));
	
	//mesh comp
	typeHeader.type        = ComponentType_MeshComp;
	typeHeader.arrayOffset = typeHeader.arrayOffset + typeHeader.size * typeHeader.count;
	typeHeader.size        = sizeof(u32) + sizeof(u32)*2 + sizeof(b32)*2; //instanceID, meshID, visible, entity_control
	typeHeader.count       = compsMeshComp.size();
	file.write((const char*)&typeHeader, sizeof(ComponentTypeHeader));
	
	//physics
	typeHeader.type        = ComponentType_Physics;
	typeHeader.arrayOffset = typeHeader.arrayOffset + typeHeader.size * typeHeader.count;
	typeHeader.size        = sizeof(u32) + sizeof(Vector3)*6 + sizeof(float)*2 + sizeof(b32);
	typeHeader.count       = compsPhysics.size();
	file.write((const char*)&typeHeader, sizeof(ComponentTypeHeader));
	
	//// write components ////
	
	//audio listener
	for(auto c : compsAudioListener){
		file.write((const char*)&c->entity->id,  sizeof(u32));
		file.write((const char*)&c->position,    sizeof(Vector3));
		file.write((const char*)&c->velocity,    sizeof(Vector3));
		file.write((const char*)&c->orientation, sizeof(Vector3));
	}
	
	//audio source
	for(auto c : compsAudioSource){
		file.write((const char*)&c->entity->id, sizeof(u32));
	}
	
	//collider box
	for(auto c : compsColliderBox){
		file.write((const char*)&c->entity->id,     sizeof(u32));
		file.write((const char*)&c->collisionLayer, sizeof(u32));
		file.write((const char*)&c->inertiaTensor,  sizeof(Matrix3));
		file.write((const char*)&c->halfDims,       sizeof(Vector3));
	}
	
	//collider aabb
	for(auto c : compsColliderAABB){
		file.write((const char*)&c->entity->id,     sizeof(u32));
		file.write((const char*)&c->collisionLayer, sizeof(u32));
		file.write((const char*)&c->inertiaTensor,  sizeof(Matrix3));
		file.write((const char*)&c->halfDims,       sizeof(Vector3));
	}
	
	//collider sphere
	for(auto c : compsColliderSphere){
		file.write((const char*)&c->entity->id,     sizeof(u32));
		file.write((const char*)&c->collisionLayer, sizeof(u32));
		file.write((const char*)&c->inertiaTensor,  sizeof(Matrix3));
		file.write((const char*)&c->radius,         sizeof(float));
	}
	
	//light
	for(auto c : compsLight){
		file.write((const char*)&c->entity->id, sizeof(u32));
		file.write((const char*)&c->position,   sizeof(Vector3));
		file.write((const char*)&c->direction,  sizeof(Vector3));
		file.write((const char*)&c->strength,   sizeof(float));
	}
	
	//mesh comp
	for(auto c : compsMeshComp){
		b32 bool1 = c->mesh_visible;
		b32 bool2 = c->ENTITY_CONTROL;
		file.write((const char*)&c->entity->id,     sizeof(u32));
		file.write((const char*)&c->instanceID,     sizeof(u32));
		file.write((const char*)&c->meshID,         sizeof(u32));
		file.write((const char*)&bool1,             sizeof(b32));
		file.write((const char*)&bool2,             sizeof(b32));
	}
	
	//physics
	for(auto c : compsPhysics){
		b32 isStatic = c->isStatic;
		file.write((const char*)&c->entity->id,      sizeof(u32));
		file.write((const char*)&c->position,        sizeof(Vector3));
		file.write((const char*)&c->rotation,        sizeof(Vector3));
		file.write((const char*)&c->velocity,        sizeof(Vector3));
		file.write((const char*)&c->acceleration,    sizeof(Vector3));
		file.write((const char*)&c->rotVelocity,     sizeof(Vector3));
		file.write((const char*)&c->rotAcceleration, sizeof(Vector3));
		file.write((const char*)&c->elasticity,      sizeof(float));
		file.write((const char*)&c->mass,            sizeof(float));
		file.write((const char*)&isStatic,           sizeof(b32));
	}
	
	//store camera's size so we know offset to following entities list then store camera
	//int camsize = sizeof(*mainCamera);
	//deshi::appendFileBinary(file, (const char*)&camsize, sizeof(int));
	//deshi::appendFileBinary(file, (const char*)mainCamera, camsize);
	
	
	//finish header
	file.seekp(0);
	file.write((const char*)&header, sizeof(SaveHeader));
	
	//// close file ////
	file.close();
}

void EntityAdmin::Load(const char* filename) {
	//// clear current stuff ////
	entities.clear(); entities.reserve(1000);
	for (auto& layer : freeCompLayers) { layer.clear(); } //TODO(delle) see if this causes a leak
	
	selectedEntity = 0;
	undoManager.Reset();
	scene.Reset();
	DengRenderer->Reset();
	DengRenderer->LoadScene(&scene);
	
	SUCCESS("Cleaned up previous level");
	SUCCESS("Loading level: ", filename);
	TIMER_START(t_l);
	
	//// read file to char array ////
	u32 cursor = 0;
	std::vector<char> file = deshi::readFileBinary(filename);
	const char* data = file.data();
	if(!data) return;
	
	//check for magic number
	u32 magic = 1213416772; //DESH
	if(memcmp(data, &magic, 4) != 0) return ERROR("Invalid magic number when loading save file: ", filename);
	
	//// parse header ////
	SaveHeader header;
	memcpy(&header, data+cursor, sizeof(SaveHeader)); cursor += sizeof(SaveHeader);
	
	//// parse and create entities ////
	if(cursor != header.entityArrayOffset) {
		return ERROR("Load failed because cursor was at '", cursor, 
					 "' when reading entities which start at '", header.entityArrayOffset, "'");
	}
	
	entities.reserve(header.entityCount);
	Entity tempEntity;
	for_n(i, header.entityCount){
		tempEntity.admin = this;
		memcpy(&tempEntity.id, data+cursor, sizeof(u32) + 64 + sizeof(Vector3)*3);
		cursor += sizeof(u32) + 64 + sizeof(Vector3)*3;
		entities.push_back(tempEntity);
	}
	
	//// parse and load/create meshes ////
	if(cursor != header.meshArrayOffset) {
		return ERROR("Load failed because cursor was at '", cursor, 
					 "' when reading meshes which start at '", header.meshArrayOffset, "'");
	}
	
	b32 baseMesh = 0;
	char meshName[64];
	for_n(i, header.meshCount){
		cursor += sizeof(u32);
		memcpy(&baseMesh, data+cursor, sizeof(b32)); 
		cursor += sizeof(b32);
		memcpy(meshName, data+cursor, 64); cursor += 64;
		if(!baseMesh) DengRenderer->CreateMesh(&scene, meshName);
	}
	
	//// parse and create components ////
	if(cursor != header.componentTypeHeaderArrayOffset) {
		return ERROR("Load failed because cursor was at '", cursor, 
					 "' when reading meshes which start at '", header.componentTypeHeaderArrayOffset, "'");
	}
	
	ComponentTypeHeader compHeader;
	for_n(i, header.componentTypeCount){
		cursor = header.componentTypeHeaderArrayOffset + (sizeof(u32)*4)*i;
		memcpy(&compHeader, data+cursor, sizeof(u32)*4);
		cursor = compHeader.arrayOffset;
		
		switch(compHeader.type){
			case(ComponentType_AudioListener):  AudioListener ::Load(entities, data, cursor, compHeader.count); break;
			case(ComponentType_AudioSource):    AudioSource   ::Load(entities, data, cursor, compHeader.count); break;
			case(ComponentType_Camera):         Camera        ::Load(entities, data, cursor, compHeader.count); break;
			case(ComponentType_ColliderBox):    BoxCollider   ::Load(entities, data, cursor, compHeader.count); break;
			case(ComponentType_ColliderAABB):   AABBCollider  ::Load(entities, data, cursor, compHeader.count); break;
			case(ComponentType_ColliderSphere): SphereCollider::Load(entities, data, cursor, compHeader.count); break;
			case(ComponentType_Light):          Light         ::Load(entities, data, cursor, compHeader.count); break;
			case(ComponentType_MeshComp):       MeshComp      ::Load(entities, data, cursor, compHeader.count); break;
			case(ComponentType_OrbManager):     OrbManager    ::Load(entities, data, cursor, compHeader.count); break;
			case(ComponentType_Physics):        Physics       ::Load(entities, data, cursor, compHeader.count); break;
			default:{
				ERROR("Failed to load a component array because of unknown component type '", 
					  compHeader.type, "' at pos: ", cursor);
			}break;
		}
	}
	
	SUCCESS("Finished loading level '", filename, "' in ", TIMER_END(t_l), "ms");
	//skip any ongoing updates
	skip = true;
}

//// Entity ////

Entity::Entity(){
	this->transform = Transform();
	this->name[63] = '\0';
}

Entity::Entity(EntityAdmin* admin, u32 id, Transform transform, const char* name, std::vector<Component*> components){
	this->admin = admin;
	this->id = id;
	this->transform = transform;
	if(name) cpystr(this->name, name, 63);
	for (Component* c : components) this->components.push_back(c);
}

Entity::~Entity() {
	for (Component* c : components) delete c;
}

void Entity::SetName(const char* name){
	if(name) cpystr(this->name, name, 63);
}

void Entity::AddComponent(Component* c) {
	if(!c) return;
	components.push_back(c);
	c->layer_index = admin->freeCompLayers[c->layer].add(c);
	c->entity = this;
	c->admin = this->admin;
}

void Entity::AddComponents(std::vector<Component*> comps) {
	u32 value = this->components.size();
	for (Component* c : comps) {
		if(!c) continue;
		this->components.push_back(c);
		c->layer_index = admin->freeCompLayers[c->layer].add(c);
		c->entity = this;
		c->admin = this->admin;
	}
}

void Entity::RemoveComponent(Component* c) {
	if(!c) return;
	for_n(i,components.size()){
		if(components[i] == c){
			admin->freeCompLayers[c->layer].remove_from(c->layer_index);
			delete c; 
			components.erase(components.begin()+i); 
			return;
		}
	}
}

void Entity::RemoveComponents(std::vector<Component*> comps) {
	while(comps.size()){
		for_n(i,components.size()){
			if(!comps[i]) continue;
			if(components[i] == comps[0]){ 
				delete comps[i]; 
				components.erase(components.begin()+i); 
				comps.erase(components.begin()); 
				break;
			}
		}
	}
}
