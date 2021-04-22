#include "PhysicsSystem.h"
#include "../../core/console.h"
#include "../../math/Math.h"
#include "../../geometry/Geometry.h"

#include "../Transform.h"
#include "../components/Physics.h"
#include "../components/Collider.h"
#include "../components/AudioSource.h"

struct PhysicsTuple { 
	Transform* transform = nullptr; 
	Physics*   physics   = nullptr; 
	Collider*  collider  = nullptr;
	PhysicsTuple(Transform* transform, Physics* physics, Collider* collider) 
		: transform(transform), physics(physics), collider(collider) {}
};

/////////////////////
//// integration ////
/////////////////////

std::vector<PhysicsTuple> GetPhysicsTuples(EntityAdmin* admin) {
	std::vector<PhysicsTuple> out;
	for(auto& e : admin->entities) {
		Transform* transform = &e.transform;
		Physics*   physics   = nullptr;
		Collider*  collider  = nullptr;
		
		for(Component* c : e.components) {
			if(Physics* phy = dynamic_cast<Physics*>(c)) { physics = phy; continue; }
			if(Collider* col = dynamic_cast<Collider*>(c)) { collider = col; continue; }
		}
		if(physics) {
			out.push_back(PhysicsTuple(transform, physics, collider));
		}
	}
	return out;
}

//TODO(delle,Ph) look into bettering this physics tick
//https://gafferongames.com/post/physics_in_3d/
inline void PhysicsTick(PhysicsTuple& t, PhysicsSystem* ps, Time* time) {
	ZoneScoped;

	//// translation ////
	
	//add input forces
	t.physics->inputVector.normalize();
	t.physics->AddForce(nullptr, t.physics->inputVector);
	t.physics->inputVector = Vector3::ZERO;
	
	//add gravity 
	t.physics->AddForce(nullptr, Vector3(0, ps->gravity, 0));
	
	//add temp air friction force
	t.physics->AddFrictionForce(nullptr, ps->frictionAir);
	
	//sum up forces to calculate acceleration
	Vector3 netForce;
	for(auto& f : t.physics->forces) {
		netForce += f;
	}
	t.physics->acceleration = netForce / t.physics->mass * 50;
	
	//update linear movement and clamp it to min/max velocity
	if (!t.physics->isStatic) {
		t.physics->velocity += t.physics->acceleration * time->fixedDeltaTime;
		float velMag = t.physics->velocity.mag();
		if (velMag > ps->maxVelocity) {
			t.physics->velocity /= velMag;
			t.physics->velocity *= ps->maxVelocity;
		}
		else if (velMag < ps->minVelocity) {
			t.physics->velocity = Vector3::ZERO;
			t.physics->acceleration = Vector3::ZERO;
		}
		t.physics->position += t.physics->velocity * time->fixedDeltaTime;
	}
	
	
	//// rotation ////
	
	//make fake rotational friction
	if(t.physics->rotVelocity != Vector3::ZERO) {
		t.physics->rotAcceleration = Vector3(t.physics->rotVelocity.x > 0 ? -1 : 1, t.physics->rotVelocity.y > 0 ? -1 : 1, t.physics->rotVelocity.z > 0 ? -1 : 1) * ps->frictionAir * t.physics->mass * 100;
	}
	
	//update rotational movement and scuffed vector rotational clamping
	t.physics->rotVelocity += t.physics->rotAcceleration * time->fixedDeltaTime;
	//if(t.physics->rotVelocity.x > ps->maxRotVelocity) {
	//	t.physics->rotVelocity.x = ps->maxRotVelocity;
	//} else if(t.physics->rotVelocity.x < -ps->maxRotVelocity) {
	//	t.physics->rotVelocity.x = -ps->maxRotVelocity;
	//} else if(abs(t.physics->rotVelocity.x) < ps->minRotVelocity) {
	//	t.physics->rotVelocity.x = 0;
	//	t.physics->rotAcceleration.x = 0;
	//}
	//if(t.physics->rotVelocity.y > ps->maxRotVelocity) {
	//	t.physics->rotVelocity.y = ps->maxRotVelocity;
	//} else if(t.physics->rotVelocity.y < -ps->maxRotVelocity) {
	//	t.physics->rotVelocity.y = -ps->maxRotVelocity;
	//} else if(abs(t.physics->rotVelocity.y) < ps->minRotVelocity) {
	//	t.physics->rotVelocity.y = 0;
	//	t.physics->rotAcceleration.y = 0;
	//}
	//if(t.physics->rotVelocity.z > ps->maxRotVelocity) {
	//	t.physics->rotVelocity.z = ps->maxRotVelocity;
	//} else if(t.physics->rotVelocity.z < -ps->maxRotVelocity) {
	//	t.physics->rotVelocity.z = -ps->maxRotVelocity;
	//} else if(abs(t.physics->rotVelocity.z) < ps->minRotVelocity) {
	//	t.physics->rotVelocity.z = 0;
	//	t.physics->rotAcceleration.z = 0;
	//}
	t.physics->rotation += t.physics->rotVelocity * time->fixedDeltaTime;
	
	//reset accelerations
	t.physics->forces.clear();
}

