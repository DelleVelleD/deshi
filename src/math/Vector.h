#pragma once
#ifndef DESHI_VECTOR_H
#define DESHI_VECTOR_H

#include <string>

struct Vector2;
struct Vector3;
struct Vector4;
struct MatrixN;
struct Matrix3;
struct Matrix4;
struct ImVec2;
struct Quaternion;
struct Vertex;

//////////////////////
//// declarations ////
//////////////////////

struct Vector2 {
	float x{}, y{};
	
	Vector2(){};
	Vector2(float inX, float inY);
	Vector2(const Vector2& v);
	Vector2(float* ptr);
	
	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 UP;
	static const Vector2 DOWN;
	static const Vector2 LEFT;
	static const Vector2 RIGHT;
	static const Vector2 UNITX;
	static const Vector2 UNITY;
	
	void    operator =  (const Vector2& rhs);
	Vector2 operator *  (float rhs) const;
	void    operator *= (float rhs);
	Vector2 operator /  (float rhs) const;
	void    operator /= (float rhs);
	Vector2 operator +  (const Vector2& rhs) const;
	void    operator += (const Vector2& rhs);
	Vector2 operator -  (const Vector2& rhs) const;
	void    operator -= (const Vector2& rhs);
	Vector2 operator *  (const Vector2& rhs) const;
	void    operator *= (const Vector2& rhs);
	Vector2 operator /  (const Vector2& rhs) const;
	void    operator /= (const Vector2& rhs);
	Vector2 operator -  () const;
	bool    operator == (const Vector2& rhs) const;
	bool    operator != (const Vector2& rhs) const;
	friend Vector2 operator * (float lhs, const Vector2& rhs) { return   rhs * lhs; }
	
	Vector2 absV() const;
	Vector2 copy() const;
	float   dot(const Vector2& rhs) const;
	Vector2 cross(const Vector2& rhs) const;
	float   mag() const;
	void    normalize();
	Vector2 normalized() const;
	void    clampMag(float min, float max);
	Vector2 clampedMag(float min, float max) const;
	float   distanceTo(const Vector2& rhs) const;
	Vector2 compOn(Vector2 rhs);
	float   projectOn(Vector2 rhs);
	Vector2 midpoint(Vector2 rhs);
	Vector2 xComp() const;
	Vector2 yComp() const;
	Vector2 xInvert() const;
	Vector2 yInvert() const;
	const std::string str() const;
	const std::string str2f() const;
	
	//vector interactions
	Vector2(const Vector3& v);
	Vector2(const Vector4& v);
	Vector3 ToVector3() const;
	Vector4 ToVector4() const;
	ImVec2  ToImVec2() const;
};
typedef Vector2 vec2;
#include "Vector2.inl"

struct Vector3 {
	float x{}, y{}, z{};
	
	Vector3(){};
	Vector3(float inX, float inY, float inZ);
	Vector3(float inX, float inY);
	Vector3(const Vector3& v);
	Vector3(float* ptr);
	
	static const Vector3 ZERO;
	static const Vector3 ONE;
	static const Vector3 RIGHT;
	static const Vector3 LEFT;
	static const Vector3 UP;
	static const Vector3 DOWN;
	static const Vector3 FORWARD;
	static const Vector3 BACK;
	static const Vector3 UNITX;
	static const Vector3 UNITY;
	static const Vector3 UNITZ;
	
	void    operator =  (const Vector3& rhs);
	void    operator =  (Vector3& rhs);
	Vector3 operator *  (float rhs) const;
	void    operator *= (float rhs);
	Vector3 operator /  (float rhs) const;
	void    operator /= (float rhs);
	Vector3 operator +  (const Vector3& rhs) const;
	void    operator += (const Vector3& rhs);
	Vector3 operator -  (const Vector3& rhs) const;
	void    operator -= (const Vector3& rhs);
	Vector3 operator *  (const Vector3& rhs) const;
	void    operator *= (const Vector3& rhs);
	Vector3 operator /  (const Vector3& rhs) const;
	void    operator /= (const Vector3& rhs);
	Vector3 operator *  (const Quaternion& rhs) const;
	Vector3 operator -  () const;
	bool    operator == (const Vector3& rhs) const;
	bool    operator != (const Vector3& rhs) const;
	friend Vector3 operator * (float lhs, const Vector3& rhs) { return   rhs * lhs; }
	
	Vector3 absV() const;
	Vector3 copy() const;
	float   dot(const Vector3& rhs) const;
	Vector3 cross(const Vector3& rhs) const;
	float   mag() const;
	void	normalize();
	Vector3 normalized() const;
	Vector3 clamp(float lo, float hi);
	void    clampMag(float min, float max);
	Vector3 clampedMag(float min, float max) const;
	float   distanceTo(const Vector3& rhs) const;
	Vector3 compOn(Vector3 rhs);
	float   projectOn(Vector3 rhs);
	Vector3 midpoint(Vector3 rhs);
	Vector3 xComp() const;
	Vector3 yComp() const;
	Vector3 zComp() const;
	Vector3 xInvert() const;
	Vector3 yInvert() const;
	Vector3 zInvert() const;
	const std::string str() const;
	const std::string str2f() const;
	
	//vector interactions
	Vector3(const Vector2& v);
	Vector3(const Vector4& v);
	Vector2 ToVector2() const;
	Vector4 ToVector4() const;
	
