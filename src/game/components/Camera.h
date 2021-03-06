#pragma once
#ifndef COMPONENT_CAMERA_H
#define COMPONENT_CAMERA_H

#include "Component.h"
#include "../../math/Vector.h"
#include "../../math/Matrix.h"

enum CameraTypeBits : u32{
	CameraType_Perspective, CameraType_Orthographic
}; typedef u32 CameraType;
global_ const char* CameraTypeStrings[] = {
	"Perspective", "Orthographic"
};

enum OrthoViews {
	RIGHT, LEFT, TOPDOWN, BOTTOMUP, FRONT, BACK
};

struct Camera : public Component {
	Vector3 position{4.f, 3.f, -4.f};
	Vector3 rotation{28.f, -45.f, 0.f};
	float nearZ; //the distance from the camera's position to screen plane
	float farZ; //the maximum render distance
	float fov; //horizontal field of view
	bool freeCamera = true; //whether the camera can move or not (no need to update if false)
	CameraType type = CameraType_Perspective;
	OrthoViews orthoview = FRONT; //TODO(sushi, Cl) combine this with type using bit masking if this is how i decide to keep doing ortho views
	
	Vector3 forward;
	Vector3 right;
	Vector3 up;
	Matrix4 viewMat;
	Matrix4 projMat;
	
	Camera();
	Camera(float fov, float nearZ = .01f, float farZ = 1000.01f, bool freeCam = true);
	
	void Update() override;
	
	//horizontal fov in degrees
	Matrix4 MakePerspectiveProjection();
	Matrix4 MakeOrthographicProjection();
	void UseOrthographicProjection();
	
	void UpdateProjectionMatrix();
	
	std::string str() override;
	std::string SaveTEXT() override;
	static void LoadDESH(Admin* admin, const char* fileData, u32& cursor, u32 countToLoad);
};

#endif //COMPONENT_CAMERA_H