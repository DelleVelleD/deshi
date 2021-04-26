#pragma once
#ifndef DESHI_MATH_H
#define DESHI_MATH_H

#include "../utils/defines.h"
#include "VectorMatrix.h"
#include "MatrixN.h"
#include "Quaternion.h"
#include "../game/components/Camera.h"

#include <math.h>
#include <algorithm>
#include <numeric>

#define F_AVG(i, f) ([&] { \
static std::vector<float> floats; \
static float nf; \
static int iter = 0; \
if(i == floats.size()){ \
floats.erase(floats.begin()); \
floats.push_back(f); \
iter++; \
} \
else{ \
floats.push_back(f); \
iter++; \
}\
if(iter == i){ \
nf = Math::average(floats, floats.size()); \
iter = 0; \
} \
return nf; \
}())

namespace Math {
	static Vector4 ProjMult(Vector4 v, Matrix4 m) {
		Vector4 nv = v * m;
		if (nv.w != 0) { nv.x /= nv.w; nv.y /= nv.w; nv.z /= nv.w; }
		return nv;
	}
	
	static Matrix4 LocalToWorld(Vector3 offsetFromOrigin) {
		return Matrix4::TranslationMatrix(offsetFromOrigin);
	}
	
	static Matrix4 WorldToLocal(Vector3 offsetFromOrigin) {
		return Matrix4::TranslationMatrix(offsetFromOrigin).Inverse();
	}
	
	//returns how far along a polynomial fit for a set of data you are
	//you are not allowed to have 2 points with the same x value here
	//using Lagrange Polynomials
	//TODO(sushi, Ma) look into maybe implementing Newton's Polynomials at some point idk if they're better but they look more simple
	static float PolynomialCurveInterpolation(std::vector<Vector2> vs, float t) {
		float sum = 0;
		for (int j = 0; j < vs.size(); j++) {
			float vy = vs[j].y; 
			float jx = vs[j].x;
			float lbp = 1;
			for (int m = 0; m < vs.size(); m++) {
				if (lbp != 0) {
					if (m != j) {
						float mx = vs[m].x;
						lbp *= (t - mx) / (jx - mx);
					}
				} else break;
			}
			sum += vy * lbp;
		}
		return sum;
	}
	
	static Vector2 Vector2RotateByAngle(float angle, Vector2 v) {
		angle = RADIANS(angle);
		return Vector2(v.x * cosf(angle) - v.y * sinf(angle), v.x * sin(angle) + v.y * cos(angle));
	}
	
}

//// Non-Quaternion vs Quaternion Interactions ////

inline Vector3 Quaternion::operator *(const Vector3& rhs) {
	return (Quaternion(rhs.x, rhs.y, rhs.z, 0) * *this).ToVector3();
}

//TODO(sushi, MaCl) move this and others into their our Quaternion / Quaternion nonQuat file
inline Vector3 Vector3::operator *(const Quaternion& rhs) const {
	return (Quaternion(x, y, z, 0) * rhs).ToVector3();
}

inline Quaternion::Quaternion(const Vector3& rotation) {
	this->RotVecToQuat(rotation);
}

inline Quaternion::Quaternion(const Vector3& axis, float theta) {
	*this = AxisAngleToQuat(theta, axis);
}

//https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles?oldformat=true
inline Vector3 Quaternion::ToVector3() {
	Vector3 angles;
	
	// roll (x-axis rotation)
	double sinr_cosp = 2 * (w * x + y * z);
	double cosr_cosp = 1 - 2 * (x * x + y * y);
	angles.x = atan2(sinr_cosp, cosr_cosp);
	
	// pitch (y-axis rotation)
	double sinp = 2 * (w * y - z * x);
	if (std::abs(sinp) >= 1)
		angles.y = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
	else
		angles.y = asin(sinp);
	
	// yaw (z-axis rotation)
	double siny_cosp = 2 * (w * z + x * y);
	double cosy_cosp = 1 - 2 * (y * y + z * z);
	angles.z = atan2(siny_cosp, cosy_cosp);
	
	return angles;
}