/////////////////////
//// collisions  ////
/////////////////////

Matrix4 LocalToWorldInertiaTensor(Physics* physics, Matrix3 inertiaTensor) {
	Matrix4 inverseTransformation = Matrix4::TransformationMatrix(physics->position, physics->rotation, Vector3::ONE).Inverse();
	return inverseTransformation.Transpose() * inertiaTensor.To4x4() * inverseTransformation;
}

inline void AABBAABBCollision(Physics* obj1, AABBCollider* obj1Col, Physics* obj2, AABBCollider* obj2Col) {
	ZoneScoped;
	
	//calculate min and max values over each axis
	vec3 max1 = obj1->position + obj1Col->halfDims;
	vec3 min1 = obj1->position - obj1Col->halfDims;
	vec3 max2 = obj2->position + obj2Col->halfDims;
	vec3 min2 = obj2->position - obj2Col->halfDims;
	
	if (//check if overlapping
		(min1.x <= max2.x && max1.x >= min2.x) &&
		(min1.y <= max2.y && max1.y >= min2.y) &&
		(min1.z <= max2.z && max1.z >= min2.z)) {
		
		float xover, yover, zover;
		
		//we need to know which box is in front over each axis so
		//the overlap is correct
		if (max1.x < max2.x) xover = max1.x - min2.x;
		else                 xover = max2.x - min1.x;
		if (max1.y < max2.y) yover = max1.y - min2.y;
		else                 yover = max2.y - min1.y;
		if (max1.z < max2.z) zover = max1.z - min2.z;
		else                 zover = max2.z - min1.z;
		
		//TODO( sushi,So) find a nicer way to determine how loud a collision sound is	 
		//obj1->entity->GetComponent<AudioSource>()->RequestPlay(obj1->velocity.mag() + obj2->velocity.mag());
		
		bool xf = false; bool yf = false; bool zf = false;
		
		//static resolution
		if (xover < yover && xover < zover) {
			if(!obj1->isStatic) obj1->position.x += xover / 2;
			if(!obj2->isStatic) obj2->position.x -= xover / 2;
			xf = true;
		}	
		else if (yover < xover && yover < zover) {
			if(!obj1->isStatic) obj1->position.y += yover / 2;
			if(!obj2->isStatic) obj2->position.y -= yover / 2;
			yf = true;
		}
		else if (zover < yover && zover < xover) {
			if(!obj1->isStatic) obj1->position.z += zover / 2;
			if(!obj2->isStatic) obj2->position.z -= zover / 2;
			zf = true;
		}
		
		//dynamic resolution
		//TODO(sushi, Ph) finish implementing AABB dynamic collision
		//get relative velocity
		Vector3 relvel = obj2->velocity - obj1->velocity;
		
		
	}
	
}

