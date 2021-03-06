#pragma once
#ifndef SYSTEM_PHYSICS_H
#define SYSTEM_PHYSICS_H

#include "../../defines.h"

struct Admin;

enum struct CollisionDetectionMode {
	DISCRETE, /*CONTINUOUS, GJK,*/ NONE
};

enum struct IntegrationMode {
	/*RK4, VERLET,*/ EULER
};

struct PhysicsSystem{
	Admin* admin;
	IntegrationMode integrationMode;
	CollisionDetectionMode collisionMode;
	f32 gravity;
	f32 frictionAir; //TODO(delle,Ph) this should depend on object shape
	f32 maxVelocity;
	f32 minVelocity;
	f32 maxRotVelocity; //per axis in degrees
	f32 minRotVelocity;
	
	u32 collisionCount;
	
	void Init(Admin* admin);
	void Update();
};

#endif //SYSTEM_PHYSICS_H