//converts an angle and an axis to a quaternion
//im not sure how this works or where it will be used and im not even sure if its
//set up properly (sorry)
inline Quaternion Quaternion::AxisAngleToQuat(float angle, Vector3 axis) {
	float angler = RADIANS(angle);
	return Quaternion(sinf(angler / 2) * axis.x, sinf(angler / 2) * axis.y, sinf(angler / 2) * axis.z, cosf(angler / 2));
}

//this may be wrong but I think a rotation vector would be 
//Vector3(roll, pitch, yaw)
//https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles?oldformat=true
inline Quaternion Quaternion::RotVecToQuat(Vector3 rotation) {
	//this is probably necessary although he didn't do this in the Gamasutra article
	Vector3 rotationrad = RADIANS(rotation);
	float cy = cos(rotationrad.z * 0.5);
	float sy = sin(rotationrad.z * 0.5);
	float cp = cos(rotationrad.y * 0.5);
	float sp = sin(rotationrad.y * 0.5);
	float cr = cos(rotationrad.x * 0.5);
	float sr = sin(rotationrad.x * 0.5);
	
	Quaternion q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;
	
	return q;
}

inline Quaternion Quaternion::QuatSlerp(Vector3 fromv, Vector3 tov, float t) {
	//this implements Spherical Linear intERPoplation
	//it interpolates between two quaternions along the shortest arc on a sphere formed by them
	//taken from https://www.wikiwand.com/en/Slerp#/Quaternion_Slerp
	
	Quaternion from = RotVecToQuat(fromv);
	Quaternion to = RotVecToQuat(tov);
	
	from.normalize();
	to.normalize();
	
	float dot = to.dot(from);
	
	if (dot < 0) {
		to = -to;
		dot = -dot;
	}
	
	const float dot_thresh = 0.9995;
	
	// calculate coefficients
	if (dot > dot_thresh) {
		// standard case (slerp)
		Quaternion result = from + ((to - from) * t);
		result.normalize();
		return result;
	}
	
	//since dot is in range [0, DOT_THRESHOLD], acos is safe
	double theta_0 = acos(dot);			//theta_0 = angle between input vectors
	double theta = theta_0 * t;			//theta = angle between v0 and result
	double sin_theta = sin(theta);		//compute this value only once
	double sin_theta_0 = sin(theta_0);	//compute this value only once
	
	double s0 = cos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
	double s1 = sin_theta / sin_theta_0;
	
	return (from * s0) + (to * s1);
}

namespace Math {
	
	//ref: https://en.cppreference.com/w/cpp/algorithm/clamp
	static float clamp(float v, float lo, float hi) {
		ASSERT(lo < hi, "The low must be less than the high clamp");
		return (v < lo) ? lo : (hi < v) ? hi : v;
	}
	
	static Vector3 clamp(Vector3 v, float lo, float hi) {
		ASSERT(lo < hi, "The low must be less than the high clamp");
		return v.clamp(lo, hi);
	}
	
	//for debugging with floats or doubles
	static std::string append_decimal(std::string s) {
		while (s.back() != '.') {
			s.pop_back();
		}
		s.pop_back();
		return s;
	}
	
	//append all trailing zeros
	static std::string append_zeroes(std::string s) {
		while (s.back() == '0') {
			s.pop_back();
		}
		s.pop_back();
		return s;
	}
	
	//append to two decimal places
	static std::string append_two_decimal(std::string s) {
		if (s.length() >= 2) {
			while (s.at(s.length() - 4) != '.') {
				s.pop_back();
			}
			s.pop_back();
		}
		return s;
	}
	
	static std::string str2f(float s) {
		char buffer[50];
		std::snprintf(buffer, 50, "%-.2f", s);
		return std::string(buffer);
	}
	
