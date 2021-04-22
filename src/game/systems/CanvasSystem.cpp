#include "CanvasSystem.h"
#include "../../core.h"
#include "../../math/Math.h"
#include "../../scene/Scene.h"
#include "../../EntityAdmin.h"
#include "PhysicsSystem.h"
#include "WorldSystem.h"
#include "../Keybinds.h"
#include "../Transform.h"
#include "../UndoManager.h"
#include "../components/AudioListener.h"
#include "../components/AudioSource.h"
#include "../components/Camera.h"
#include "../components/Collider.h"
#include "../components/Light.h"
#include "../components/MeshComp.h"
#include "../components/Physics.h"

//for time
#include <iomanip>
#include <sstream>

ImVec4 ColToVec4(Color p) {
	return ImVec4((float)p.r / 255, (float)p.g / 255, (float)p.b / 255, p.a / 255);
}

bool WinHovFlag = false;
float menubarheight = 0;
float debugbarheight = 0;
float debugtoolswidth = 0;

float padding = 0.95;

//defines to make repetitve things less ugly and more editable

//check if mouse is over window so we can prevent mouse from being captured by engine
#define WinHovCheck if (IsWindowHovered()) WinHovFlag = true 

//allows me to manually set padding so i have a little more control than ImGui gives me (I think idk lol)
#define SetPadding SetCursorPosX((GetWindowWidth() - (GetWindowWidth() * padding)) / 2)

//current palette:
//https://lospec.com/palette-list/slso8
//TODO(sushi, Ui) implement menu style file loading sort of stuff yeah
//TODO(sushi, Ui) standardize what UI element each color belongs to
struct {
	Color c1 = Color(0x0d2b45);
	Color c2 = Color(0x203c56);
	Color c3 = Color(0x544e68);
	Color c4 = Color(0x8d697a);
	Color c5 = Color(0xd08159);
	Color c6 = Color(0xffaa5e);
	Color c7 = Color(0xffd4a3);
	Color c8 = Color(0xffecd6);
	Color c9 = Color(20, 20, 20);
}colors;

std::vector<std::string> files;
std::vector<std::string> textures;


//functions to simplify the usage of our DebugLayer
namespace ImGui {
	void BeginDebugLayer() {
		//ImGui::SetNextWindowSize(ImVec2(DengWindow->width, DengWindow->height));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ColToVec4(Color(0, 0, 0, 0)));
		ImGui::Begin("DebugLayer", 0, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
	}
	
	//not necessary, but I'm adding it for clarity in code
	void EndDebugLayer() {
		ImGui::PopStyleColor();
		ImGui::End();
	}
	
	void DebugDrawCircle(Vector2 pos, float radius, Color color) {
		ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(pos.x, pos.y), radius, ImGui::GetColorU32(ColToVec4(color)));
	}
	
	void DebugDrawCircle3(Vector3 pos, float radius, Camera* c, Vector2 windimen, Color color) {
		Vector2 pos2 = Math::WorldToScreen2(pos, c->projectionMatrix, c->viewMatrix, windimen);
		ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(pos.x, pos.y), radius, ImGui::GetColorU32(ColToVec4(color)));
	}
	
	void DebugDrawLine(Vector2 pos1, Vector2 pos2, Color color) {
		ImGui::GetBackgroundDrawList()->AddLine(pos1.ToImVec2(), pos2.ToImVec2(), ImGui::GetColorU32(ColToVec4(color)));
	}
	
	void DebugDrawLine3(Vector3 pos1, Vector3 pos2, Camera* c, Vector2 windimen, Color color) {
		Vector2 pos12 = Math::WorldToScreen2(pos1, c->projectionMatrix, c->viewMatrix, windimen);
		Vector2 pos22 = Math::WorldToScreen2(pos2, c->projectionMatrix, c->viewMatrix, windimen);
		ImGui::GetBackgroundDrawList()->AddLine(pos12.ToImVec2(), pos22.ToImVec2(), ImGui::GetColorU32(ColToVec4(color)));
	}
	
	void DebugDrawText(const char* text, Vector2 pos, Color color) {		
		ImGui::SetCursorPos(pos.ToImVec2());
		
		ImGui::PushStyleColor(ImGuiCol_Text, ColToVec4(color));
		ImGui::Text(text);
		ImGui::PopStyleColor();
	}
	
	void DebugDrawText3(const char* text, Vector3 pos, Camera* c, Vector2 windimen, Color color) {
		Vector2 pos2 = Math::WorldToScreen2(pos, c->projectionMatrix, c->viewMatrix, windimen);
		ImGui::SetCursorPos(pos2.ToImVec2());
		
		ImGui::PushStyleColor(ImGuiCol_Text, ColToVec4(color));
		ImGui::Text(text);
		ImGui::PopStyleColor();
	}
}

//// utility ui elements ///

void CopyButton(const char* text) {
	if(ImGui::Button("Copy")){ ImGui::LogToClipboard(); ImGui::LogText(text); ImGui::LogFinish(); }
}

bool InputVector3(const char* id, Vector3* vecPtr, bool inputUpdate = false) {
	ImGui::SetNextItemWidth(-FLT_MIN);
	if(inputUpdate) {
		return ImGui::InputFloat3(id, (float*)vecPtr); 
	} else {
		return ImGui::InputFloat3(id, (float*)vecPtr, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue); 
	}
}

void AddPadding(float x){
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x);
}

//// major ui elements ////

