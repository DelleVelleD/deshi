#pragma once
#ifndef SYSTEM_PHYSICS_H
#define SYSTEM_PHYSICS_H

#include "System.h"

enum struct CollisionDetectionMode {
	DISCRETE, /*CONTINUOUS, GJK,*/ NONE
};

enum struct IntegrationMode {
	/*RK4, VERLET,*/ EULER
};

struct PhysicsSystem : public System {
	IntegrationMode integrationMode      = IntegrationMode::EULER;
	CollisionDetectionMode collisionMode = CollisionDetectionMode::DISCRETE;
	
	float gravity     = -9.81f;
	float frictionAir = 0.01f; //TODO(delle,Ph) this should depend on object shape
	
	float maxVelocity    = 100.f;
	float minVelocity    = 0.005f;
	float maxRotVelocity = 360.f; //per axis in degrees
	float minRotVelocity = 1.f;
	
	void Update() override;
};

#endif //SYSTEM_PHYSICS_H