	//round a float to two decimal places
	static float round2f(float f) { return (float)((int)(f * 100 + .5)) / 100; }
	static float round4f(float f) { return (float)((int)(f * 10000 + .5)) / 10000; }

	
	static Vector3 round2v(Vector3 v) {
		return Vector3(
					   (float)((int)(v.x * 100 + .5)) / 100,
					   (float)((int)(v.y * 100 + .5)) / 100,
					   (float)((int)(v.z * 100 + .5)) / 100);
	}
	
	//average any std container probably
	template<class FWIt>
		static float average(FWIt a, const FWIt b, int size) { return std::accumulate(a, b, 0.0) / size; }
	
	template<class T>
		static double average(const T& container, int size) { return average(std::begin(container), std::end(container), size); }
	
	//interpolating
	static float   lerpf(float p1, float p2, float t) { return (1.f - t) * p1 + t * p2; }
	static Vector3 lerpv(Vector3 v1, Vector3 v2, float t) { return  v1 * (1.f - t) + v2 * t; }
	static Vector2 lerpv(Vector2 v1, Vector2 v2, float t) { return  v1 * (1.f - t) + v2 * t; }
	
	static Matrix4 lerpm4(Matrix4 m1, Matrix4 m2, float t) { return m1 * (1.f - t) + m2 * t; }

	//returns in degrees
	static float AngBetweenVectors(Vector3 v1, Vector3 v2) {
		return DEGREES(acosf(v1.dot(v2) / (v1.mag() * v2.mag())));
	}

	//returns in degrees
	static float AngBetweenVectors(Vector2 v1, Vector2 v2) {
		return DEGREES(atan2(v1.x * v2.y - v1.y * v2.x, v1.dot(v2)));
	}

	//this function returns a matrix that tells a vector how to look at a specific point in space.
	static Matrix4 LookAtMatrix(const Vector3& pos, const Vector3& target) {
		if(pos == target) { return LookAtMatrix(pos, target + Vector3(.01f, 0, 0)); }
		
		//get new forward direction
		Vector3 newFor = (target - pos).normalized();
		
		//get right direction
		Vector3 newRight; 
		if(newFor == Vector3::UP || newFor == Vector3::DOWN) { 
			newRight = Vector3::RIGHT; 
		} 
		else {
			newRight = (Vector3::UP.cross(newFor)).normalized(); 
		}
		
		//get up direction
		Vector3 newUp = newFor.cross(newRight); 
		
		//make look-at matrix
		return Matrix4(newRight.x, newRight.y, newRight.z, 0,
					   newUp.x,    -newUp.y,   newUp.z,    0,
					   newFor.x,   newFor.y,   newFor.z,   0,
					   pos.x,      pos.y,      pos.z,      1);
	}


	
	//this ones for getting the up vector back for sound orientation
	static Matrix4 LookAtMatrix(const Vector3& pos, const Vector3& target, Vector3& up) {
		if (pos == target) { return LookAtMatrix(pos, target + Vector3(.01f, 0, 0)); }
		
		Vector3 newFor = (target - pos).normalized();
		
		Vector3 newRight;
		if (newFor == Vector3::UP || newFor == Vector3::DOWN) {
			newRight = Vector3::RIGHT;
		} 
		else {
			newRight = (Vector3::UP.cross(newFor)).normalized();
		}
		
		//get up direction
		up = newFor.cross(newRight);
		
		//make look-at matrix
		return Matrix4(newRight.x, newRight.y, newRight.z, 0,
					   up.x,       -up.y,      up.z,       0,
					   newFor.x,   newFor.y,   newFor.z,   0,
					   pos.x,      pos.y,      pos.z,      1);
	}
	
	//this assumes its in degrees
	static Vector3 SphericalToRectangularCoords(Vector3 v) {
		float y = RADIANS(v.y);
		float z = RADIANS(v.z);
		return Vector3(v.x * sinf(z) * cosf(y), v.x * cosf(z), v.x * sinf(z) * sinf(y));
	}
	
	static Vector3 RectangularToSphericalCoords(Vector3 v) {
		float rho = RADIANS(sqrt(v.mag()));
		float theta = RADIANS(atan(v.y / v.z));
		float phi = acos(v.z / v.mag()); //maybe use v.y instead of v.z because y is our vertical axis
		return Vector3(rho, theta, phi);
		
	}
	
