#include "Collider.h"

#include "../../EntityAdmin.h"



////////////////////////////////////////////////////////////
// Box Collider
//////////////////////////////////////////////////////////



BoxCollider::BoxCollider(Vector3 halfDimensions, float mass, i8 collisionLayer) {
	this->halfDims = halfDimensions;
	this->collisionLayer = collisionLayer;
	this->isTrigger = false;
	this->inertiaTensor = InertiaTensors::SolidCuboid(2 * abs(halfDims.x), 2 * abs(halfDims.x), 2 * abs(halfDims.x), mass);
	strncpy_s(name, "BoxCollider", 63);
	sortid = 2;
}

BoxCollider::BoxCollider(Vector3 halfDimensions, float mass, bool isTrigger, Command* command, i8 collisionLayer) {
	this->halfDims = halfDimensions;
	this->collisionLayer = collisionLayer;
	this->isTrigger = isTrigger;
	this->command = command;
	if (!isTrigger) {
		this->inertiaTensor = InertiaTensors::SolidCuboid(2 * abs(halfDims.x), 2 * abs(halfDims.x), 2 * abs(halfDims.x), mass);
	}
	strncpy_s(name, "BoxCollider", 63);
	sortid = 2;
}



////////////////////////////////////////////////////////////
// AABB Collider
//////////////////////////////////////////////////////////



AABBCollider::AABBCollider(Vector3 halfDimensions, float mass, i8 collisionLayer) {
	this->halfDims = halfDimensions;
	this->collisionLayer = collisionLayer;
	this->isTrigger = false;
	this->inertiaTensor = InertiaTensors::SolidCuboid(2 * abs(halfDims.x), 2 * abs(halfDims.x), 2 * abs(halfDims.x), mass);
	strncpy_s(name, "AABBCollider", 63);
	sortid = 3;
}

AABBCollider::AABBCollider(Vector3 halfDimensions, float mass, bool isTrigger, Command* command, i8 collisionLayer) {
	this->halfDims = halfDimensions;
	this->collisionLayer = collisionLayer;
	this->isTrigger = isTrigger;
	this->command = command;
	if (!isTrigger) {
		this->inertiaTensor = InertiaTensors::SolidCuboid(2 * abs(halfDims.x), 2 * abs(halfDims.x), 2 * abs(halfDims.x), mass);
	}
	strncpy_s(name, "AABBCollider", 63);
	sortid = 3;
}



////////////////////////////////////////////////////////////
// Sphere Collider
//////////////////////////////////////////////////////////



SphereCollider::SphereCollider(float radius, float mass, i8 collisionLayer) {
	this->radius = radius;
	this->collisionLayer = collisionLayer;
	this->isTrigger = false;
	this->inertiaTensor = InertiaTensors::SolidSphere(radius, mass);
	strncpy_s(name, "SphereCollider", 63);
	sortid = 4;
}

SphereCollider::SphereCollider(float radius, float mass, bool isTrigger, Command* command, i8 collisionLayer) {
	this->radius = radius;
	this->collisionLayer = collisionLayer;
	this->isTrigger = isTrigger;
	this->command = command;
	if (!isTrigger) {
		this->inertiaTensor = InertiaTensors::SolidSphere(radius, mass);
	}
	strncpy_s(name, "SphereCollider", 63);
	sortid = 4;
}