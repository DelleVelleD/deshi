#include "MeshComp.h"
#include "../../core.h"
#include "../Transform.h"
#include "../../scene/Model.h"
#include "../../scene/Scene.h"

#include "../systems/CanvasSystem.h"

#include "../../EntityAdmin.h"

MeshComp::MeshComp() {
	this->mesh = 0;
	this->meshID = -1;
	this->instanceID = -1
		;
	cpystr(name, "MeshComp", 63);
	send = new Sender();
	layer = ComponentLayer_Canvas;
}

MeshComp::MeshComp(u32 meshID, u32 instanceID) {
	this->mesh = 0;
	this->meshID = meshID;
	this->instanceID = instanceID;
	
	cpystr(name, "MeshComp", 63);
	send = new Sender();
	layer = ComponentLayer_Canvas;
}

MeshComp::MeshComp(Mesh* m, u32 meshID, u32 instanceID) {
	this->mesh = m;
	this->meshID = meshID;
	this->instanceID = instanceID;
	
	cpystr(name, "MeshComp", 63);
	send = new Sender();
	layer = ComponentLayer_Canvas;
}

void MeshComp::Init() {
	if(!mesh) mesh = DengRenderer->GetMeshPtr(meshID);
	//NOTE ideally we check for an existing mesh pointer, but 
	// something is filling the mesh pointer with bad food
}

void MeshComp::ToggleVisibility() {
	mesh_visible = !mesh_visible;
	DengRenderer->UpdateMeshVisibility(meshID, mesh_visible);
}

void MeshComp::Visible(bool visible) {
	mesh_visible = visible;
	DengRenderer->UpdateMeshVisibility(meshID, mesh_visible);
}

void MeshComp::ReceiveEvent(Event event) {
	switch (event) {
		case TEST_EVENT:
		LOG("MeshComp receieved event");
		break;
	}
}

void MeshComp::ChangeMaterialShader(u32 s) {
	std::vector<u32> ids = DengRenderer->GetMaterialIDs(meshID);
	for (u32 id : ids) {
		DengRenderer->UpdateMaterialShader(id, s);
	}
}

void MeshComp::ChangeMaterialTexture(u32 t) {
	std::vector<u32> ids = DengRenderer->GetMaterialIDs(meshID);
	
	for (u32 id : ids) {
		DengRenderer->UpdateMaterialTexture(id, 0, t);
	}
}

//this should only be used when the entity is not controlling the Mesh
void MeshComp::UpdateMeshTransform(Vector3 position, Vector3 rotation, Vector3 scale) {
	DengRenderer->UpdateMeshMatrix(meshID, Matrix4::TransformationMatrix(position, rotation, scale));
}

void MeshComp::Update() {
	ASSERT(mesh->vertexCount, "Mesh has no vertices");
	

	ImGui::BeginDebugLayer();

	Color c1 = Color(15, 30, 50) * 3;
	Color c2 = Color(50, 30, 15) * 3;

	int i = 0;
	for (auto t : mesh->triangles) {
		ImGui::DebugDrawLine3(t->midpoint(), t->nbr[0]->midpoint(), g_admin->mainCamera, DengWindow->dimensions, (i % 2 == 0) ? c1 : c2);
		ImGui::DebugDrawLine3(t->midpoint(), t->nbr[1]->midpoint(), g_admin->mainCamera, DengWindow->dimensions, (i % 2 == 0) ? c1 : c2);
		ImGui::DebugDrawLine3(t->midpoint(), t->nbr[2]->midpoint(), g_admin->mainCamera, DengWindow->dimensions, (i % 2 == 0) ? c1 : c2);
		i++;
	}


	ImGui::EndDebugLayer();

	//update mesh's transform with entities tranform
	if(ENTITY_CONTROL) DengRenderer->UpdateMeshMatrix(meshID, entity->transform.TransformMatrix());
}

std::vector<char> MeshComp::Save() {
	std::vector<char> out;
	return out;
}

void MeshComp::Load(std::vector<Entity>& entities, const char* data, u32& cursor, u32 count){
	u32 entityID = 0xFFFFFFFF;
	u32 instanceID = -1;
	u32 meshID = -1;
	b32 visible = 0;
	b32 entity_control = 0;
	for_n(i,count){
		memcpy(&entityID, data+cursor, sizeof(u32)); 
		cursor += sizeof(u32);
		if(entityID >= entities.size()) {
			ERROR("Failed to load mesh component at pos '", cursor-sizeof(u32),
				  "' because it has an invalid entity ID: ", entityID);
			continue;
		}
		
		memcpy(&instanceID, data+cursor, sizeof(u32)); cursor += sizeof(u32);
		memcpy(&meshID, data+cursor, sizeof(u32)); cursor += sizeof(u32);
		memcpy(&visible, data+cursor, sizeof(b32)); cursor += sizeof(b32);
		memcpy(&entity_control, data+cursor, sizeof(b32)); cursor += sizeof(b32);
		
		MeshComp* mc = new MeshComp();
		mc->mesh = DengRenderer->GetMeshPtr(meshID);
		mc->instanceID = instanceID;
		mc->meshID = meshID;
		mc->mesh_visible = visible;
		mc->ENTITY_CONTROL = entity_control;
		entities[entityID].AddComponent(mc);
	}
}