	static float DistTwoPoints(Vector3 a, Vector3 b) {
		return sqrtf(
					 (a.x - b.x) * (a.x - b.x) +
					 (a.y - b.y) * (a.y - b.y) +
					 (a.z - b.z) * (a.z - b.z));
	}
	
	static float DistPointToPlane(Vector3 point, Vector3 plane_n, Vector3 plane_p) {
		return (plane_n.x * point.x + plane_n.y * point.y + plane_n.z * point.z - plane_n.dot(plane_p));
	}
	
	//where a line intersects with a plane, 'returns' how far along line you were as t value
	static Vector3 VectorPlaneIntersect(Vector3 plane_p, Vector3 plane_n, Vector3 line_start, Vector3 line_end, float& t) {
		//plane_n.normalize();
		float plane_d = -plane_n.dot(plane_p);
		float ad = line_start.dot(plane_n);
		float bd = line_end.dot(plane_n);
		t = (-plane_d - ad) / (bd - ad);
		Vector3 line_start_to_end = line_end - line_start;
		Vector3 line_to_intersect = line_start_to_end * t;
		return line_start + line_to_intersect;
	}

	//where a line intersects with a plane
	static Vector3 VectorPlaneIntersect(Vector3 plane_p, Vector3 plane_n, Vector3 line_start, Vector3 line_end) {
		//plane_n.normalize();
		float plane_d = -plane_n.dot(plane_p);
		float ad = line_start.dot(plane_n);
		float bd = line_end.dot(plane_n);
		float t = (-plane_d - ad) / (bd - ad);
		Vector3 line_start_to_end = line_end - line_start;
		Vector3 line_to_intersect = line_start_to_end * t;
		return line_start + line_to_intersect;
	}
	
	//return where two lines intersect on the x axis with slope and the y-intercept
	static Vector2 LineIntersect2(float slope1, float ycross1, float slope2, float ycross2) {
		MatrixN lhs(2,2,{ slope1, ycross1, slope2, ycross2 });
		MatrixN rhs(2,1,{ 1, 1 });
		MatrixN det = lhs.Inverse() * rhs;
		float x = 1 / det(1,0) * det(0,0);
		float y = slope1 * x + ycross1;
		return Vector2(x, y);
	}
	
	//returns where two lines intersect in 3D space //TODO(sushi, MaGe) implement this
	static Vector3 LineIntersect3(Vector3 adir, Vector3 ap, Vector3 bdir, Vector3 bp) {}
	
	//the input vectors should be in viewMatrix/camera space
	//returns true if the line can be rendered after clipping, false otherwise
	static bool ClipLineToZPlanes(Vector3& start, Vector3& end, Camera* camera) {
		//clip to the near plane
		Vector3 planePoint = Vector3(0, 0, camera->nearZ);
		Vector3 planeNormal = Vector3::FORWARD;
		float d = planeNormal.dot(planePoint);
		bool startBeyondPlane = planeNormal.dot(start) - d < 0;
		bool endBeyondPlane = planeNormal.dot(end) - d < 0;
		float t;
		if (startBeyondPlane && !endBeyondPlane) {
			start = Math::VectorPlaneIntersect(planePoint, planeNormal, start, end, t);
		} else if (!startBeyondPlane && endBeyondPlane) {
			end = Math::VectorPlaneIntersect(planePoint, planeNormal, start, end, t);
		} else if (startBeyondPlane && endBeyondPlane) {
			return false;
		}
		
		//clip to the far plane
		planePoint = Vector3(0, 0, camera->farZ);
		planeNormal = Vector3::BACK;
		d = planeNormal.dot(planePoint);
		startBeyondPlane = planeNormal.dot(start) - d < 0;
		endBeyondPlane = planeNormal.dot(end) - d < 0;
		if (startBeyondPlane && !endBeyondPlane) {
			start = Math::VectorPlaneIntersect(planePoint, planeNormal, start, end, t);
		} else if (!startBeyondPlane && endBeyondPlane) {
			end = Math::VectorPlaneIntersect(planePoint, planeNormal, start, end, t);
		} else if (startBeyondPlane && endBeyondPlane) {
			return false;
		}
		return true;
	} //ClipLineToZPlanes
	
