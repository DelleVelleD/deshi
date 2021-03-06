#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

#include "../../defines.h"
#include "../Event.h"

#include <vector>
#include <string>

struct Entity;
struct Admin;

//suffix is the system that comes after the layer
enum ComponentLayerBits : u32{
	ComponentLayer_NONE = 0,
	ComponentLayer_Physics,
	ComponentLayer_Canvas,
	ComponentLayer_Sound,
	ComponentLayer_World,
	SystemLayer_Physics = 0,
}; typedef u32 ComponentLayer;

struct ComponentTypeHeader{
	u32 type;
	u32 size;
	u32 count;
	u32 arrayOffset;
};

enum ComponentTypeBits : u32{
	ComponentType_NONE              = 0,
	ComponentType_MeshComp          = 1 << 0,
	ComponentType_Physics           = 1 << 1, 
	ComponentType_Collider          = 1 << 2, //TODO(delle,Cl) consolidate these to one collider since we have ColliderType now
	ComponentType_ColliderBox       = 1 << 3,
	ComponentType_ColliderAABB      = 1 << 4,
	ComponentType_ColliderSphere    = 1 << 5,
	ComponentType_ColliderLandscape = 1 << 6,
	ComponentType_AudioListener     = 1 << 7,
	ComponentType_AudioSource       = 1 << 8,
	ComponentType_Camera            = 1 << 9,
	ComponentType_Light             = 1 << 10,
	ComponentType_OrbManager        = 1 << 11,
	ComponentType_Door              = 1 << 12,
	ComponentType_Player            = 1 << 13,
	ComponentType_Movement          = 1 << 14,
}; typedef u32 ComponentType;
global_ const char* ComponentTypeStrings[] = {
	"None", "MeshComp", "Physics", "Collider", "ColliderBox", "ColliderAABB", "ColliderSphere", "ColliderLandscape", "AudioListener", "AudioSource", "Camera", "Light", "OrbManager", "Door", "Player", "Movement"
};

struct Component : public Receiver {
	Admin* admin;
	u32 entityID;
	u32 compID; //this should ONLY be used for saving/loading, not indexing anykind of array for now
	char name[DESHI_NAME_SIZE];
	ComponentType comptype;
	Entity* entity;
	Sender* sender = nullptr; //sender for outputting events to a list of receivers
	Event event = Event_NONE; //event to be sent TODO(sushi) implement multiple events being able to be sent
	ComponentLayer layer = ComponentLayer_NONE;
	int layer_index; //index in the layer for deletion
	
	virtual ~Component() {
		if(sender) sender->RemoveReceiver(this);
	}
	
	void ConnectSend(Component* c) {
		c->sender->AddReceiver(this);
		
	}
	
	void SetEvent(Event event) { this->event = event; }
	
	void SetCompID(u32 id) { compID = id; }
	
	//Init only gets called when this component's entity is spawned thru the world system
	virtual void Init() {};
	virtual void Update() {};
	virtual void ReceiveEvent(Event event) override {};
	
	virtual std::string SaveTEXT() { return ""; };
	
	virtual std::string str(){ return ""; };
};

#endif //COMPONENT_H