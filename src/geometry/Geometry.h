#pragma once
#include "../math/Math.h"

namespace Geometry {

	//std::vector<Vector2> GenerateOutline(Mesh* m) {
	//	std::vector<u32> indexArray;
	//	for (auto& b : m->batchArray) {
	//		for (int i = 0; i < b.indexArray.size(); i++) {
	//			indexArray.push_back(b.indexArray[i]);
	//		}
	//	}
	//}


	Vector3 ClosestPointOnAABB(Vector3 center, Vector3 halfDims, Vector3 target) {
		return Vector3(
			fmaxf(center.x - halfDims.x, fminf(target.x, center.x + halfDims.x)),
			fmaxf(center.y - halfDims.y, fminf(target.y, center.y + halfDims.y)),
			fmaxf(center.y - halfDims.z, fminf(target.z, center.z + halfDims.z)));
	}

	Vector3 ClosestPointOnSphere(Vector3 center, float radius, Vector3 target) {
		return (target - center).normalized() * radius;
	}

	Vector3 ClosestPointOnBox(Vector3 center, Vector3 halfDims, Vector3 rotation, Vector3 target) {
		target *= Matrix4::RotationMatrixAroundPoint(center, rotation).Inverse(); //TODO(delle,Geo) test ClosestPointOnBox
		return Vector3(
			fmaxf(center.x - halfDims.x, fminf(target.x, center.x + halfDims.x)),
			fmaxf(center.y - halfDims.y, fminf(target.y, center.y + halfDims.y)),
			fmaxf(center.y - halfDims.z, fminf(target.z, center.z + halfDims.z)));
	}

};