void CanvasSystem::MenuBar() {
	ZoneScoped;

	using namespace ImGui;
	
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleColor(ImGuiCol_PopupBg,   ColToVec4(Color(20, 20, 20, 255)));
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ColToVec4(Color(20, 20, 20, 255)));
	
	
	if(BeginMainMenuBar()) {
		WinHovCheck; 
		
		menubarheight = GetWindowHeight();
		if(BeginMenu("File")) {
			WinHovCheck; 
			
			if (MenuItem("New")) {
				admin->entities.clear(); admin->entities.reserve(1000);
				for (auto& layer : admin->freeCompLayers) { layer.clear(); } //TODO(delle) see if this causes a leak
				
				admin->selectedEntity = 0;
				admin->undoManager.Reset();
				admin->scene.Reset();
				g_renderer->Reset();
				g_renderer->LoadScene(&admin->scene);
			}
			if (MenuItem("Save")) {
				admin->Save();
			}
			if (MenuItem("Load")) {
				admin->Load("data/save.desh"); //TODO(delle) add dropdown/something for specific file loading
			}
			ImGui::EndMenu();
		}
		if(BeginMenu("Edit")) {
			WinHovCheck; 
			
			if (MenuItem("placeholder")) {
				
			}
			ImGui::EndMenu();
		}
		if(BeginMenu("Spawn")) {
			WinHovCheck; 
			
			for (int i = 0; i < files.size(); i++) {
				if (files[i].find(".obj") != std::string::npos) {
					if(MenuItem(files[i].c_str())) { DengConsole->ExecCommand("load_obj", files[i]); }
				}
			}
			EndMenu();
		}//agh
		if (BeginMenu("Window")) {
			WinHovCheck; 
			
			if (MenuItem("Object Property Menu")) showDebugTools = !showDebugTools;
			if (MenuItem("Debug Bar")) showDebugBar = !showDebugBar;
			if (MenuItem("DebugLayer")) showDebugLayer = !showDebugLayer;
			if (MenuItem("ImGui Demo Window")) showImGuiDemoWindow = !showImGuiDemoWindow;
			EndMenu();
		}
		
		EndMainMenuBar();
	}
	
	PopStyleColor();
	PopStyleColor();
	PopStyleVar();
	PopStyleVar();
	
}