	//cohen-sutherland algorithm https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
	//the input vectors should be in screen space
	//returns true if the line can be rendered after clipping, false otherwise
	static bool ClipLineToBorderPlanes(Vector3& start, Vector3& end, Vector2 dimensions) {
		//clip to the vertical and horizontal planes
		const int CLIP_INSIDE = 0;
		const int CLIP_LEFT = 1;
		const int CLIP_RIGHT = 2;
		const int CLIP_BOTTOM = 4;
		const int CLIP_TOP = 8;
		auto ComputeOutCode = [&](Vector3& vertex) {
			int code = CLIP_INSIDE;
			if (vertex.x < 0) {
				code |= CLIP_LEFT;
			}
			else if (vertex.x > dimensions.x) {
				code |= CLIP_RIGHT;
			}
			if (vertex.y < 0) { //these are inverted because we are in screen space
				code |= CLIP_TOP;
			}
			else if (vertex.y > dimensions.y) {
				code |= CLIP_BOTTOM;
			}
			return code;
		};
		
		int lineStartCode = ComputeOutCode(start);
		int lineEndCode = ComputeOutCode(end);
		
		//loop until all points are within or outside the screen zone
		while (true) {
			if (!(lineStartCode | lineEndCode)) {
				//both points are inside the screen zone
				return true;
			}
			else if (lineStartCode & lineEndCode) {
				//both points are in the same outside zone
				return false;
			}
			else {
				float x, y;
				//select one of the points outside
				int code = lineEndCode > lineStartCode ? lineEndCode : lineStartCode;
				
				//clip the points the the screen bounds by finding the intersection point
				if			(code & CLIP_TOP) {		//point is above screen
					x = start.x + (end.x - start.x) * (-start.y) / (end.y - start.y);
					y = 0;
				} 
				else if	(code & CLIP_BOTTOM) {		//point is below screen
					x = start.x + (end.x - start.x) * (dimensions.y - start.y) / (end.y - start.y);
					y = dimensions.y;
				} 
				else if	(code & CLIP_RIGHT) {		//point is right of screen
					y = start.y + (end.y - start.y) * (dimensions.x - start.x) / (end.x - start.x);
					x = dimensions.x;
				} 
				else if	(code & CLIP_LEFT) {		//point is left of screen
					y = start.y + (end.y - start.y) * (-start.x) / (end.x - start.x);
					x = 0;
				}
				
				//update the vector's points and restart loop
				if (code == lineStartCode) {
					start.x = x;
					start.y = y;
					lineStartCode = ComputeOutCode(start);
				}
				else {
					end.x = x;
					end.y = y;
					lineEndCode = ComputeOutCode(end);
				}
			}
		}
	} //ClipLineToBorderPlanes
	
	//NOTE these triangle functions are quite useless I think but I put them here in case
	//since I deleted Triangle.h

	//returns area of a triangle of sides a and b
	static float TriangleArea(Vector3 a, Vector3 b) { return a.cross(b).mag() / 2; }
	
	//The normal this returns heavily depends on how you give it the points
	static Vector3 TriangleNormal(Vector3 p1, Vector3 p2, Vector3 p3) {
		return (p3 - p1).cross(p2 - p1).normalized();
	}

	static Vector3 TriangleMidpoint(Vector3 p1, Vector3 p2, Vector3 p3) {
		return (p1 + p2 + p3) / 3;
	}

	static Vector3 WorldToCamera3(Vector3 vertex, Matrix4 viewMatrix) {
		return Math::ProjMult(vertex.ToVector4(), viewMatrix).ToVector3();
	}
	
	static Vector4 WorldToCamera4(Vector3 vertex, Matrix4 viewMatrix) {
		return Math::ProjMult(vertex.ToVector4(), viewMatrix);
	}
	
