#pragma once
#ifndef SYSTEM_WORLD_H
#define SYSTEM_WORLD_H

#include "System.h"

/*
	Note on how this system is meant to work:

	Rather than dealing with objects being created/deleted in the middle of the frame and
	the consequence that might occur from that, I opted to use a creation/deletion buffer
	that only acts at the end of the frame (after rendering).

	Advantages:
	1. Objects wont get removed/inserted in the middle of the frame, possibly causing
	unexpected behaviour and affecting some systems while avoiding others.
	2. If the frame is going too long, we can opt to delay the creation or deletion of
	an entity or component, though I would say to be careful with delayed creation as players
	might notice even if the creation of something is one or two frames delayed.
	3. You can always add/remove an Entity manually into the middle of the frame through
	the EntityAdmin if necessary

	Disadvantages:
	1. More noticeable delays when on low framerates.
	2. An entity queued for deletion will still affect systems until the end of the frame.
	3. The buffer has potential to grow too large if delays keep getting stacked
*/

struct Entity;
struct Component;
struct EntityAdmin;
struct Transform;

struct WorldSystem : public System {
	void Update() override;
	
	std::vector<Entity*> creationBuffer;
	std::vector<Entity*> deletionBuffer;
	
	//// Entity Lifetime Functions ////
	
	//initializes an entity with no components and adds it to the creation buffer
	//returns the eventual position in the admin's entities array
	u32 CreateEntity(EntityAdmin* admin, const char* name = 0);
	
	//initializes an entity with a component vector and adds it to the creation buffer
	//returns the eventual position in the admin's entities array
	u32 CreateEntity(EntityAdmin* admin, std::vector<Component*> components, const char* name = 0, Transform transform = Transform());
	
	//initializes an entity with a component vector and adds it to entities immedietly
	//returns a pointer to the entitiy
	Entity* CreateEntityNow(EntityAdmin* admin, std::vector<Component*> components, const char* name = 0, Transform transform = Transform());

	//adds an already initialized entity to the creation buffer
	//returns its place in the queue
	void AddEntityToCreationBuffer(EntityAdmin* admin, Entity* entity);
	
	//adds an already initialized entity to the deletion buffer
	//returns its place in the queue, -1 if the entity could not be found
	void AddEntityToDeletionBuffer(EntityAdmin* admin, Entity* entity);
};

#endif //SYSTEM_WORLD_H