inline void AABBSphereCollision(Physics* aabb, AABBCollider* aabbCol, Physics* sphere, SphereCollider* sphereCol) {
	ZoneScoped;
	
	Vector3 aabbPoint = Geometry::ClosestPointOnAABB(aabb->position, aabbCol->halfDims, sphere->position);
	Vector3 vectorBetween = aabbPoint - sphere->position; //sphere towards aabb
	float distanceBetween = vectorBetween.mag();
	if(distanceBetween < sphereCol->radius) {
		if(!aabbCol->command && !sphereCol->command) {
			//SUCCESS("collision happened");
			//aabb->entity->GetComponent<AudioSource>()->request_play = true;
			//static resolution
			if (aabbPoint == sphere->position) { 
				//NOTE if the closest point is the same, the vector between will have no direction; this 
				//is supposed to be a remedy to that by offsetting in the direction between thier centers
				vectorBetween = aabb->position - sphere->position;
			}
			float overlap = .5f * (sphereCol->radius - distanceBetween);
			Vector3 normal = -vectorBetween.normalized();
			vectorBetween = -normal * overlap;
			if(aabb->isStatic && aabb->isStatic){
				//do nothing b/c neither can move
			}else if(aabb->isStatic){
				sphere->position -= vectorBetween;
			}else if(sphere->isStatic){
				aabb->position += vectorBetween;
			}else{
				aabb->position += vectorBetween / 2.f;
				sphere->position -= vectorBetween / 2.f;
			}
			
			//dynamic resolution
			Matrix4 sphereInertiaTensorInverse = LocalToWorldInertiaTensor(sphere, sphereCol->inertiaTensor).Inverse();
			Vector3 ra = sphere->position + Geometry::ClosestPointOnSphere(sphere->position, sphereCol->radius, aabbPoint);
			Vector3 sphereAngularVelocityChange = normal.cross(ra);
			sphereAngularVelocityChange *= sphereInertiaTensorInverse;
			float inverseMassA = 1.f / sphere->mass;
			float scalar = inverseMassA + sphereAngularVelocityChange.cross(ra).dot(normal);
			
			Matrix4 aabbInertiaTensorInverse = LocalToWorldInertiaTensor(aabb, aabbCol->inertiaTensor).Inverse();
			Vector3 rb = aabb->position + aabbPoint;
			Vector3 aabbAngularVelocityChange = normal.cross(rb);
			aabbAngularVelocityChange *= aabbInertiaTensorInverse;
			float inverseMassB = 1.f / aabb->mass; 
			scalar += inverseMassB + aabbAngularVelocityChange.cross(rb).dot(normal);
			
			float coefRest = (aabb->elasticity + sphere->elasticity); //this is completely unfounded is science :)
			float impulseMod = (coefRest + 1) * (sphere->velocity - aabb->velocity).mag(); //this too :)
			Vector3 impulse = normal * impulseMod;
			aabb->AddImpulse(sphere, -impulse);
			sphere->rotVelocity -= sphereAngularVelocityChange;
			//aabb->entity->rotVelocity -= aabbAngularVelocityChange; //we dont do this because AABB shouldnt rotate
		}
	}
}

inline void AABBBoxCollision(Physics* aabb, AABBCollider* aabbCol, Physics* box, BoxCollider* boxCol) {
	ERROR("AABB-Box collision not implemented in PhysicsSystem.cpp");
}

inline void SphereSphereCollision(Physics* s1, SphereCollider* sc1, Physics* s2, SphereCollider* sc2) {
	ZoneScoped;

	//static resolution
	float dist = (s1->position - s2->position).mag();
	float rsum = sc1->radius + sc2->radius;
	if (rsum > dist) {
		Vector3 s1t2 = s2->position - s1->position;
		float overlap = (rsum - dist) / 2;
		if (!s1->isStatic) s1->position -= s1t2.normalized() * overlap;
		if (!s2->isStatic) s2->position += s1t2.normalized() * overlap;

		//dynamic resolution
		//from https://www.gamasutra.com/view/feature/131424/pool_hall_lessons_fast_accurate_.php?print=1
		Vector3 n = s1->position - s2->position;
		n.normalize();

		float a1 = s1->velocity.dot(n);
		float a2 = s2->velocity.dot(n);

		float opP = (2 * (a1 - a2)) / (s1->mass + s2->mass);

		s1->velocity = s1->velocity - opP * s2->mass * n;
		s2->velocity = s2->velocity + opP * s1->mass * n;
	}


}

inline void SphereLandscapeCollision(Physics* s, SphereCollider* sc, Physics* ls, SphereCollider* lsc) {
	
}

inline void SphereBoxCollision(Physics* sphere, SphereCollider* sphereCol, Physics* box, BoxCollider* boxCol) {
	ERROR("Sphere-Box collision not implemented in PhysicsSystem.cpp");
}

inline void BoxBoxCollision(Physics* box, BoxCollider* boxCol, Physics* other, BoxCollider* otherCol) {
	ERROR("Box-Box collision not implemented in PhysicsSystem.cpp");
}

