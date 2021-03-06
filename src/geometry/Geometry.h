#pragma once
#include "../math/Math.h"
#include "../scene/Model.h"

namespace Geometry {
	
	//returns the point that's furthest along a normal 
	static u32 FurthestPointAlongNormal(std::vector<Vector3> p, Vector3 n) {
		float furthest = -INFINITY;
		u32 furthestid = 0;
		for (int i = 0; i < p.size(); i++) {
			float dist = n.dot(p[i]);
			if (dist > furthest) {
				furthest = dist;
				furthestid = i;
			}
		}
		return furthestid;
	}

	static u32 FurthestTriangleAlongNormal(Mesh* m, Matrix4 rotation, Vector3 n) {
		float furthest = -INFINITY;
		u32 furthestTriId = 0;
		for (int i = 0; i < m->triangles.size(); i++) {
			Triangle* t = m->triangles[i];
			Vector3 norm = t->norm * rotation;

			float dp = norm.dot(n);
			
			if (dp > furthest) {
				furthestTriId = i;
				furthest = dp;
			}
		}
		
		return furthestTriId;
	}


	static Vector3 ClosestPointOnAABB(Vector3 center, Vector3 halfDims, Vector3 target) {
		return Vector3(
					   fmaxf(center.x - halfDims.x, fminf(target.x, center.x + halfDims.x)),
					   fmaxf(center.y - halfDims.y, fminf(target.y, center.y + halfDims.y)),
					   fmaxf(center.y - halfDims.z, fminf(target.z, center.z + halfDims.z)));
	}
	
	static Vector3 ClosestPointOnSphere(Vector3 center, float radius, Vector3 target) {
		return (target - center).normalized() * radius;
	}
	
	static Vector3 ClosestPointOnBox(Vector3 center, Vector3 halfDims, Vector3 rotation, Vector3 target) {
		target *= Matrix4::RotationMatrixAroundPoint(center, rotation).Inverse(); //TODO(delle,Geo) test ClosestPointOnBox
		return Vector3(
					   fmaxf(center.x - halfDims.x, fminf(target.x, center.x + halfDims.x)),
					   fmaxf(center.y - halfDims.y, fminf(target.y, center.y + halfDims.y)),
					   fmaxf(center.y - halfDims.z, fminf(target.z, center.z + halfDims.z)));
	}
};