inline void EntitiesTab(EntityAdmin* admin, float fontsize){
	ZoneScoped;

	using namespace ImGui;
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ColToVec4(Color(25, 25, 25)));
	if (BeginChild("entityListScroll", ImVec2(GetWindowWidth() * 0.95, 100), false)) {
		WinHovCheck; 
		if (admin->entities.size() == 0) {
			float time = DengTime->totalTime;
			std::string str1 = "Nothing yet...";
			float strlen1 = (fontsize - (fontsize / 2)) * str1.size();
			for (int i = 0; i < str1.size(); i++) {
				SetCursorPos(ImVec2((GetWindowSize().x - strlen1) / 2 + i * (fontsize / 2), (GetWindowSize().y - fontsize) / 2 + sin(10 * time + cos(10 * time + (i * M_PI / 2)) + (i * M_PI / 2))));
				Text(str1.substr(i, 1).c_str());
			}
		}
		else {
			if (BeginTable("split3", 4, ImGuiTableFlags_BordersInner)) {
				
				std::string str1 = "ID";
				float strlen1 = (fontsize - (fontsize / 2)) * str1.size();
				
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Vis", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Components");
				int counter = 0;
				
				
				for (auto& entity : admin->entities) {
					counter++;
					PushID(counter);
					TableNextRow(); TableNextColumn();
					std::string id = std::to_string(entity.id);
					MeshComp* m = entity.GetComponent<MeshComp>();
					
					//SetCursorPosX((GetColumnWidth() - (fontsize - (fontsize / 2)) * id.size()) / 2);
					if (ImGui::Button(id.c_str())) {
						admin->selectedEntity = &entity;
						if(m) DengRenderer->SetSelectedMesh(m->meshID);
					}
					TableNextColumn();
					
					//TODO(UiEnt, sushi) implement visibility for things other than meshes like lights, etc.
					if (m) {
						if (m->mesh_visible) {
							if (SmallButton("O")) {
								m->ToggleVisibility();
							}
						}
						else {
							if (SmallButton("X")) {
								m->ToggleVisibility();
							}
						}
					}
					else {
						Text("NM");
					}
					
					TableNextColumn();
					Text(TOSTRING(" ", entity.name).c_str());
					static bool rename = false;
					static char buff[64] = {};
					static char ogname[64] = {};
					if (ImGui::IsItemClicked()) {
						rename = true;
						cpystr(buff, entity.name, 63);
						cpystr(ogname, entity.name, 63);
					}
					
					if (rename) {
						if (ImGui::InputText("name input", buff, sizeof(buff), ImGuiInputTextFlags_EnterReturnsTrue)) {
							cpystr(entity.name, buff, 63);
							rename = false;
						}
						if (DengInput->KeyPressed(Key::ESCAPE)) {
							cpystr(entity.name, ogname, 63);
							rename = false;
						}
					}
					
					TableNextColumn();
					//TODO(sushi, Ui) find something better to put here
					Text(TOSTRING(" comps: ", entity.components.size()).c_str());
					PopID();
				}
				
				ImGui::EndTable();
			}
		}
		EndChild();
	}
	ImGui::PopStyleColor();
	
	ImGui::Separator();
	
	if (BeginTabBar("SettingsTabs")) {
		//Selected Entity property editing
		if (BeginTabItem("Ent")) {
			if (admin->selectedEntity) {
				if (BeginChild("SelectedEntityMenu", ImVec2(GetWindowWidth(), 500), true)) {
					WinHovCheck;
					Entity* sel = admin->selectedEntity;
					SetPadding;
					Text(TOSTRING("Selected Entity: ", sel->name).c_str());
					Text("Components: ");
					if (BeginTabBar("ObjectPropertyMenus")) {
						if (BeginTabItem("Obj")) {
							SetCursorPosX((GetWindowWidth() - (GetWindowWidth() * 0.95)) / 2);
							if (BeginChild("ObjMenu", ImVec2(GetWindowWidth()* 0.95, 400), true)) {
								WinHovCheck;
								
								Text("Transform");
								Separator();
								
								SetPadding;
								Text("Position ");
								vec3 oldVec = sel->transform.position;
								SameLine(); if(InputVector3("position", &sel->transform.position)){
									if(Physics* p = sel->GetComponent<Physics>()){
										p->position = sel->transform.position;
										admin->undoManager.AddUndoTranslate(&sel->transform, &oldVec, &p->position);
									}else{
										admin->undoManager.AddUndoTranslate(&sel->transform, &oldVec, &sel->transform.position);
									}
								}
								Separator();
								
								SetPadding;
								Text("Rotation ");
								oldVec = sel->transform.rotation;
								SameLine(); if(InputVector3("rotation", &sel->transform.rotation)){
									if(Physics* p = sel->GetComponent<Physics>()){
										p->rotation = sel->transform.rotation;
										admin->undoManager.AddUndoRotate(&sel->transform, &oldVec, &p->rotation);
									}else{
										admin->undoManager.AddUndoRotate(&sel->transform, &oldVec, &sel->transform.rotation);
									}
								}
								Separator();
								
								SetPadding;
								Text("Scale    ");
								oldVec = sel->transform.scale;
								SameLine(); if(InputVector3("scale", &sel->transform.scale)){
									admin->undoManager.AddUndoScale(&sel->transform, &oldVec, &sel->transform.scale);
								}
								Separator();

								if (Physics* p = sel->GetComponent<Physics>()) {
									SetPadding;
									Text("Velocity ");
									SameLine(); if (InputVector3("velocity", &p->velocity));
									Separator();

									SetPadding;
									Text("Accel    ");
									SameLine(); if (InputVector3("acceleration", &p->acceleration));
									Separator();

									SetPadding;
									Text("RotVel   ");
									SameLine(); if (InputVector3("rvelocity", &p->rotVelocity));
									Separator();

									SetPadding;
									Text("RotAccel ");
									SameLine(); if (InputVector3("racceleration", &p->rotAcceleration));
									Separator();
								}
								EndChild();
							}
							EndTabItem();
						}
						
						//Components menu
						if (BeginTabItem("Comp")) {
							SetCursorPosX((GetWindowWidth() - (GetWindowWidth() * 0.95)) / 2);
							if (BeginChild("SelectedComponentsWindow", ImVec2(GetWindowWidth() * 0.95, 100), true)) {
								WinHovCheck;
								if (ImGui::BeginTable("SelectedComponents", 1)) {
									ImGui::TableSetupColumn("Comp", ImGuiTableColumnFlags_WidthFixed);
									for (Component* c : sel->components) {
										TableNextColumn(); //TableNextRow();
										SetPadding;
										Text(c->name);
										SameLine(CalcItemWidth() + 20);
										if (Button("Del")) {
											sel->RemoveComponent(c);
										}
									}
									ImGui::EndTable();
								}
								EndChild();
							}
							EndTabItem();
						}
						
						//Materials menu
						if (BeginTabItem("Mat")) {
							SetPadding;
							if (BeginChild("SelectedMaterialsWindow", ImVec2(GetWindowWidth() * 0.95, 400), true, ImGuiWindowFlags_NoScrollbar)) {
								WinHovCheck;
								if (BeginTabBar("MaterialTab")) {
									if (BeginTabItem("Shdr")) {
										SetPadding;
										if (BeginChild("ShaderTab", ImVec2(GetWindowWidth() * 0.95, 200), true)) {
											WinHovCheck;
											MeshComp* m = sel->GetComponent<MeshComp>();
											if (m) {
												//TODO(sushi, Ui) set up showing multiple batches shaders when that becomes relevant
												//Text(TOSTRING("Shader: ", shadertostring.at(m->mesh->batchArray[0].shader)).c_str());
												SetPadding;
												if (ImGui::TreeNode("Shader Select")) {
													static int selected = -1;
													for (int i = 0; i < shadertostringint.size(); i++) {
														if (Selectable(shadertostringint[i].c_str(), selected == i)) {
															selected = i;
															for (int iter = 0; iter < sel->components.size(); iter++) {
																if (MeshComp* mc = dynamic_cast<MeshComp*>(sel->components[iter])) {
																	mc->ChangeMaterialShader(stringtoshader.at(shadertostringint[i]));
																}
															}
														}
													}
													TreePop();
												}
											}
											EndChild();
										}
										EndTabItem();
									}
									if (BeginTabItem("Tex")) {
										SetPadding;
										if (BeginChild("TexTab", ImVec2(GetWindowWidth() * 0.95, 200), true)) {
											WinHovCheck;
											MeshComp* m = sel->GetComponent<MeshComp>();
											if (m) {
												if (m->mesh->batchArray.size() > 0) {
													if (m->mesh->batchArray[0].textureArray.size() > 0) {
														Text(TOSTRING("Texture: ", m->mesh->batchArray[0].textureArray[0].filename).c_str());
													}
													else {
														Text("Current Texture: None");
													}
													//TODO(sushi, Ui) immplement showing multiple textures when yeah
													Separator();
													SetPadding;
													
													static int selected = -1;
													for (int i = 0; i < textures.size(); i++) {
														SetPadding;
														if (Selectable(textures[i].c_str(), selected == i)) {
															selected = i;
															for (int iter = 0; iter < sel->components.size(); iter++) {
																if (MeshComp* mc = dynamic_cast<MeshComp*>(sel->components[iter])) {
																	Texture tex(textures[i].c_str(), 0);
																	mc->ChangeMaterialTexture(g_renderer->LoadTexture(tex));
																}
															}
														}
													}
												}
											}
											EndChild();
										}
										EndTabItem();
									}
									EndTabBar();
								}
								EndChild();
							}
							EndTabItem();
						}
						EndTabBar();
					}
					EndChild();
				}
			}
			else {
				SetPadding;
				Text("Selected Entity: None");
			}
			EndTabItem();
		}
		if (BeginTabItem("Cam")) {
			if (BeginChild("SelectedEntityMenu", ImVec2(GetWindowWidth(), 500), true)) {
				WinHovCheck;
				SetPadding;
				
				Camera* c = admin->mainCamera;
				
				Text("Transform");
				Separator();
				
				SetPadding;
				Text("Position ");
				SameLine(); 
				InputVector3("position", &c->position);
				Separator();
				
				SetPadding;
				Text("Rotation ");
				SameLine(); InputVector3("rotation", &c->rotation);
				Separator();
				
				if (Button("Reset camera")) DengConsole->ExecCommand("cam_reset");
				
				Separator();
				
				SetPadding;
				Text("fov:   ");
				SameLine(); ImGui::SetNextItemWidth(-FLT_MIN); 
				if (InputFloat("fov", &c->fov, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsDecimal)) {
					c->UpdateProjectionMatrix();
				}
				Separator();
				
				SetPadding;
				Text("nearZ: ");
				SameLine(); ImGui::SetNextItemWidth(-FLT_MIN); 
				if (InputFloat("nearz", &c->nearZ, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsDecimal)) {
					c->UpdateProjectionMatrix();
				};
				Separator();
				
				SetPadding;
				Text("farZ:  ");
				SameLine(); ImGui::SetNextItemWidth(-FLT_MIN); 
				if (InputFloat("farz", &c->farZ, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsDecimal)) {
					c->UpdateProjectionMatrix();
				};
				Separator();
				
				EndChild();
			}
			EndTabItem();
		}
		
		if (BeginTabItem("Phys")) {
			if (BeginChild("SelectedEntityMenu", ImVec2(GetWindowWidth(), 500), true)) {
				
				SetPadding;
				ImGui::Selectable("Pause", &admin->pause_phys);
				
				SetPadding;
				ImGui::InputFloat("gravity", &admin->physics->gravity);
				
				SetPadding;
				ImGui::Text("#colliders: not implemented");
				
				EndChild();
			}
			EndTabItem();
		}
		
		EndTabBar();
	}
}

