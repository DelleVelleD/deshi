#pragma once
#ifndef COMPONENT_PHYSICS_H
#define COMPONENT_PHYSICS_H

#include "Component.h"
#include "../../math/Vector.h"


//only used for 2D currently
struct Physics;
struct Manifold {
	Physics* a = nullptr;
	Physics* b = nullptr;

	int refID = 0;
	Vector2 colpoints[2];
	float depth[2];
	int nColPoints = 0;

	Vector2 norm;
};

struct Physics : public Component {
	Vector3 position;
	Vector3 rotation;
	
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 rotVelocity;
	Vector3 rotAcceleration;
	
	float elasticity; //less than 1 in most cases
	float mass;
	
	std::vector<Vector3> forces;
	Vector3 inputVector = Vector3::ZERO;
	
	bool isStatic = false;
	//bool staticRotation = false;
	//TODO(delle,Ph) separate static movement and rotation
	bool twoDphys = false;
	
	Physics();
	Physics(Vector3 position, Vector3 rotation, Vector3 velocity = Vector3::ZERO, Vector3 acceleration = Vector3::ZERO,
			Vector3 rotVeloctiy = Vector3::ZERO,Vector3 rotAcceleration = Vector3::ZERO, float elasticity = .5f, 
			float mass = 1.f, bool isStatic = false);
	Physics(Vector3 position, Vector3 rotation, float mass, float elasticity);
	
	//adds the input vector to the target's input vector
	void AddInput(Vector3 input);
	
	//changes acceleration by adding a force to target, target also applies the impulse to creator
	void AddForce(Physics* creator, Vector3 force);
	
	//if no creator, assume air friction; if creator, assume sliding friction
	//TODO(delle,Ph) change air friction to calculate for shape of object
	void AddFrictionForce(Physics* creator, float frictionCoef);
	
	//changes velocity by adding an impulse to target, target also applies the impulse to creator
	void AddImpulse(Physics* creator, Vector3 impulse);
	void AddImpulseNomass(Physics* creator, Vector3 impulse);
	
	std::vector<char> Save() override;
	static void Load(std::vector<Entity>& entityArray, const char* fileData, u32& cursor, u32 countToLoad);
};

#endif //COMPONENT_PHYSICS_H