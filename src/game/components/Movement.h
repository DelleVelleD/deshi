#pragma once
#ifndef COMPONENT_MOVEMENT_H
#define COMPONENT_MOVEMENT_H

#include "Component.h"
#include "../../math/Vector.h"

//TODO(sushi) add different states such as Stationary, Walking, Running, etc. and probably make them so we can do bit masking to check for multiple states
enum MoveState {
	InAir, OnGround
};

struct Physics;

struct Movement : public Component {
	Vector3 position;
	Vector3 inputs;

	//pointer to player's physics
	Physics* phys;

	MoveState moveState;

	float gndAccel = 10;
	float airAccel = 100;

	Movement(Physics* phys);

	void Update() override;

	std::vector<char> Save() override;
	static void Load(std::vector<Entity>& entityArray, const char* fileData, u32& cursor, u32 countToLoad);
};





#endif