inline void CreateTab(EntityAdmin* admin, float fontsize){
	ZoneScoped;

	
	using namespace ImGui;
	
	//// creation variables ////
	const char* presets[] = {"None", "AABB", "Box", "Sphere", "Player"};
	local_persist int current_preset = 0;
	local_persist char entity_name[64] = {};
	local_persist vec3 entity_pos{}, entity_rot{}, entity_scale = Vector3::ONE;
	local_persist bool comp_audiolistener{}, comp_audiosource{}, comp_collider{}, comp_mesh{}, comp_light{}, comp_physics{};
	const char* colliders[] = {"None", "Box", "AABB", "Sphere"};
	local_persist int  collider_type = ColliderType_NONE;
	local_persist vec3 collider_halfdims = Vector3::ONE;
	local_persist f32  collider_radius  = 1.f;
	local_persist const char* mesh_name;
	local_persist u32  mesh_id = -1, mesh_instance_id = 0;
	local_persist f32  light_strength = 1.f;
	local_persist vec3 physics_velocity{}, physics_accel{}, physics_rotVel{}, physics_rotAccel{};
	local_persist f32  physics_elasticity = .5f, physics_mass = 1.f;
	local_persist bool physics_staticPosition{}, physics_staticRotation{};
	u32 entity_id = admin->entities.size();
	
	//// create panel ////
	PushStyleVar(ImGuiStyleVar_IndentSpacing, 5.0f);
	SetPadding; if (BeginChild("CreateMenu", ImVec2(GetWindowWidth() * 0.95f, GetWindowHeight() * .9f), true)) { WinHovCheck;
		if (Button("Create")){
			//create components
			AudioListener* al = 0;
			if(comp_audiolistener){ 
				al = new AudioListener(entity_pos, Vector3::ZERO, entity_rot); 
			}
			AudioSource* as = 0;
			if(comp_audiosource) {
				as = new AudioSource();
			}
			Collider* coll = 0;
			if(comp_collider){
				switch(collider_type){
					case ColliderType_Box:{
						coll = new BoxCollider(collider_halfdims, physics_mass);
					}break;
					case ColliderType_AABB:{
						coll = new AABBCollider(collider_halfdims, physics_mass);
					}break;
					case ColliderType_Sphere:{
						coll = new SphereCollider(collider_radius, physics_mass);
					}break;
				}
			}
			MeshComp* mc = 0;
			if(comp_mesh){
				u32 new_mesh_id = DengRenderer->CreateMesh(mesh_id, 
														   mat4::TransformationMatrix(entity_pos, entity_rot, entity_scale));
				mc = new MeshComp(new_mesh_id, mesh_instance_id);
			}
			Light* light = 0;
			if(comp_light){
				light = new Light(entity_pos, entity_rot, light_strength);
			}
			Physics* phys = 0;
			if(comp_physics){
				phys = new Physics(entity_pos, entity_rot, physics_velocity, physics_accel, physics_rotVel,
								   physics_rotAccel, physics_elasticity, physics_mass, physics_staticPosition);
				if(comp_audiolistener) al->velocity = physics_velocity;
			}
			
			//create entity
			admin->world->CreateEntity(admin, {al, as, coll, mc, light, phys}, entity_name, 
									   Transform(entity_pos, entity_rot, entity_scale));
		}Separator();
		
		//// presets ////
		SetPadding; Text("Presets    "); SameLine(); SetNextItemWidth(-1); 
		if(Combo("##preset_combo", &current_preset, presets, IM_ARRAYSIZE(presets))){
			switch(current_preset){
				case(0):default:{
					comp_audiolistener = comp_audiosource = comp_collider = comp_mesh = comp_light = comp_physics = false;
					collider_type = ColliderType_NONE;
					collider_halfdims = Vector3::ONE;
					collider_radius  = 1.f;
					mesh_id = mesh_instance_id = -1;
					light_strength = 1.f;
					physics_velocity = Vector3::ZERO; physics_accel    = Vector3::ZERO; 
					physics_rotVel   = Vector3::ZERO; physics_rotAccel = Vector3::ZERO;
					physics_elasticity = .5f; physics_mass = 1.f;
					physics_staticPosition = physics_staticRotation = true;
					cpystr(entity_name, TOSTRING("default", entity_id).c_str(), 63);
				}break;
				case(1):{
					comp_audiolistener = comp_audiosource = comp_light = false;
					comp_collider = comp_mesh = comp_physics = true;
					collider_type = ColliderType_AABB;
					collider_halfdims = Vector3::ONE;
					mesh_id = DengRenderer->GetBaseMeshID("default_box");
					physics_staticPosition = physics_staticRotation = true;
					cpystr(entity_name, TOSTRING("aabb", entity_id).c_str(), 63);
				}break;
				case(2):{
					comp_audiolistener = comp_audiosource = comp_light = false;
					comp_collider = comp_mesh = comp_physics = true;
					collider_halfdims = Vector3::ONE;
					mesh_id = DengRenderer->GetBaseMeshID("default_box");
					physics_staticPosition = physics_staticRotation = true;
					cpystr(entity_name, TOSTRING("box", entity_id).c_str(), 63);
				}break;
				case(3):{
					comp_audiolistener = comp_audiosource = comp_light = false;
					comp_collider = comp_mesh = comp_physics = true;
					collider_type = ColliderType_Sphere;
					collider_radius = 1.f;
					mesh_id = DengRenderer->GetBaseMeshID("sphere.obj");
					physics_staticPosition = physics_staticRotation = true;
					cpystr(entity_name, TOSTRING("sphere", entity_id).c_str(), 63);
				}break;
				case(4):{
					comp_light = false;
					comp_audiolistener = comp_audiosource = comp_collider = comp_mesh = comp_physics = true;
					collider_type = ColliderType_AABB; //TODO(delle,PhCl) ideally cylinder/capsule collider
					collider_halfdims = vec3(1, 2, 1);
					comp_mesh = true;
					mesh_id = DengRenderer->GetBaseMeshID("bmonkey.obj");
					physics_staticPosition = physics_staticRotation = false;
					cpystr(entity_name, "player", 63);
				}break;
			}
			if(mesh_id < DengRenderer->meshes.size()) mesh_name = DengRenderer->meshes[mesh_id].name;
		}
		
		SetPadding; Text("Name: "); 
		SameLine(); SetNextItemWidth(-1); InputText("##unique_id", entity_name, 64, ImGuiInputTextFlags_EnterReturnsTrue | 
													ImGuiInputTextFlags_AutoSelectAll);
		
		//// transform ////
		int tree_flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoAutoOpenOnLog;
		SetNextItemOpen(true, ImGuiCond_Once);
		if (TreeNodeEx("Transform", tree_flags)){
			Text("Position     "); SameLine(); InputVector3("entity_pos",   &entity_pos);   Separator();
			Text("Rotation     "); SameLine(); InputVector3("entity_rot",   &entity_rot);   Separator();
			Text("Scale        "); SameLine(); InputVector3("entity_scale", &entity_scale);
			TreePop();
		}
		
		//// toggle components ////
		SetNextItemOpen(true, ImGuiCond_Once);
		if (TreeNodeEx("Components", tree_flags)){
			Checkbox("Mesh", &comp_mesh);
			SameLine(); Checkbox("Physics", &comp_physics); 
			SameLine(); Checkbox("Collider", &comp_collider);
			Checkbox("Audio Listener", &comp_audiolistener);
			SameLine(); Checkbox("Light", &comp_light);
			Checkbox("Audio Source", &comp_audiosource);
			TreePop();
		}
		
		//// component headers ////
		if(comp_mesh && TreeNodeEx("Mesh", tree_flags)){
			Text(TOSTRING("MeshID: ", mesh_id).c_str());
			SetNextItemWidth(-1); if(BeginCombo("##mesh_combo", mesh_name)){
				for_n(i, DengRenderer->meshes.size()){
					if(DengRenderer->meshes[i].base && Selectable(DengRenderer->meshes[i].name, mesh_id == i)){
						mesh_id = i; 
						mesh_name = DengRenderer->meshes[i].name;
					}
				}
				EndCombo();
			}
			TreePop();
		}
		if(comp_physics && TreeNodeEx("Physics", tree_flags)){
			Text("Velocity     "); SameLine(); InputVector3("phys_vel",   &physics_velocity);    Separator();
			Text("Accelertaion "); SameLine(); InputVector3("phys_accel",   &physics_accel);     Separator();
			Text("Rot Velocity "); SameLine(); InputVector3("phys_rotvel", &physics_rotVel);     Separator();
			Text("Rot Accel    "); SameLine(); InputVector3("phys_rotaccel", &physics_rotAccel); Separator();
			Text("Elasticity   "); SameLine(); InputFloat("phys_elastic", &physics_elasticity);  Separator();
			Text("Mass         "); SameLine(); InputFloat("phys_mass", &physics_mass);           Separator();
			Checkbox("Static Position", &physics_staticPosition);                                Separator();
			Checkbox("Static Rotation", &physics_staticRotation);
			TreePop();
		}
		if(comp_collider && TreeNodeEx("Collider", tree_flags)){
			SetNextItemWidth(-1); Combo("##coll_combo", &collider_type, colliders, IM_ARRAYSIZE(colliders));
			switch(collider_type){
				case ColliderType_Box: case ColliderType_AABB:{
					Text("Half Dims    "); SameLine(); InputVector3("coll_halfdims", &collider_halfdims);
				}break;
				case ColliderType_Sphere:{
					Text("Radius       "); SameLine(); InputFloat("coll_radius", &collider_radius);
				}break;
			}
			TreePop();
		}
		if(comp_audiolistener && TreeNodeEx("Audio Listener", tree_flags)){
			//TODO(sushi,Ui) add audio listener create menu
			TreePop();
		}
		if(comp_audiosource && TreeNodeEx("Audio Source", tree_flags)){
			//TODO(sushi,Ui) add audio source create menu
			TreePop();
		}
		if(comp_light && TreeNodeEx("Light", tree_flags)){
			Text("Strength     "); SameLine(); InputFloat("phys_mass", &physics_mass); Separator();
			TreePop();
		}
		EndChild();
	}
	PopStyleVar();
}