	static Vector3 CameraToWorld3(Vector3 vertex, Matrix4 viewMatrix) {
		return Math::ProjMult(vertex.ToVector4(), viewMatrix.Inverse()).ToVector3();
	}
	
	static Vector4 CameraToWorld4(Vector3 vertex, Matrix4 viewMatrix) {
		return Math::ProjMult(vertex.ToVector4(), viewMatrix.Inverse());
	}
	
	static Vector2 CameraToScreen2(Vector3 csVertex, Matrix4 projectionMatrix, Vector2 screenDimensions) {
		Vector3 vm = Math::ProjMult(csVertex.ToVector4(), projectionMatrix).ToVector3();
		vm.x += 1.0f; vm.y += 1.0f;
		vm.x *= 0.5f * (float)screenDimensions.x;
		vm.y *= 0.5f * (float)screenDimensions.y;
		return vm.ToVector2();
	}
	
	static Vector3 CameraToScreen3(Vector3 csVertex, Matrix4 projectionMatrix, Vector2 screenDimensions) {
		Vector3 vm = Math::ProjMult(csVertex.ToVector4(), projectionMatrix).ToVector3();
		vm.x += 1.0f; vm.y += 1.0f;
		vm.x *= 0.5f * (float)screenDimensions.x;
		vm.y *= 0.5f * (float)screenDimensions.y;
		return vm;
	}
	
	static Vector3 CameraToScreen3(Vector3 csVertex, Matrix4 projectionMatrix, Vector2 screenDimensions, float& w) {
		Vector4 bleh = csVertex.ToVector4() * projectionMatrix;
		w = bleh.w;
		Vector3 vm = bleh.normalized().ToVector3();
		vm.x += 1.0f; vm.y += 1.0f;
		vm.x *= 0.5f * (float)screenDimensions.x;
		vm.y *= 0.5f * (float)screenDimensions.y;
		return vm;
	}
	
	static Vector3 CameraToScreen3(Vector4 csVertex, Matrix4 projectionMatrix, Vector2 screenDimensions) {
		Vector3 vm = Math::ProjMult(csVertex, projectionMatrix).ToVector3();
		vm.x += 1.0f; vm.y += 1.0f;
		vm.x *= 0.5f * (float)screenDimensions.x;
		vm.y *= 0.5f * (float)screenDimensions.y;
		return vm;
	}
	
	static Vector4 CameraToScreen4(Vector4 csVertex, Matrix4 projectionMatrix, Vector2 screenDimensions) {
		Vector4 vm = (csVertex * projectionMatrix).normalized();
		vm.x += 1.0f; vm.y += 1.0f;
		vm.x *= 0.5f * (float)screenDimensions.x;
		vm.y *= 0.5f * (float)screenDimensions.y;
		return vm;
	}
	
	static Vector3 WorldToScreen(Vector3 point, Matrix4 ProjMat, Matrix4 ViewMat, Vector2 screenDimensions) {
		return CameraToScreen3(WorldToCamera4(point, ViewMat), ProjMat, screenDimensions);
	}
	
	static Vector2 WorldToScreen2(Vector3 point, Matrix4 ProjMat, Matrix4 ViewMat, Vector2 screenDimensions) {
		Vector3 v = CameraToScreen3(WorldToCamera4(point, ViewMat), ProjMat, screenDimensions);
		return Vector2(v.x, v.y);
	}
	
	static Vector3 ScreenToWorld(Vector2 pos, Matrix4 ProjMat, Matrix4 view, Vector2 screenDimensions) {
		Vector3 out(pos);
		out.x /= .5f * (float)screenDimensions.x;
		out.y /= .5f * (float)screenDimensions.y;
		out.x -= 1.f; out.y -= 1.f; out.z = -1.f;
		out = Math::ProjMult(out.ToVector4(), ProjMat.Inverse()).ToVector3();
		out = Math::ProjMult(out.ToVector4(), view.Inverse()).ToVector3();
		return out;
	}
};

#endif //DESHI_MATH_H