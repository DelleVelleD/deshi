#pragma once
#ifndef COMPONENT_MESHCOMP_H
#define COMPONENT_MESHCOMP_H

#include "Component.h"

struct Mesh;

struct MeshComp : public Component {
	Mesh* mesh;
	u32 instanceID;
	u32 meshID;
	bool mesh_visible = true;
	bool ENTITY_CONTROL = true;
	
	MeshComp();
	MeshComp(u32 meshID, u32 instanceID = 0);
	MeshComp(Mesh* m, u32 meshID = 0, u32 instanceID = 0);
	
	void ToggleVisibility();
	void Visible(bool visible);
	void UpdateMeshTransform(Vector3 position, Vector3 rotation, Vector3 scale);
	
	void ChangeMaterialShader(u32 s);
	void ChangeMaterialTexture(u32 t);
	
	void ReceiveEvent(Event event) override;
	
	void Init() override;
	void Update() override;
	
	std::vector<char> Save() override;
	static void Load(std::vector<Entity>& entityArray, const char* fileData, u32& cursor, u32 countToLoad);
};


#endif //COMPONENT_MESHCOMP_H