void CanvasSystem::DebugTools() {
	ZoneScoped;

	using namespace ImGui;
	
	float fontsize = ImGui::GetFontSize();
	
	//resize tool menu if main menu bar is open
	ImGui::SetNextWindowSize(ImVec2(DengWindow->width / 5, DengWindow->height - (menubarheight + debugbarheight)));
	ImGui::SetNextWindowPos(ImVec2(0, menubarheight));
	
	//window styling
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 5);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,   ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(2, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 0);
	
	ImGui::PushStyleColor(ImGuiCol_Border,               ColToVec4(Color( 0,  0,  0)));
	ImGui::PushStyleColor(ImGuiCol_Button,               ColToVec4(Color(40, 40, 40)));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive,         ColToVec4(Color(48, 48, 48)));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered,        ColToVec4(Color(60, 60, 60)));
	ImGui::PushStyleColor(ImGuiCol_WindowBg,             ColToVec4(colors.c9));
	ImGui::PushStyleColor(ImGuiCol_PopupBg,              ColToVec4(Color(20, 20, 20)));
	ImGui::PushStyleColor(ImGuiCol_FrameBg,              ColToVec4(Color(35, 45, 50)));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive,        ColToVec4(Color(42, 54, 60)));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,       ColToVec4(Color(54, 68, 75)));
	ImGui::PushStyleColor(ImGuiCol_Header,               ColToVec4(Color(35, 45, 50)));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive,         ColToVec4(Color( 0, 74, 74)));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered,        ColToVec4(Color( 0, 93, 93)));
	ImGui::PushStyleColor(ImGuiCol_TableBorderLight,     ColToVec4(Color(45, 45, 45)));
	ImGui::PushStyleColor(ImGuiCol_TableHeaderBg,        ColToVec4(Color(10, 10, 10)));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarBg,          ColToVec4(Color(10, 10, 10)));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab,        ColToVec4(Color(55, 55, 55)));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive,  ColToVec4(Color(75, 75, 75)));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ColToVec4(Color(65, 65, 65)));
	ImGui::PushStyleColor(ImGuiCol_TabActive,            ColToVec4(Color::VERY_DARK_CYAN));
	ImGui::PushStyleColor(ImGuiCol_TabHovered,           ColToVec4(Color::DARK_CYAN));
	ImGui::PushStyleColor(ImGuiCol_Tab,                  ColToVec4(colors.c1));
	
	ImGui::Begin("DebugTools", (bool*)1, ImGuiWindowFlags_NoFocusOnAppearing |  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	
	//capture mouse if hovering over this window
	WinHovCheck; 
	
	SetPadding;
	if (BeginTabBar("MajorTabs")) {
		if (BeginTabItem("Entities")) {
			EntitiesTab(admin, fontsize);
			EndTabItem();
		}
		if (BeginTabItem("Create")) {
			CreateTab(admin, fontsize);
			EndTabItem();
		}
		
		EndTabBar();
	}
	
	ImGui::PopStyleVar(7);
	ImGui::PopStyleColor(21);
	ImGui::End();
}