//NOTE make sure you are using the right physics component, because the collision 
//functions dont check that the provided one matches the tuple
inline void CheckCollision(PhysicsTuple& tuple, PhysicsTuple& other) {
	ZoneScoped;

	switch(tuple.collider->type){
		case(ColliderType_Box):
		switch(other.collider->type){
		case(ColliderType_Box): {
			ZoneName("Box v Box", sizeof("Box v Box")); BoxBoxCollision(tuple.physics, (BoxCollider*)tuple.collider,
				other.physics, (BoxCollider*)other.collider); }break;
		case(ColliderType_Sphere): {
			ZoneName("Box v Sphere", sizeof("Box v Sphere")); SphereBoxCollision(other.physics, (SphereCollider*)other.collider,
				tuple.physics, (BoxCollider*)tuple.collider); }break;
		case(ColliderType_AABB): {
			ZoneName("Box v AABB", sizeof("Box v AABB")); AABBBoxCollision(other.physics, (AABBCollider*)other.collider,
				tuple.physics, (BoxCollider*)tuple.collider); }break;
		}break;
		case(ColliderType_Sphere):
			switch (other.collider->type) {
			case(ColliderType_Box): {
				ZoneName("Sphere v Box", sizeof("Sphere v Box")); SphereBoxCollision(tuple.physics, (SphereCollider*)tuple.collider,
					other.physics, (BoxCollider*)other.collider); }break;
			case(ColliderType_Sphere): {
				ZoneName("Sphere v Sphere", sizeof("Sphere v Sphere")); SphereSphereCollision(tuple.physics, (SphereCollider*)tuple.collider,
					other.physics, (SphereCollider*)other.collider); }break;
			case(ColliderType_AABB): {
				ZoneName("Sphere v AABB", sizeof("Sphere v AABB")); AABBSphereCollision(other.physics, (AABBCollider*)other.collider,
					tuple.physics, (SphereCollider*)tuple.collider); }break;
			}break;
		case(ColliderType_AABB):
			switch (other.collider->type) {
			case(ColliderType_Box): {
				ZoneName("AABB v Box", sizeof("AABB v Box")); AABBBoxCollision(tuple.physics, (AABBCollider*)tuple.collider,
					other.physics, (BoxCollider*)other.collider); }break;
			case(ColliderType_Sphere): {
				ZoneName("AABB v Sphere", sizeof("AABB v Sphere")); AABBSphereCollision(tuple.physics, (AABBCollider*)tuple.collider,
					other.physics, (SphereCollider*)other.collider); }break;
			case(ColliderType_AABB): {
				ZoneName("AABB v AABB", sizeof("AABB v AABB")); AABBAABBCollision  (tuple.physics, (AABBCollider*)  tuple.collider,
															other.physics, (AABBCollider*)  other.collider); }break;
		}break;
	}
}

inline void CollisionTick(std::vector<PhysicsTuple>& tuples, PhysicsTuple& t){
	if(t.collider) {
		for(auto& tuple : tuples) {
			if(&t != &tuple && tuple.collider && t.collider->collisionLayer == tuple.collider->collisionLayer) {
				CheckCollision(t, tuple);
			}
		}
	}
}

//////////////////////////
//// system functions ////
//////////////////////////

void PhysicsSystem::Update() {
	ZoneScoped;

	std::vector<PhysicsTuple> tuples = GetPhysicsTuples(admin);
	
	//update physics extra times per frame if frame time delta is larger than physics time delta
	while(DengTime->fixedAccumulator >= DengTime->fixedDeltaTime) {
		for(auto& t : tuples) {
			PhysicsTick(t, this, DengTime);
			CollisionTick(tuples, t);
		}
		DengTime->fixedAccumulator -= DengTime->fixedDeltaTime;
		DengTime->fixedTotalTime += DengTime->fixedDeltaTime;
	}
	
	//interpolate between new physics position and old transform position by the leftover time
	float alpha = DengTime->fixedAccumulator / DengTime->fixedDeltaTime;
	for(auto& t : tuples) {
		t.transform->prevPosition = t.transform->position;
		t.transform->prevRotation = t.transform->rotation;
		t.transform->position = t.transform->position * (1.f - alpha) + t.physics->position * alpha;
		t.transform->rotation = t.transform->rotation * (1.f - alpha) + t.physics->rotation * alpha;
		//t.transform->rotation = Quaternion::QuatSlerp(t.transform->rotation, t.transform->prevRotation, alpha).ToVector3();
		
		//t.transform->rotation *= Matrix4::RotationMatrixAroundPoint(t.transform->position, t.transform->rotation*(1.f - alpha) + t.physics->rotation*alpha);
		//TODO(delle,Ph) look into better rotational interpolation once we switch to quaternions
	}
}
