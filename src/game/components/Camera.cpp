#include "Camera.h"
#include "../../core.h"
#include "../../EntityAdmin.h"
#include "../../math/Math.h"
#include "../../scene/Scene.h"
#include "../Keybinds.h"

Camera::Camera(EntityAdmin*a, float fov, float nearZ, float farZ, bool freeCam) : Component(a) {
	this->nearZ = nearZ;
	this->farZ = farZ;
	this->fov = fov;
	this->freeCamera = freeCam;
	
	forward = (Vector3::FORWARD * Matrix4::RotationMatrix(rotation)).normalized();
	right = Vector3::UP.cross(forward).normalized();
	up = right.cross(forward).normalized();
	
	viewMatrix = Math::LookAtMatrix(position, position + forward).Inverse();
	UpdateProjectionMatrix();
	
	DengRenderer->UpdateCameraViewMatrix(viewMatrix);
	DengRenderer->UpdateCameraPosition(position);
	
	cpystr(name, "Camera", 63);
	layer = ComponentLayer_NONE;
}

void Camera::UseOrthographicProjection() {
}

Matrix4 Camera::MakePerspectiveProjection(){
	float renderDistance = farZ - nearZ;
	float aspectRatio = f32(DengWindow->height) / f32(DengWindow->width);
	float fovRad = 1.f / tanf(RADIANS(fov * .5f));
	return Matrix4( //NOTE setting (1,1) to negative flips the y-axis
				   aspectRatio * fovRad, 0,	   0,							  0,
				   0,					-fovRad, 0,							  0,
				   0,					0,	   farZ / renderDistance,		  1,
				   0,					0,	   -(farZ*nearZ) / renderDistance, 0);
}

Matrix4 Camera::MakeOrthographicProjection() {
	std::pair<Vector3, Vector3> bbox = admin->scene.SceneBoundingBox();
	
	//convert bounding box to camera space
	Vector3 maxcam = Math::WorldToCamera3(bbox.first,  admin->mainCamera->viewMatrix);
	Vector3 mincam = Math::WorldToCamera3(bbox.second, admin->mainCamera->viewMatrix); 
	
	//make screen box from camera space bounding box
	float maxx = std::max(fabs(mincam.x), fabs(maxcam.x));
	float maxy = std::max(fabs(mincam.y), fabs(maxcam.y));
	float max  = std::max(maxx, maxy);
	
	float aspectRatio = (float)DengWindow->width / DengWindow->height;
	float r = max * aspectRatio, t = max;
	float l = -r, b = -t;
	
	static float zoom = 10;
	static float oloffsetx = 0;
	static float oloffsety = 0;
	static float offsetx = 0;
	static float offsety = 0;
	static Vector2 initmouse;
	static bool initoffset = false;
	
	
	//orthographic view controls
	if (DengInput->KeyPressed(DengKeys.orthoZoomIn) && zoom != 1) zoom -= 1;
	if (DengInput->KeyPressed(DengKeys.orthoZoomOut)) zoom += 1;
	
	if (DengInput->KeyPressed(DengKeys.orthoOffset)) initoffset = true;
	
	if (DengInput->KeyDown(DengKeys.orthoOffset)) {
		if (initoffset) {
			initmouse = DengInput->mousePos;
			initoffset = false;
		}
		offsetx = 0.02 * (DengInput->mousePos.x - initmouse.x);
		offsety = 0.02 * (DengInput->mousePos.y - initmouse.y);
	}
	
	if (DengInput->KeyReleased(DengKeys.orthoOffset)) {
		oloffsetx += offsetx; oloffsety += offsety;
		offsetx = 0; offsety = 0;
	}
	
	if (DengInput->KeyPressed(DengKeys.orthoResetOffset)) {
		oloffsetx = 0; oloffsety = 0;
	}
	
	r += zoom * aspectRatio; l = -r;
	r -= offsetx + oloffsetx; l -= offsetx + oloffsetx;
	t += zoom; b -= zoom;
	t += offsety + oloffsety; b += offsety + oloffsety;
	
	float f = admin->mainCamera->farZ;
	float n = admin->mainCamera->nearZ;
	
	return Matrix4(2/(r-l),      0,            0,            0,
				   0,            2/(b-t),      0,            0,
				   0,            0,            -2/(f-n),     0,
				   -(r+l)/(r-l), -(t+b)/(b-t), -(f+n)/(n-f), 1);
}

void Camera::UpdateProjectionMatrix(){
	switch(type){
		case(CameraType::PERSPECTIVE):default:{ projectionMatrix = MakePerspectiveProjection(); } break;
		case(CameraType::ORTHOGRAPHIC):{ projectionMatrix = MakeOrthographicProjection(); }break;
	}
	DengRenderer->UpdateCameraProjectionMatrix(projectionMatrix);
}

std::string Camera::str(){
	std::string camType;
	switch(type){
		case(CameraType::PERSPECTIVE):{ camType = "Perspective"; } break;
		case(CameraType::ORTHOGRAPHIC):{ camType = "Orthographic"; }break;
	}
	return TOSTRING("[c:yellow]Camera Info:[c]",
					"\nPosition: ", position,
					"\nRotation: ", rotation,
					"\nNear Plane: ", nearZ,
					"\nFar Plane: ", farZ,
					"\nHorizontal FOV: ", fov,
					"\nType: ", camType,
					"\nStatic: ", (freeCamera)? "false" : "true");
}

void Camera::Update() {
	ZoneScoped;

	if(freeCamera){
		Window* window = DengWindow;
		Renderer* renderer = DengRenderer;
		
		//clamp camera rotation
		rotation.x = Math::clamp(rotation.x, -89.9f, 89.9f);
		if(rotation.y > 1440.f || rotation.y < -1440.f){ rotation.y = 0.f; }
		
		//update direction vectors
		forward = (Vector3::FORWARD * Matrix4::RotationMatrix(rotation)).normalized();
		right = Vector3::UP.cross(forward).normalized();
		up = right.cross(forward).normalized();
		
		target = position + forward;
		
		viewMatrix = Math::LookAtMatrix(position, target).Inverse();
		
		//update renderer camera properties
		if (type == CameraType::ORTHOGRAPHIC) {
			
			switch (orthoview) {
				case FRONT:    position = Vector3(0, 0, -999); rotation = Vector3(0, 0, 0);   break;
				case BACK:     position = Vector3(0, 0, 999);  rotation = Vector3(0, 180, 0); break;
				case RIGHT:    position = Vector3(999, 0, 0);  rotation = Vector3(0, -90, 0);  break;
				case LEFT:     position = Vector3(-999, 0, 0); rotation = Vector3(0, 90, 0); break;
				case TOPDOWN:  position = Vector3(0, 999, 0);  rotation = Vector3(89.9, 0, 0); break;
				case BOTTOMUP: position = Vector3(0, -999, 0); rotation = Vector3(-89.9, 0, 0);  break;
			}
			UpdateProjectionMatrix();
		}
		renderer->UpdateCameraViewMatrix(viewMatrix);
		renderer->UpdateCameraPosition(position);
		
	}
}

std::vector<char> Camera::Save() {
	std::vector<char> out;
	return out;
}

void Camera::Load(std::vector<Entity>& entities, const char* data, u32& cursor, u32 count){
	ERROR("Camera load function not setup");
}