void CanvasSystem::DebugBar() {
	ZoneScoped;

	using namespace ImGui;
	
	//for getting fps
	ImGuiIO& io = ImGui::GetIO();
	
	int FPS = floor(io.Framerate);
	
	//num of active columns
	int activecols = 7;
	
	//font size for centering text
	float fontsize = ImGui::GetFontSize();
	
	//flags for showing different things
	static bool show_fps = true;
	static bool show_fps_graph = true;
	static bool show_world_stats = true;
	static bool show_selected_stats = true;
	static bool show_floating_fps_graph = false;
	static bool show_time = true;
	
	ImGui::SetNextWindowSize(ImVec2(DengWindow->width, 20));
	ImGui::SetNextWindowPos(ImVec2(0, DengWindow->height - 20));
	
	
	//window styling
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,   ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(2, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Border,           ColToVec4(Color(0, 0, 0, 255)));
	ImGui::PushStyleColor(ImGuiCol_WindowBg,         ColToVec4(Color(20, 20, 20, 255)));
	ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ColToVec4(Color(45, 45, 45, 255)));
	
	ImGui::Begin("DebugBar", (bool*)1, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	debugbarheight = 20;
	//capture mouse if hovering over this window
	WinHovCheck; 
	
	activecols = show_fps + show_fps_graph + 3 * show_world_stats + 2 * show_selected_stats + show_time + 1;
	if (BeginTable("DebugBarTable", activecols, ImGuiTableFlags_BordersV | ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_SizingFixedFit)) {
		
		//precalc strings and stuff so we can set column widths appropriately
		std::string str1 = TOSTRING("wents: ", admin->entities.size());
		float strlen1 = (fontsize - (fontsize / 2)) * str1.size();
		std::string str2 = TOSTRING("wtris: ", g_renderer->stats.totalTriangles);
		float strlen2 = (fontsize - (fontsize / 2)) * str2.size();
		std::string str3 = TOSTRING("wverts: ", g_renderer->stats.totalVertices);
		float strlen3 = (fontsize - (fontsize / 2)) * str3.size();
		std::string str4 = TOSTRING("stris: ", "0");
		float strlen4 = (fontsize - (fontsize / 2)) * str4.size();
		std::string str5 = TOSTRING("sverts: ", "0");
		float strlen5 = (fontsize - (fontsize / 2)) * str5.size();
		
		ImGui::TableSetupColumn("FPS",            ImGuiTableColumnFlags_WidthFixed, 64);
		ImGui::TableSetupColumn("FPSGraphInline", ImGuiTableColumnFlags_WidthFixed, 64);
		ImGui::TableSetupColumn("EntCount",       ImGuiTableColumnFlags_None, strlen1 * 1.3);
		ImGui::TableSetupColumn("TriCount",       ImGuiTableColumnFlags_None, strlen2 * 1.3);
		ImGui::TableSetupColumn("VerCount",       ImGuiTableColumnFlags_None, strlen3 * 1.3);
		ImGui::TableSetupColumn("SelTriCount",    ImGuiTableColumnFlags_None, strlen4 * 1.3);
		ImGui::TableSetupColumn("SelVerCount",    ImGuiTableColumnFlags_None, strlen5 * 1.3);
		ImGui::TableSetupColumn("MiddleSep",      ImGuiTableColumnFlags_WidthStretch, 0);
		ImGui::TableSetupColumn("Time",           ImGuiTableColumnFlags_WidthFixed, 64);
		
		
		//FPS
		
		if (TableNextColumn() && show_fps) {
			//trying to keep it from changing width of column
			//actually not necessary anymore but im going to keep it cause 
			//it keeps the numbers right aligned
			if (FPS % 1000 == FPS) {
				Text(TOSTRING("FPS:  ", FPS).c_str());
			}
			else if (FPS % 100 == FPS) {
				Text(TOSTRING("FPS:   ", FPS).c_str());
			}
			else {
				Text(TOSTRING("FPS: ", FPS).c_str());
			}
			
		}
		
		//FPS graph inline
		if (TableNextColumn() && show_fps_graph) {
			//how much data we store
			static int prevstoresize = 100;
			static int storesize = 100;
			
			//how often we update
			static int fupdate = 20;
			static int frame_count = 0;
			
			//maximum FPS
			static int maxval = 0;
			
			//real values and printed values
			static std::vector<float> values(storesize);
			static std::vector<float> pvalues(storesize);
			
			//dynamic resizing that may get removed later if it sucks
			//if FPS finds itself as less than half of what the max used to be we lower the max
			if (FPS > maxval || FPS < maxval / 2) {
				maxval = FPS;
			}
			
			//if changing the amount of data we're storing we have to reverse
			//each data set twice to ensure the data stays in the right place when we move it
			if (prevstoresize != storesize) {
				std::reverse(values.begin(), values.end());    values.resize(storesize);  std::reverse(values.begin(), values.end());
				std::reverse(pvalues.begin(), pvalues.end());  pvalues.resize(storesize); std::reverse(pvalues.begin(), pvalues.end());
				prevstoresize = storesize;
			}
			
			std::rotate(values.begin(), values.begin() + 1, values.end());
			
			//update real set if we're not updating yet or update the graph if we are
			if (frame_count < fupdate) {
				values[values.size() - 1] = FPS;
				frame_count++;
			}
			else {
				float avg = Math::average(values.begin(), values.end(), storesize);
				std::rotate(pvalues.begin(), pvalues.begin() + 1, pvalues.end());
				pvalues[pvalues.size() - 1] = std::floorf(avg);
				
				frame_count = 0;
			}
			
			ImGui::PushStyleColor(ImGuiCol_PlotLines, ColToVec4(Color(0, 255, 200, 255)));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ColToVec4(Color(20, 20, 20, 255)));
			
			ImGui::PlotLines("", &pvalues[0], pvalues.size(), 0, 0, 0, maxval, ImVec2(64, 20));
			
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
		
		
		//World stats
		
		//Entity Count
		if (TableNextColumn() && show_world_stats) {
			ImGui::SameLine((GetColumnWidth() - strlen1) / 2);
			Text(str1.c_str());
		}
		
		//Triangle Count
		if (TableNextColumn() && show_world_stats) {
			//TODO( sushi,Ui) implement triangle count when its avaliable
			ImGui::SameLine((GetColumnWidth() - strlen2) / 2);
			Text(str2.c_str());
		}
		
		//Vertice Count
		if (TableNextColumn() && show_world_stats) {
			//TODO( sushi,Ui) implement vertice count when its avaliable
			ImGui::SameLine((GetColumnWidth() - strlen3) / 2);
			Text(str3.c_str());
		}
		
		
		
		// Selected Stats
		
		
		
		//Triangle Count
		if (TableNextColumn() && show_selected_stats) {
			//TODO( sushi,Ui) implement triangle count when its avaliable
			Entity* e = admin->selectedEntity;
			ImGui::SameLine((GetColumnWidth() - strlen4) / 2);
			Text(str4.c_str());
		}
		
		//Vertice Count
		if (TableNextColumn() && show_selected_stats) {
			//TODO( sushi,Ui) implement vertice count when its avaliable
			Entity* e = admin->selectedEntity;
			ImGui::SameLine((GetColumnWidth() - strlen5) / 2);
			Text(str5.c_str());
		}
		
		//Middle Empty Separator
		if (TableNextColumn()) {
			static float time = DengTime->totalTime;
			if (DengConsole->cons_error_warn) {
				time = DengTime->totalTime;
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, GetColorU32(ColToVec4(Color(255 * (sin(2 * M_PI * time + cos(2 * M_PI * time)) + 1)/2, 0, 0, 255))));
				
				PushItemWidth(-1);
				std::string str6 = DengConsole->last_error;
				float strlen6 = (fontsize - (fontsize / 2)) * str6.size();
				ImGui::SameLine((GetColumnWidth() - strlen6) / 2);
				ImGui::PushStyleColor(ImGuiCol_Text, ColToVec4(Color(255 * -(sin(2 * M_PI * time + cos(2 * M_PI * time)) - 1)/2, 0, 0, 255)));
				Text(str6.c_str());
				PopStyleColor();
			}
			
			
		}
		
		//Show Time
		if (TableNextColumn()) {
			//https://stackoverflow.com/questions/24686846/get-current-time-in-milliseconds-or-hhmmssmmm-format
			using namespace std::chrono;
			
			//get current time
			auto now = system_clock::now();
			
			//convert to std::time_t so we can convert to std::tm
			auto timer = system_clock::to_time_t(now);
			
			//convert to broken time
			std::tm bt = *std::localtime(&timer);
			
			std::ostringstream oss;
			
			oss << std::put_time(&bt, "%H:%M:%S");
			
			std::string str7 = oss.str();
			float strlen7 = (fontsize - (fontsize / 2)) * str7.size();
			ImGui::SameLine(32 - (strlen7 / 2));
			
			Text(str7.c_str());
			
		}
		
		
		//Context menu for toggling parts of the bar
		if (ImGui::IsMouseReleased(1) && IsWindowHovered()) OpenPopup("Context");
		if (BeginPopup("Context")) {
			DengConsole->IMGUI_MOUSE_CAPTURE = true;
			ImGui::Separator();
			if (Button("Open Debug Menu")) {
				//showDebugTools = true;
				CloseCurrentPopup();
			}
			
			EndPopup();
		}
		ImGui::EndTable();
	}
	
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::End();
}