	//matrix interactions
	Vector3 operator *  (const Matrix3& rhs) const;
	void    operator *= (const Matrix3& rhs);
	Vector3 operator *  (const Matrix4& rhs) const;
	void    operator *= (const Matrix4& rhs);
	Vector3 ProjectionMultiply(Matrix4 projection) const;
	MatrixN ToM1x3() const;
	MatrixN ToM1x4(float w) const;
};
typedef Vector3 vec3;
#include "Vector3.inl"

struct Vector4 {
	float x{}, y{}, z{}, w{};
	
	Vector4(){};
	Vector4(float inX, float inY, float inZ, float inW);
	Vector4(const Vector4& v);
	Vector4(float* ptr);
	
	static const Vector4 ZERO;
	static const Vector4 ONE;
	
	void    operator =	(const Vector4& rhs);
	Vector4 operator *  (const float& rhs) const;
	void    operator *= (const float& rhs);
	Vector4 operator /  (const float& rhs) const;
	void    operator /= (const float& rhs);
	Vector4 operator +  (const Vector4& rhs) const;
	void    operator += (const Vector4& rhs);
	Vector4 operator -  (const Vector4& rhs) const;
	void    operator -= (const Vector4& rhs);
	Vector4 operator *  (const Vector4& rhs) const;
	void    operator *= (const Vector4& rhs);
	Vector4 operator /  (const Vector4& rhs) const;
	void    operator /= (const Vector4& rhs);
	Vector4 operator -  () const;
	bool    operator == (const Vector4& rhs) const;
	bool    operator != (const Vector4& rhs) const;
	friend Vector4 operator * (const float& lhs, const Vector4& rhs) { return rhs * lhs; }
	
	Vector4 absV() const;
	Vector4 copy() const;
	float   dot(const Vector4& rhs) const;
	float   mag() const;
	Vector4 normalized() const;
	Vector4 xComp() const;
	Vector4 yComp() const;
	Vector4 zComp() const;
	Vector4 wComp() const;
	Vector4 xInvert() const;
	Vector4 yInvert() const;
	Vector4 zInvert() const;
	Vector4 wInvert() const;
	const std::string str() const;
	const std::string str2f() const;
	
	//vector interactions
	Vector4(const Vector2& v, float z, float w);
	Vector4(const Vector3& v, float w);
	Vector3 ToVector3() const;
	void takeVec3(Vector3 v);
	
	//matrix interactions
	Vector4 operator *  (const Matrix4& rhs) const;
	void    operator *= (const Matrix4& rhs);
};
typedef Vector4 vec4;
#include "Vector4.inl"

//////////////////////
//// interactions ////
//////////////////////

//// Vector2 ////

inline Vector2::
Vector2(const Vector3& v){
	x = v.x; y = v.y;
}

inline Vector2::
Vector2(const Vector4& v){
	x = v.x; y = v.y;
}

inline ImVec2 Vector2::
ToImVec2() const {
	return ImVec2(x, y);
}

inline Vector3 Vector2::
ToVector3() const {
	return Vector3(x, y, 0);
}

inline Vector4 Vector2::
ToVector4() const {
	return Vector4(x, y, 0, 0);
}

//// Vector3 ////

inline Vector3::
Vector3(const Vector2& v) {
	x = v.x; y = v.y; z = 0;
}

inline Vector3::
Vector3(const Vector4& v) {
	x = v.x; y = v.y; z = v.z;
}

inline Vector2 Vector3::
ToVector2() const {
	return Vector2(x, y);
}

inline Vector4 Vector3::
ToVector4() const {
	return Vector4(x, y, z, 1);
}

//// Vector4 ////

inline Vector4::
Vector4(const Vector2& v, float inZ, float inW) {
	x = v.x; y = v.y; z = inZ; w = inW;
}

inline Vector4::
Vector4(const Vector3& v, float inW) {
	x = v.x; y = v.y; z = v.z; w = inW;
}

inline Vector3 Vector4::
ToVector3() const {
	return Vector3(x, y, z);
}

//takes data from a vector3 and leaves w alone
inline void Vector4::
takeVec3(Vector3 v) {
	x = v.x; y = v.y; z = v.z;
}

/////////////////
//// defines ////
/////////////////

#define RANDVEC(a) Vector3(rand() % a + 1, rand() % a + 1, rand() % a + 1)

//averages a vector v over an interval i and returns that average
#define V_AVG(i, v) ([&] { \
static std::vector<Vector3> vectors; \
static Vector3 nv; \
static int iter = 0; \
if(i == vectors.size()){ \
vectors.erase(vectors.begin()); \
vectors.push_back(v); \
iter++; \
} \
else{ \
vectors.push_back(v); \
iter++; \
}\
if(iter == i){ \
nv = Math::averageVector3(vectors); \
iter = 0; \
} \
return nv; \
}())

//averages vectors but consistently returns the value
#define V_AVGCON(i, v) ([&] { \
static std::vector<Vector3> vectors; \
static Vector3 nv; \
if(i == vectors.size()){ \
vectors.erase(vectors.begin()); \
vectors.push_back(v); \
} \
else{ \
vectors.push_back(v); \
} \
nv = Math::averageVector3(vectors); \
return nv; \
}())

//this stores an input vector and returns the previously stored vector
//if you pass true for the second param it will replace the stored vector and return it
//else it just returns the stored vector
#define V_STORE(v, t) ([&]()->Vector3{static Vector3 vect[1];\
Vector3 vr = vect[0];\
if(t){ vect[0] = v; return vr; } \
else return vr; }\
())

#endif //DESHI_VECTOR_H