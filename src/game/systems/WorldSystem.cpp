#include "WorldSystem.h"
#include "../../core/console.h"
#include "../../EntityAdmin.h"
#include "../../utils/debug.h"
#include "../components/Component.h"
#include "../Transform.h"

void WorldSystem::Update() {
	ZoneScoped;
	//deletion buffer
	for(Entity* entity : deletionBuffer) {
		admin->entities.erase(admin->entities.begin()+entity->id);
	}
	deletionBuffer.clear();
	
	//creation buffer
	for(Entity* e : creationBuffer) {
		admin->entities.emplace_back(admin, (u32)admin->entities.size(), e->transform, e->name, e->components);
		for(Component* c : e->components){ 
			c->entity = &admin->entities[admin->entities.size()-1]; 
			c->Init();
		}
		operator delete(e); //call this to not delete components, but still delete the staging entity (doesnt call destructor)
	}
	creationBuffer.clear();
}

u32 WorldSystem::CreateEntity(EntityAdmin* admin, const char* name) {
	Entity* e = new Entity;
	e->SetName(name);
	e->admin = admin;
	creationBuffer.push_back(e);
	return admin->entities.size() + creationBuffer.size() - 1;
}

u32 WorldSystem::CreateEntity(EntityAdmin* admin, std::vector<Component*> components, const char* name, Transform transform) {
	Entity* e = new Entity;
	e->SetName(name);
	e->admin = admin;
	e->transform = transform;
	e->AddComponents(components);
	creationBuffer.push_back(e);
	return admin->entities.size() + creationBuffer.size() - 1;
}

void WorldSystem::AddEntityToCreationBuffer(EntityAdmin* admin, Entity* entity) {
	entity->admin = admin;
	creationBuffer.push_back(entity);
}

void WorldSystem::AddEntityToDeletionBuffer(EntityAdmin* admin, Entity* entity) {
	if(entity->id < admin->entities.size()){
		deletionBuffer.push_back(entity);
	}else{
		ERROR("Attempted to add entity '", entity->id, "' to deletion buffer when it doesn't exist on the admin");
	}
}