//sort of sandbox for drawing ImGui stuff over the entire screen
void CanvasSystem::DebugLayer() {
	ZoneScoped;


	ImGui::SetNextWindowSize(ImVec2(DengWindow->width, DengWindow->height));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ColToVec4(Color(0, 0, 0, 0)));
	Camera* c = admin->mainCamera;
	float time = DengTime->totalTime;
	
	static std::vector<std::pair<float, Vector2>> times;
	
	static std::vector<Vector3> spots;
	
	
	ImGui::Begin("DebugLayer", 0, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
	
	Vector2 mp = DengInput->mousePos;
	
	float fontsize = ImGui::GetFontSize();
	
	
	
	
	//psuedo grid
	int lines = 100;
	for (int i = 0; i < lines * 2; i++) {
		Vector3 cpos = c->position;
		Vector3 v1 = Math::WorldToCamera4(Vector3(floor(cpos.x) + -lines + i, 0, floor(cpos.z) + -lines), c->viewMatrix).ToVector3();
		Vector3 v2 = Math::WorldToCamera4(Vector3(floor(cpos.x) + -lines + i, 0, floor(cpos.z) + lines), c->viewMatrix).ToVector3();
		Vector3 v3 = Math::WorldToCamera4(Vector3(floor(cpos.x) + -lines, 0, floor(cpos.z) + -lines + i), c->viewMatrix).ToVector3();
		Vector3 v4 = Math::WorldToCamera4(Vector3(floor(cpos.x) + lines, 0, floor(cpos.z) + -lines + i), c->viewMatrix).ToVector3();
		
		
		//TODO(sushi, CamMa) make grid lines appear properly when in different orthographic views
		//if (c->type == CameraType::ORTHOGRAPHIC) {
		//
		//	if (c->orthoview == FRONT) {
		//		v1 = Math::WorldToCamera4(Vector3(floor(cpos.x) + -lines + i, 0, floor(cpos.y) + -lines), c->viewMatrix).ToVector3();
		//		v2 = Math::WorldToCamera4(Vector3(floor(cpos.x) + -lines + i, 0, floor(cpos.y) + lines), c->viewMatrix).ToVector3();
		//		v3 = Math::WorldToCamera4(Vector3(floor(cpos.x) + -lines, 0, floor(cpos.y) + -lines + i), c->viewMatrix).ToVector3();
		//		v4 = Math::WorldToCamera4(Vector3(floor(cpos.x) + lines, 0, floor(cpos.y) + -lines + i), c->viewMatrix).ToVector3();
		//
		//	}
		//	
		//
		//
		//}
		
		bool l1flag = false;
		bool l2flag = false;
		
		if (floor(cpos.x) - lines + i == 0) {
			l1flag = true;
		}
		if (floor(cpos.z) - lines + i == 0) {
			l2flag = true;
		}
		//Vector3 v1t = v1.ToVector3();
		//Vector3 v2t = v2.ToVector3();
		//Vector3 v3t = v3.ToVector3();
		//Vector3 v4t = v4.ToVector3();
		
		
		if (Math::ClipLineToZPlanes(v1, v2, c)) {
			Vector3 v1s = Math::CameraToScreen3(v1, c->projectionMatrix, DengWindow->dimensions);
			Vector3 v2s = Math::CameraToScreen3(v2, c->projectionMatrix, DengWindow->dimensions);
			Math::ClipLineToBorderPlanes(v1s, v2s, DengWindow->dimensions);
			if (!l1flag) ImGui::GetBackgroundDrawList()->AddLine(v1s.ToVector2().ToImVec2(), v2s.ToVector2().ToImVec2(), ImGui::GetColorU32(ImVec4(1, 1, 1, 0.3)));
			else         ImGui::GetBackgroundDrawList()->AddLine(v1s.ToVector2().ToImVec2(), v2s.ToVector2().ToImVec2(), ImGui::GetColorU32(ImVec4(1, 0, 0, 1)));
		}
		if (Math::ClipLineToZPlanes(v3, v4, c)) {
			Vector3 v3s = Math::CameraToScreen3(v3, c->projectionMatrix, DengWindow->dimensions);
			Vector3 v4s = Math::CameraToScreen3(v4, c->projectionMatrix, DengWindow->dimensions);
			Math::ClipLineToBorderPlanes(v3s, v4s, DengWindow->dimensions);
			if (!l2flag) ImGui::GetBackgroundDrawList()->AddLine(v3s.ToVector2().ToImVec2(), v4s.ToVector2().ToImVec2(), ImGui::GetColorU32(ImVec4(1, 1, 1, 0.3)));
			else         ImGui::GetBackgroundDrawList()->AddLine(v3s.ToVector2().ToImVec2(), v4s.ToVector2().ToImVec2(), ImGui::GetColorU32(ImVec4(0, 0, 1, 1)));
		}
	}
	
	if (DengInput->KeyPressed(MouseButton::LEFT) && rand() % 100 + 1 == 80) {
		times.push_back(std::pair<float, Vector2>(0.f, mp));
	}
	
	int index = 0;
	for (auto& f : times) {
		ImGui::PushStyleColor(ImGuiCol_Text, ColToVec4(Color(255. * fabs(sinf(time)), 255. * fabs(cosf(time)), 255, 255)));
		
		f.first += DengTime->deltaTime;
		
		Vector2 p = f.second;
		
		ImGui::SetCursorPos(ImVec2(p.x + 20 * sin(2 * time), p.y - 200 * (f.first / 5)));
		
		Vector2 curpos = Vector2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY());
		
		std::string str1 = "hehe!!!!";
		float strlen1 = (fontsize - (fontsize / 2)) * str1.size();
		for (int i = 0; i < str1.size(); i++) {
			ImGui::SetCursorPos(ImVec2(
									   curpos.x + i * fontsize / 2,
									   curpos.y + sin(10 * time + cos(10 * time + (i * M_PI / 2)) + (i * M_PI / 2))
									   ));
			ImGui::Text(str1.substr(i, 1).c_str());
		}
		
		if (f.first >= 5) {
			times.erase(times.begin() + index);
			index--;
		}
		
		ImGui::PopStyleColor();
		index++;
	}
	ImGui::Text("test");
	
	
	if (admin->paused) {
		std::string s = "ENGINE PAUSED";
		float strlen = (fontsize - (fontsize / 2)) * s.size();
		//ImGui::SameLine(32 - (strlen / 2));
		ImGui::SetCursorPos(ImVec2(DengWindow->width - strlen * 1.3, menubarheight));
		ImGui::Text(s.c_str());
	}
	
	if (admin->debugTimes) {
		std::string time1 = DengTime->FormatTickTime ("Time       : {t}\n"
													  "Window     : {w}\n"
													  "Input      : {i}\n");
		time1            += DengTime->FormatAdminTime("Physics Lyr: {P}\n"
													  "        Sys: {p}\n"
													  "Canvas  Lyr: {C}\n"
													  "        Sys: {c}\n"
													  "World   Lyr: {W}\n"
													  "        Sys: {w}\n"
													  "Sound   Lyr: {S}\n"
													  "        Sys: {s}\n"
													  "Last    Lyr: {L}\n");
		time1            += DengTime->FormatTickTime ("Admin      : {a}\n"
													  "Console    : {c}\n"
													  "Render     : {r}\n"
													  "Frame      : {f}");
		
		float fontw = (fontsize - (fontsize / 2));
		ImGui::SetCursorPos(ImVec2(DengWindow->width - fontw * 18 * 1.3 - 20, menubarheight));
		ImGui::Text(time1.c_str());
	}
	
	
	
	ImGui::PopStyleColor();
	ImGui::End();
}

void CanvasSystem::DrawUI(void) {
	ZoneScoped;

	if(admin->state == GameState::PLAY){
		
	}
	else if(admin->state == GameState::MENU){
		
	}
	else if(admin->state == GameState::EDITOR || admin->state == GameState::PLAY_DEBUG){
		if (DengInput->KeyPressed(DengKeys.toggleDebugMenu)) showDebugTools = !showDebugTools;
		if (DengInput->KeyPressed(DengKeys.toggleDebugBar)) showDebugBar = !showDebugBar;
		if (DengInput->KeyPressed(DengKeys.toggleMenuBar)) showMenuBar = !showMenuBar;
		
		if (showDebugLayer) DebugLayer();
		if (showDebugTools) DebugTools();
		if (showDebugBar)   DebugBar();
		if (showMenuBar)    MenuBar();
		if (showImGuiDemoWindow) ImGui::ShowDemoWindow();
		
		
		if (!showMenuBar) {
			menubarheight = 0;
		}
		
		if (!showDebugBar){
			debugbarheight = 0;
		}
		if (!showDebugTools) {
			debugtoolswidth = 0;
		}
	}
	else{
		ASSERT(false, "Unknown game state in CanvasSystem");
	}
}

void CanvasSystem::Init(EntityAdmin* admin) {
	System::Init(admin);
	files = deshi::iterateDirectory(deshi::dirModels());
	textures = deshi::iterateDirectory(deshi::dirTextures());
}

void CanvasSystem::Update() {
	WinHovFlag = 0;
	if(!DengWindow->minimized) DrawUI();
	//DrawUI();  //HACK program crashes somewhere in DebugTools() if minimized
	if (ConsoleHovFlag || WinHovFlag) DengConsole->IMGUI_MOUSE_CAPTURE = true;
	else                              DengConsole->IMGUI_MOUSE_CAPTURE = false;
}

