#include "console.h"
#include "../core.h"
#include "../utils/Command.h"
#include "../game/Keybinds.h"
#include "../game/Transform.h"
#include "../game/UndoManager.h"
#include "../game/systems/WorldSystem.h"
#include "../game/systems/CanvasSystem.h"
#include "../game/components/Camera.h"
#include "../game/components/Physics.h"
#include "../game/components/Collider.h"
#include "../game/components/AudioSource.h"
#include "../game/components/MeshComp.h"
#include "../game/components/Player.h"
#include "../game/components/Movement.h"
#include "../scene/Scene.h"
#include "../EntityAdmin.h"

#include "../external/imgui/imgui_impl_glfw.h"
#include "../external/imgui/imgui_impl_vulkan.h"

#include <functional>
#include <ctime>
#include <time.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

//regex for checking paramaters
std::regex RegColorFormat("(?:\\[c:([^\\]]*)\\]([^\\]]*)\\[c\\]|([^\\[]+))", std::regex::optimize);
std::regex RegPosParam("-pos=\\([0-9|.|-]+,[0-9|.|-]+,[0-9|.|-]+\\)", std::regex::optimize);
std::regex RegRotParam("-rot=\\([0-9|.|-]+,[0-9|.|-]+,[0-9|.|-]+\\)", std::regex::optimize);
std::regex RegScaleParam("-scale=\\([0-9|.|-]+,[0-9|.|-]+,[0-9|.|-]+\\)", std::regex::optimize);
std::regex RegSizeParam("-size=\\([0-9|.|-]+,[0-9|.|-]+,[0-9|.|-]+\\)", std::regex::optimize);
std::regex VecNumMatch("[,\\(]?([0-9|.|-]+)[,\\)]?[,\\(]?([0-9|.|-]+)[,\\)]?[,\\(]?([0-9|.|-]+)[,\\)]?", std::regex::optimize);
std::regex StringRegex(const char* param){ return std::regex(std::string("-")+ param +"=([A-z]+)", std::regex::optimize|std::regex::icase); }
std::regex IntRegex(const char* param)   { return std::regex(std::string("-")+ param +"=([-]?[0-9]+)", std::regex::optimize); }
std::regex FloatRegex(const char* param) { return std::regex(std::string("-")+ param +"=([-]?[0-9|.]+)", std::regex::optimize); }
std::regex BoolRegex(const char* param)  { return std::regex(std::string("-")+ param +"=(true|1|false|0)", std::regex::optimize|std::regex::icase); }

using namespace ImGui;

int buffersize = 0;

bool sel_com = false; //true when selecting an auto complete possibility
bool sel_com_ret = false; //tells the callback function that it is going to replace text
std::string sel_com_str = ""; //the string we're replacing input with
std::vector<std::string> posis;
//int match_sel = 0;

std::map<std::string, Color> colstrmap{
	{"red", Color::RED},
	{"dred", Color::DARK_RED},
	{"blue", Color::BLUE},
	{"dblue", Color::DARK_BLUE},
	{"cyan", Color::CYAN},
	{"dcyan", Color::DARK_CYAN},
	{"grey", Color::GREY},
	{"dgrey", Color::DARK_GREY},
	{"green", Color::GREEN},
	{"dgreen", Color::DARK_GREEN},
	{"yellow", Color::YELLOW},
	{"dyellow", Color::DARK_YELLOW},
	{"magen", Color::MAGENTA},
	{"dmagen", Color::DARK_MAGENTA},
	{"black", Color::BLACK},
	{"error", Color::RED} //special error color for the console to know when to flash the debug bar
};

//map for making new components in commands
std::map<std::string, std::function<Component*()>> compstrmap;

ImVec4 ColorToVec4(Color p) {
	return ImVec4((float)p.r / 255, (float)p.g / 255, (float)p.b / 255, p.a / 255);
}

void Console::AddLog(std::string input) {
	
	if (this) {
		std::smatch m;
		
		while (std::regex_search(input, m, RegColorFormat)) { //parse text for color formatting
			
			//check if were dealing with a formatted part of the string
			if (std::regex_search(m[0].str(), std::regex("\\[c:[^\\]]+\\]"))) {
				//if we are, push the actual text with its color into text vector
				buffer.push_back(std::pair<std::string, Color>(m[2].str(), colstrmap.at(m[1])));
				buffersize += m[2].str().size();
				if (m[1] == "error") {
					cons_error_warn = true;
					last_error = m[2].str();
				}
			}
			else {
				//if we arent then just push the line into text vector
				buffer.push_back(std::pair<std::string, Color>(m[0].str(), Color::BLANK));
				buffersize += m[2].str().size();
			}
			input = m.suffix();
		}
		buffer[buffer.size() - 1].first += "\n";
	}
}


Command* Console::GetCommand(std::string command) {
	try {
		return commands.at(command);
	} catch (std::exception e) {
		PushConsole(TOSTRING("[c:error]Command '", command, "' does not exist[c]"));
		return 0;
	}
}

std::string Console::ExecCommand(std::string command) {
	if (commands.find(command) != commands.end()) {
		return commands.at(command)->Exec(g_admin, "");
	}
	else {
		return "[c:red]Command[c] \"" + command + "\" [c:red]not found.[c]";
	}
}

std::string Console::ExecCommand(std::string command, std::string args) {
	if (commands.find(command) != commands.end()) {
		return commands.at(command)->Exec(g_admin, args);
	}
	else {
		return "[c:red]Command[c] \"" + command + "\" [c:red]not found.[c]";
	}
}

int Console::TextEditCallback(ImGuiInputTextCallbackData* data) {
	switch (data->EventFlag) {
		case ImGuiInputTextFlags_CallbackCompletion: {
			std::string input = data->Buf;
			
			int fwordl = 0;
			
			if (std::regex_search(input, std::regex("^.+ +"))) {
				fwordl = input.find_first_of(" ") + 1;
				input.erase(0, input.find_first_of(" ") + 1);
				
			}
			
			std::regex e("^" + input + ".*");
			std::vector<std::string> posi;
			for (std::pair<std::string, Command*> c : commands) {
				if (std::regex_search(c.first, e)) {
					posi.push_back(c.first);
				}
			}
			//TODO( sushi,Cmd) implement showing a commands help if tab is pressed when the command is already typed
			
			if (posi.size() == 0) {
				AddLog("no matches found");
			}
			else if (posi.size() == 1) {
				data->DeleteChars(fwordl, data->BufTextLen - fwordl);
				data->InsertChars(data->CursorPos, posi[0].c_str());
			}
			else { //if there are multiple we handle them in a selection table
				posis.clear();
				posis = posi;
				sel_com = true;
				
				
			}
			
			scrollToBottom = true; //scroll to bottom when auto completing 
			
			break;
		}
		case ImGuiInputTextFlags_CallbackHistory: {
			
			const int prev_hist_pos = historyPos;
			if (data->EventKey == ImGuiKey_UpArrow) {
				if (historyPos == -1) {
					historyPos = history.size() - 1;
				}
				else if (historyPos > 0) {
					historyPos--;
				}
			}
			else if (data->EventKey == ImGuiKey_DownArrow) {
				if (historyPos != -1) {
					if (++historyPos >= history.size()) {
						historyPos = -1;
					}
				}
			}
			
			if (prev_hist_pos != historyPos)
			{
				std::string history_str = (historyPos >= 0) ? history[historyPos] : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, history_str.c_str());
			}
			
			break;
		}
		case ImGuiInputTextFlags_CallbackAlways: {
			if (sel_com_ret) {
				std::string str = data->Buf;
				
				int fwordl = 0; //we need to make sure we don't override valid input
				if (std::regex_search(str, std::regex("^.+ *"))) {
					fwordl = str.find_first_of(" ") + 1;
					if (fwordl == 0) {
						str.erase(0, str.length());
					}
					else {
						str.erase(0, str.find_first_of(" ") + 1);
					}
					
				}
				
				str += sel_com_str;
				
				data->DeleteChars(0, data->BufTextLen - fwordl);
				data->InsertChars(data->BufTextLen, str.c_str());
				sel_com_ret = false;
			}
		}
	}
	
	return 0;
}



int Console::TextEditCallbackStub(ImGuiInputTextCallbackData* data) {
	return DengConsole->TextEditCallback(data);
}


void Console::DrawConsole() {
	ZoneScoped;

	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO();
	
	style.AntiAliasedFill = false;
	style.AntiAliasedLines = false;
	style.AntiAliasedLinesUseTex = false;
	
	//for some reason these werent set in the actual backend and it was causing issues
	//this could no longer be true since we switch to Vulkan but ima keep it just incase :)
	io.BackendFlags = ImGuiBackendFlags_HasGamepad | ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigWindowsResizeFromEdges = true;
	
	//window styling
	PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0);
	PushStyleColor(ImGuiCol_Border, ColorToVec4(Color(0, 0, 0, 255)));
	PushStyleColor(ImGuiCol_TitleBg, ColorToVec4(Color(0, 0, 0, 255)));
	PushStyleColor(ImGuiCol_WindowBg, ColorToVec4(Color(0, 0, 0, 255)));
	PushStyleColor(ImGuiCol_TitleBgActive, ColorToVec4(Color(0, 0, 0, 255)));
	PushStyleColor(ImGuiCol_ScrollbarGrab, ColorToVec4(Color(37, 36, 36, 255)));
	PushStyleColor(ImGuiCol_ScrollbarGrabActive, ColorToVec4(Color(0, 94, 83, 255)));
	PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ColorToVec4(Color(48, 85, 90, 255)));
	
	//initialize console window
	SetNextWindowSize(ImVec2(DengWindow->width, DengWindow->height / 1.5));
	SetNextWindowPos(ImVec2(0, 0));
	
	ImGui::Begin("Console!", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
	
	//capture mouse if hovering over this window
	//TODO(sushi, InCon) this is working for some reason pls fix it 
	//if (IsWindowHovered()) admin->canvas->ConsoleHovFlag = true; 
	//else admin->canvas->ConsoleHovFlag = false; 
	
	bool reclaim_focus = false;
	
	//display completion table
	//this could probably be done in a better way but idc it works
	static int match_sel = 0;
	bool ok_flag = false;
	if (sel_com) {
		bool selected = false;
		bool escape = false;
		if (DengInput->KeyPressed(Key::DOWN) && match_sel < posis.size() - 1) { match_sel++; }
		if (DengInput->KeyPressed(Key::UP) && match_sel > 0) { match_sel--; }
		if (DengInput->KeyPressed(Key::ENTER)) { selected = true; reclaim_focus = true; }
		if (DengInput->KeyPressed(Key::ESCAPE)) { escape = true; match_sel = 0; reclaim_focus = true; }
		
		if (escape) { ok_flag = true; }
		else {
			ImGui::SetNextItemOpen(1);
			if (TreeNode("match table")) {
				if (BeginChild("matchScroll", ImVec2(0, 100), false)) {
					if (BeginTable("match table", 1, ImGuiTableFlags_BordersH)) {//posi.size())) {
						
						int i = 0;
						for (std::string s : posis) {
							TableNextColumn();
							if (i == match_sel) {
								SetScrollHereY(0);
								PushStyleColor(ImGuiCol_Text, ColorToVec4(Color::RED));
								Text(s.c_str());
								ImGui::PopStyleColor();
								if (selected) {
									sel_com_ret = true;
									sel_com_str = s;
									ok_flag = true;
									match_sel = 0;
									break;
								}
							}
							else {
								Text(s.c_str());
							}
							i++;
						}
						EndTable();
					}
					EndChild();
				}
				TreePop();
			}
		}
	}
	
	// Reserve enough left-over height for 1 separator + 1 input text
	//TODO(sushi, Con) figure out why the scroll bar doesnt allow you to drag it
	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	PushStyleColor(ImGuiCol_ChildBg, ColorToVec4(Color(4, 17, 21, 255)));
	BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (BeginPopupContextWindow()) {
		if (ImGui::Selectable("hehe")) AddLog("hoho");
		EndPopup();
	}
	
	
	//print previous text
	ImGuiListClipper clipper;
	clipper.Begin(buffer.size());
	while (clipper.Step()) {
		for (std::pair<std::string, Color> p : buffer) {
			//for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++){
			//color formatting is "[c:red]text[c] text text"
			//TODO( sushi,OpCon) maybe optimize by only drawing what we know will be displayed on screen instead of parsing through all of it
			
			if (p.second == Color::BLANK) {
				SameLine(0, 0);
				TextWrapped(p.first.c_str());
			}
			else {
				PushStyleColor(ImGuiCol_Text, ColorToVec4(p.second));
				SameLine(0, 0);
				TextWrapped(p.first.c_str());
				ImGui::PopStyleColor();
			}
			
			if (p.first[p.first.size() - 1] == '\n') {
				TextWrapped("\n");
			}
		}
	}
	
	
	//auto scroll window
	if (scrollToBottom || (autoScroll && GetScrollY() >= GetScrollMaxY())) SetScrollHereY(1);
	scrollToBottom = false;
	
	EndChild();
	ImGui::PopStyleColor();
	//get input from text box
	ImGuiInputTextFlags input_text_flags = 0;
	if (!sel_com)  input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackAlways;
	else  input_text_flags = 0;
	
	
	PushStyleColor(ImGuiCol_FrameBg, ColorToVec4(Color::VERY_DARK_CYAN));
	SetNextItemWidth(ImGui::GetWindowWidth() - 15);
	ImGui::SetItemDefaultFocus();
	if (InputText("", inputBuf, sizeof(inputBuf), input_text_flags, &TextEditCallbackStub, (void*)this)) {
		
		std::string s = inputBuf;
		reclaim_focus = true;
		
		if (s.size() != 0) history.push_back(s);
		
		AddLog(TOSTRING("[c:cyan]/[c][c:dcyan]\\[c] ", s)); //print command typed
		
		//cut off arguments into their own string
		std::string args;
		size_t t = s.find_first_of(" ");
		if (t != std::string::npos) {
			args = s.substr(t);
			s.erase(t, s.size() - 1);
		}
		
		if (s.size() != 0 ) {
			AddLog(ExecCommand(s, args)); //attempt to execute command and print result
		}
		
		historyPos = -1; //reset history position
		
		memset(inputBuf, 0, sizeof(s)); //erase input from text box
		
		scrollToBottom = true; //scroll to bottom when we press enter
	}
	
	
	ImGui::SetKeyboardFocusHere(-1);
	
	reclaim_focus = false;
	
	IMGUI_KEY_CAPTURE = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();	    ImGui::PopStyleColor();
	ImGui::PopStyleColor();	            ImGui::PopStyleColor();    ImGui::PopStyleVar();
	ImGui::PopStyleColor();     ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	
	//if we selected something from completion menu
	//we have to do this here to prevent enter from sending a command
	if (ok_flag) { sel_com = false; }
	
	
	ImGui::End();
}

//this must be a separate funciton because TextEditCallback had a fit when I tried
//making this the main AddLog function
void Console::PushConsole(std::string s) {
	AddLog(s);
}

//flushes the buffer to a file once it reaches a certain size
void Console::FlushBuffer() {
	std::string output = "";
	for (std::pair<std::string, Color> a : buffer) {
		output += a.first;
	}
	
	if (!std::filesystem::is_directory("logs")) {
		std::filesystem::create_directory("logs");
	}
	
	
	static std::string filename = DengTime->FormatDateTime("logs/deshiLog_{M}-{d}-{y}_{h}.{m}.{s}.txt");
	static bool session = false;
	
	std::ofstream file;
	
	//if start of session make new file
	if (!session) {
		
		file.open(filename);
		file << DengTime->FormatDateTime("Deshi Console Log {w} {M}/{d}/{y} {h}:{m}:{s}") << std::endl;
		file << "\n" << output;
		session = true;
		
	}
	else {
		file.open(filename, std::fstream::app);
		file << output;
	}
	
}



///////////////////////////////////////////////////////////////////////
// command creation functions
//////////////////////////////////////////////////////////////////////

#define NEWCOMMAND(name, desc, func) commands[name] =\
new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {\
try{ func }catch(...){ return desc; }\
}, name, desc);

#define COMMANDFUNC(name) std::string command_##name##_back(EntityAdmin* admin, std::vector<std::string> args)
#define ADDCOMMAND(name, desc) commands[#name] = new Command(command_##name##_back, #name, desc)


////////////////////////////////////////
//// various uncategorized commands ////
////////////////////////////////////////

COMMANDFUNC(daytime){
	return DengTime->FormatDateTime("{w} {M}/{d}/{y} {h}:{m}:{s}");
}

COMMANDFUNC(time_engine){
	return DengTime->FormatTickTime("Time:   {t}ms Window:{w}ms Input:{i}ms Admin:{a}ms\n"
									"Console:{c}ms Render:{r}ms Frame:{f}ms Delta:{d}ms");
}

COMMANDFUNC(time_game){
	return DengTime->FormatAdminTime("Layers:  Physics:{P}ms Canvas:{C}ms World:{W}ms Send:{S}ms Last:{L}ms\n"
									 "Systems: Physics:{p}ms Canvas:{c}ms World:{w}ms Send:{s}ms");
}

COMMANDFUNC(undo){
	admin->undoManager.Undo(); return "";
}

COMMANDFUNC(redo){
	admin->undoManager.Redo(); return "";
}

COMMANDFUNC(add_player) {
	if (args.size() > 0) {
		std::cmatch m;
		Vector3 position{}, rotation{}, scale = { 1.f, 1.f, 1.f };
		float mass = 1.f;
		float elasticity = 0;
		bool staticc = true;
		ColliderType ctype;

		//check for optional params after the first arg
		for (auto s = args.begin() + 1; s != args.end(); ++s) {
			if (std::regex_match(*s, RegPosParam)) { // -pos=(1,2,3)
				std::regex_search(s->c_str(), m, VecNumMatch);
				position = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
			}
			else if (std::regex_match(*s, RegRotParam)) { //-rot=(1.1,2,3)
				std::regex_search(s->c_str(), m, VecNumMatch);
				rotation = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
			}
			else if (std::regex_match(*s, RegScaleParam)) { //-scale=(0,1,0)
				std::regex_search(s->c_str(), m, VecNumMatch);
				scale = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
			}
			else if (std::regex_search(s->c_str(), m, StringRegex("collider"))) {
				if (m[1] == "aabb") ctype = ColliderType_AABB;
				else if (m[1] == "sphere") ctype = ColliderType_Sphere;
				else if (m[1] == "landscape") ctype = ColliderType_Landscape;
				else if (m[1] == "box") ctype = ColliderType_Box;
			}
			else if (std::regex_search(s->c_str(), m, FloatRegex("mass"))) {
				if (std::stof(m[1]) < 0) return "[c:red]Mass must be greater than 0[c]";
				mass = std::stof(m[1]);
			}
			else if (std::regex_search(s->c_str(), m, BoolRegex("static"))) {
				if (m[1] == "0" || m[1] == "false") staticc = false;
			}
			else if (std::regex_search(s->c_str(), m, FloatRegex("elasticity"))) {
				elasticity = std::stof(m[1]);
			}
			else {
				return "[c:red]Invalid parameter: " + *s + "[c]";
			}
		}

		//cut off the .obj extension for entity name
		char name[64];
		cpystr(name, args[0].substr(0, args[0].size() - 4).c_str(), 63);

		//create the mesh
		u32 id = DengRenderer->CreateMesh(&admin->scene, args[0].c_str());
		Mesh* mesh = DengRenderer->GetMeshPtr(id);

		//collider
		Collider* col = nullptr;
		switch (ctype) {
		case ColliderType_AABB: col = new AABBCollider(mesh, 1); break;
		case ColliderType_Sphere: col = new SphereCollider(1, 1); break;
		case ColliderType_Landscape: col = new LandscapeCollider(mesh);
		case ColliderType_Box: col = new BoxCollider(Vector3(1, 1, 1), 1);
		}

		MeshComp* mc = new MeshComp(mesh, id);
		Physics* p = new Physics(position, rotation, Vector3::ZERO, Vector3::ZERO,
			Vector3::ZERO, Vector3::ZERO, elasticity, mass, staticc);
		AudioSource* s = new AudioSource("data/sounds/Kick.wav", p);
		Movement* mov = new Movement(p);
		Player* pl = new Player(mov);
		admin->player = admin->world->CreateEntityNow(admin, { mc, p, s, col, mov, pl },
			name, Transform(position, rotation, scale));
		admin->controller.playermove = mov;
		return TOSTRING("Added player.");
	}
	return "load_obj <model.obj:String> -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)";
}

void Console::AddRandomCommands(){
	//TODO(sushi,Cmd) reimplement this at some point
	//commands["debug_global"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
	//	GLOBAL_DEBUG = !GLOBAL_DEBUG;
	//	if (GLOBAL_DEBUG) return "GLOBAL_DEBUG = true";
	//	else return "GLOBAL_DEBUG = false";
	//}, "debug_global", "debug_global");
	
	commands["engine_pause"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
											   admin->paused = !admin->paused;
											   if (admin->paused) return "engine_pause = true";
											   else return "engine_pause = false";
										   }, "engine_pause", "toggles pausing the engine");
	
	
	ADDCOMMAND(daytime, "Logs the time in day-time format");
	ADDCOMMAND(time_engine, "Logs the engine times");
	ADDCOMMAND(time_game, "Logs the game times");
	ADDCOMMAND(undo, "Undos previous level editor action");
	ADDCOMMAND(redo, "Redos last undone level editor action");
	ADDCOMMAND(add_player, "Adds a player to the world.");
}

////////////////////////////////////
//// render commands and inputs ////
////////////////////////////////////

void Console::AddRenderCommands() {
	//TODO(delle,Cmd) create material
	
	//TODO(delle,Cmd) create box 
	
	//TODO(delle,Cmd) create planarized box
	
	NEWCOMMAND("render_stats", "Lists different rendering stats for the previous frame", {
				   //TODO(delle,Cmd) this
				   return "";
			   });
	
	NEWCOMMAND("render_options", "render_options <wireframe:Bool>", {
				   if (args.size() > 0) {
					   try {
						   DengRenderer->settings.wireframe = std::stoi(args[0]);
						   return (DengRenderer->settings.wireframe) ? "wireframe=1" : "wireframe=0";
					   }
					   catch (...) {
						   return "render_options <wireframe:Bool>";
					   }
				   }
				   return (DengRenderer->settings.wireframe) ? "wireframe=1" : "wireframe=0";
			   });
	
	commands["spawn_box_uv"] =
		new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
						try{
							std::cmatch m;
							Vector3 position{}, rotation{}, scale = { 1.f, 1.f, 1.f };
							
							for (auto s = args.begin(); s != args.end(); ++s) {
								if (std::regex_match(*s, RegPosParam)) { // -pos=(1,2,3)
									std::regex_search(s->c_str(), m, VecNumMatch);
									position = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
								}
								else if (std::regex_match(*s, RegRotParam)) { //-rot=(1.1,2,3)
									std::regex_search(s->c_str(), m, VecNumMatch);
									rotation = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
								}
								else if (std::regex_match(*s, RegScaleParam)) { //-scale=(0,1,0)
									std::regex_search(s->c_str(), m, VecNumMatch);
									scale = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
								}
								else {
									return "[c:red]Invalid parameter: " + *s + "[c]";
								}
							}
							
							u32 id = DengRenderer->CreateMesh(2, Matrix4::TransformationMatrix(position, rotation, scale));
							
							MeshComp* mc = new MeshComp(DengRenderer->GetMeshPtr(id), id);
							admin->world->CreateEntity(admin, { mc }, "uv_texture_box", Transform(position, rotation, scale));
							
							return TOSTRING("Created textured box with id: ", id);
						}catch(...){
							return "spawn_box_uv -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)";
						}
					}, "spawn_box_uv", "spawn_box_uv -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)");
	
	NEWCOMMAND("mat_texture", "mat_texture <materialID:Uint> <textureType:Uint> <textureID:Uint>", {
				   if (args.size() != 3) { return "material_texture <materialID:Uint> <textureType:Uint> <textureID:Uint>"; }
				   
				   int matID = std::stoi(args[0]);
				   int texType = std::stoi(args[1]);
				   int texID = std::stoi(args[2]);
				   DengRenderer->UpdateMaterialTexture(matID, texType, texID);
				   return TOSTRING("Updated material", matID, "'s texture", texType, " to ", texID);
			   });
	
	NEWCOMMAND("mat_shader", "mat_shader <materialID:Uint> <shaderID:Uint>", {
				   if (args.size() != 2) { return "material_shader <materialID:Uint> <shaderID:Uint>"; }
				   int matID = std::stoi(args[0]);
				   int shader = std::stoi(args[1]);
				   DengRenderer->UpdateMaterialShader(matID, shader);
				   return TOSTRING("Updated material", matID, "'s shader to ", shader);
			   });
	
	NEWCOMMAND("mat_list", "mat_list", {
				   Renderer* r = DengRenderer;
				   std::string out = "[c:yellow]Materials List:\nID  Shader  Albedo  Normal  Specular  Light[c]";
				   for(auto mat : r->materials){
					   out += TOSTRING("\n", mat.id, "  ", shadertostringint[mat.shader], "  ",
									   mat.albedoID, ":", r->textures[mat.albedoID].filename, "  ",
									   mat.normalID, ":", r->textures[mat.normalID].filename, "  ",
									   mat.specularID, ":", r->textures[mat.specularID].filename, "  ",
									   mat.lightID, ":", r->textures[mat.lightID].filename);
				   }
				   return out;
			   });
	
	NEWCOMMAND("shader_reload", "shader_reload <shaderID:Uint>", {
				   if (args.size() != 1) return "shader_reload <shaderID:Uint>";
				   int id = std::stoi(args[0]);
				   if (id == -1) {
					   DengRenderer->ReloadAllShaders();
					   return "[c:magen]Reloading all shaders[c]";
				   }else{
					   DengRenderer->ReloadShader(id);
					   return ""; //printed in renderer
				   }
			   });
	
	//TODO(delle,ReCl) update this to be dynamic when shader loading is (if ever)
	NEWCOMMAND("shader_list", "Lists the shaders and their IDs", {
				   return TOSTRING("[c:yellow]ID    SHADER          Description[c]\n",
								   "0    Flat            Vertex color shading without normal/edge smoothing\n",
								   "1    Phong           Vertex color shading with normal smoothing (good with spheres)\n",
								   "2    TwoD            Vertex color shading with 2D position, rotation, and scale\n",
								   "3    PBR             Physically-based rendering; 4 textures per material\n",
								   "4    Wireframe       Vertex color shading with no polygon fill\n",
								   "5    Lavalamp        Sushi's experimental shader\n",
								   "6    Test0           Testing shader 1\n",
								   "7    Test1           Testing shader 2");
			   });
	NEWCOMMAND("shader_freeze", "Toggles shader data being uploaded to GPU", {
				   DengRenderer->shaderData.freeze = !DengRenderer->shaderData.freeze;
				   return (DengRenderer->shaderData.freeze)? "Shaders frozen" : "Shaders unfrozen";
			   });
	
	NEWCOMMAND("mesh_visible", "mesh_visible <meshID:Uint> <visible:Bool>", {
				   if (args.size() == 2) {
					   try {
						   int meshID = std::stoi(args[0]);
						   bool vis = std::stoi(args[1]);
						   DengRenderer->UpdateMeshVisibility(meshID, vis);
						   return TOSTRING("Setting mesh", meshID, "'s visibility to ", vis);
					   }
					   catch (...) {
						   return "mesh_visible <meshID:Uint> <visible:Bool>";
					   }
				   }
				   return "mesh_visible <meshID:Uint> <visible:Bool>";
			   });
	
	commands["mesh_create"] =
		new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
						try{
							if (args.size() > 0) {
								int meshID = std::stoi(args[0]);
								
								std::cmatch m;
								Vector3 position{}, rotation{}, scale = { 1.f, 1.f, 1.f };
								
								//check for optional params after the first arg
								for (auto s = args.begin() + 1; s != args.end(); ++s) {
									if (std::regex_match(*s, RegPosParam)) { // -pos=(1,2,3)
										std::regex_search(s->c_str(), m, VecNumMatch);
										position = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
									}
									else if (std::regex_match(*s, RegRotParam)) { //-rot=(1.1,2,3)
										std::regex_search(s->c_str(), m, VecNumMatch);
										rotation = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
									}
									else if (std::regex_match(*s, RegScaleParam)) { //-scale=(0,1,0)
										std::regex_search(s->c_str(), m, VecNumMatch);
										scale = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
									}
									else {
										return "[c:red]Invalid parameter: " + *s + "[c]";
									}
								}
								
								u32 id = DengRenderer->CreateMesh(meshID, Matrix4::TransformationMatrix(position, rotation, scale));
								Mesh* ptr = DengRenderer->GetMeshPtr(id);
								
								MeshComp* mc = new MeshComp(ptr, id);
								Physics* p = new Physics(position, rotation);
								AudioSource* s = new AudioSource("data/sounds/Kick.wav", p);
								admin->world->CreateEntity(admin, { mc, p, s });
								
								return TOSTRING("Created mesh with id: ", id, " based on mesh: ", meshID);
							}
							return "mesh_create <meshID:Uint> -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)";
						}catch(...){
							return "mesh_create <meshID:Uint> -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)";
						}
					}, "mesh_create", "mesh_create <meshID:Uint> -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)");
	
	NEWCOMMAND("mesh_batch_material", "mesh_batch_material <meshID:Uint> <batchID:Uint> <materialID:Uint>", {
				   if (args.size() != 3) { return "mesh_batch_material <meshID:Uint> <batchID:Uint> <materialID:Uint>"; }
				   int mesh = std::stoi(args[0]);
				   int batch = std::stoi(args[1]);
				   int mat = std::stoi(args[2]);
				   DengRenderer->UpdateMeshBatchMaterial(mesh, batch, mat);
				   return TOSTRING("Changed mesh", mesh, "'s batch", batch, "'s material to ", mat);
			   });
	
	//mesh_update_matrix, a bit more difficult b/c it should only update the passed arguments
	
	commands["mesh_transform_matrix"] =
		new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
						if (args.size() > 1) {
							Mesh mesh; std::cmatch m;
							Vector3 position{}, rotation{}, scale = { 1.f, 1.f, 1.f };
							
							//check for optional params after the first arg
							for (auto s = args.begin() + 1; s != args.end(); ++s) {
								if (std::regex_match(*s, RegPosParam)) { // -pos=(1,2,3)
									std::regex_search(s->c_str(), m, VecNumMatch);
									position = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
								}
								else if (std::regex_match(*s, RegRotParam)) { //-rot=(1.1,2,3)
									std::regex_search(s->c_str(), m, VecNumMatch);
									rotation = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
								}
								else if (std::regex_match(*s, RegScaleParam)) { //-scale=(0,1,0)
									std::regex_search(s->c_str(), m, VecNumMatch);
									scale = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
								}
								else {
									return "[c:red]Invalid parameter: " + *s + "[c]";
								}
							}
							
							//update the mesh's matrix
							try {
								DengRenderer->TransformMeshMatrix(std::stoi(args[0]), Matrix4::TransformationMatrix(position, rotation, scale));
								return TOSTRING("Transforming mesh", args[0], "'s matrix");
							}
							catch (...) {
								return "mesh_transform_matrix <meshID:Uint> -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)";
							}
						}
						else {
							return "mesh_transform_matrix <meshID:Uint> -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)";
						}
					}, "mesh_transform_matrix", "mesh_transform_matrix <meshID:Uint> -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)");
	
	//TODO(delle,CmdCl) figure out why the macro doesnt work here or on the one above
	commands["load_obj"] =
		new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
						if (args.size() > 0) {
							std::cmatch m;
							Vector3 position{}, rotation{}, scale = { 1.f, 1.f, 1.f };
							float mass = 1.f;
							float elasticity = 0;
							bool staticc = true;
							ColliderType ctype;
							
							//check for optional params after the first arg
							for (auto s = args.begin() + 1; s != args.end(); ++s) {
								if (std::regex_match(*s, RegPosParam)) { // -pos=(1,2,3)
									std::regex_search(s->c_str(), m, VecNumMatch);
									position = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
								}
								else if (std::regex_match(*s, RegRotParam)) { //-rot=(1.1,2,3)
									std::regex_search(s->c_str(), m, VecNumMatch);
									rotation = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
								}
								else if (std::regex_match(*s, RegScaleParam)) { //-scale=(0,1,0)
									std::regex_search(s->c_str(), m, VecNumMatch);
									scale = Vector3(std::stof(m[1]), std::stof(m[2]), std::stof(m[3]));
								}
								else if (std::regex_search(s->c_str(), m, StringRegex("collider"))) {
									if (m[1] == "aabb") ctype = ColliderType_AABB;
									else if (m[1] == "sphere") ctype = ColliderType_Sphere;
									else if (m[1] == "landscape") ctype = ColliderType_Landscape;
									else if (m[1] == "box") ctype = ColliderType_Box;
								}
								else if (std::regex_search(s->c_str(), m, FloatRegex("mass"))) {
									if (std::stof(m[1]) < 0) return "[c:red]Mass must be greater than 0[c]";
									mass = std::stof(m[1]);
								}
								else if (std::regex_search(s->c_str(), m, BoolRegex("static"))) {
									if (m[1] == "0" || m[1] == "false") staticc = false;
								}
								else if (std::regex_search(s->c_str(), m, FloatRegex("elasticity"))) {
									elasticity = std::stof(m[1]);
								}
								else {
									return "[c:red]Invalid parameter: " + *s + "[c]";
								}
							}
							
							//cut off the .obj extension for entity name
							char name[64];
							cpystr(name, args[0].substr(0, args[0].size() - 4).c_str(), 63);
							
							//create the mesh
							u32 id = DengRenderer->CreateMesh(&admin->scene, args[0].c_str());
							Mesh* mesh = DengRenderer->GetMeshPtr(id);
							
							//collider
							Collider* col = nullptr;
							switch (ctype) {
							case ColliderType_AABB: col = new AABBCollider(mesh, 1); break;
							case ColliderType_Sphere: col = new SphereCollider(1, 1); break;
							case ColliderType_Landscape: col = new LandscapeCollider(mesh);
							case ColliderType_Box: col = new BoxCollider(Vector3(1, 1, 1), 1);
							}
							
							MeshComp* mc = new MeshComp(mesh, id);
							Physics* p = new Physics(position, rotation, Vector3::ZERO, Vector3::ZERO, 
													 Vector3::ZERO, Vector3::ZERO, elasticity, mass, staticc);
							AudioSource* s = new AudioSource("data/sounds/Kick.wav", p);
							admin->world->CreateEntity(admin, { mc, p, s, col}, 
													   name, Transform(position, rotation, scale));
							
							return TOSTRING("Loaded mesh ", args[0], " to ID: ", id);
						}
						return "load_obj <model.obj:String> -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)";
					}, "load_obj", "load_obj <model.obj:String> -pos=(x,y,z) -rot=(x,y,z) -scale=(x,y,z)");
	
	NEWCOMMAND("texture_load", "texture_load <texture.png:String> [type:Uint]", {
				   if (args.size() > 0) {
					   Texture tex(args[0].c_str());
					   if (args.size() == 2) { tex.type = u32(std::stoi(args[1])); }
					   u32 id = DengRenderer->LoadTexture(tex);
					   return TOSTRING("Loaded texture ", args[0], " to ID: ", id);
				   }
				   return "texture_load <texture.png:String> [type:Uint]";
			   });
	
	NEWCOMMAND("texture_list", "Lists the textures and their info", {
				   return DengRenderer->ListTextures();
			   });
	
	NEWCOMMAND("texture_type_list", "Lists the texture types and their IDs", {
				   return TOSTRING("[c:yellow]Texture Types: (can be combined)[c]\n"
								   "   0=Albedo, Color, Diffuse\n"
								   "   1=Normal, Bump\n"
								   "   2=Light, Ambient\n"
								   "   4=Specular, Reflective\n"
								   "   8=Cube      (not supported yet)\n"
								   "  16=Sphere    (not supported yet)");
			   });
}

////////////////////////////////////
//// camera commands and inputs ////
////////////////////////////////////

void Console::AddCameraCommands() {
	NEWCOMMAND("cam_info", "Prints camera variables", {
				   return admin->mainCamera->str();
			   });
	
	NEWCOMMAND("cam_matrix_projection", "Prints camera's projection matrix", {
				   return admin->mainCamera->projectionMatrix.str2f();
			   });
	
	NEWCOMMAND("cam_matrix_view", "Prints camera's view matrix", {
				   return admin->mainCamera->viewMatrix.str2f();
			   });
	
	NEWCOMMAND("cam_reset", "Resets camera", {
				   admin->mainCamera->position = Vector3(4.f, 3.f, -4.f);
				   admin->mainCamera->rotation = Vector3(28.f, -45.f, 0.f);
				   return "reset camera";
			   });
	
	commands["cam_vars"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
										   Camera* c = admin->mainCamera;
										   if(args.size() == 0){ return "cam_vars -pos=(x,y,z) -rot=(x,y,z) -static=(Bool) -nearZ=(Float) -farZ=(Float) -fov=(Float)"; }
										   try{
											   std::cmatch m;
											   std::regex nearZ = FloatRegex("nearZ");
											   std::regex farZ = FloatRegex("farZ");
											   std::regex fov = FloatRegex("fov");
											   std::regex freeCam = BoolRegex("static");
											   
											   for (auto s = args.begin(); s != args.end(); ++s) {
												   if (std::regex_match(*s, RegPosParam)) {
													   std::regex_search(s->c_str(), m, VecNumMatch);
													   c->position = {std::stof(m[1]), std::stof(m[2]), std::stof(m[3])};
												   }
												   else if (std::regex_match(*s, RegRotParam)) {
													   std::regex_search(s->c_str(), m, VecNumMatch);
													   c->rotation = {std::stof(m[1]), std::stof(m[2]), std::stof(m[3])};
												   }
												   else if (std::regex_search(s->c_str(), m, nearZ)) {
													   c->nearZ = std::stof(m[1]);
												   }
												   else if (std::regex_search(s->c_str(), m, farZ)) {
													   c->farZ = std::stof(m[1]);
												   }
												   else if (std::regex_search(s->c_str(), m, fov)) {
													   c->fov = std::stof(m[1]);
												   }
												   else if (std::regex_search(s->c_str(), m, freeCam)) {
													   if(m[1].str() == "0" || m[1].str() == "false"){
														   c->freeCamera = true; //backwards cus naming
													   }else{
														   c->freeCamera = false;
													   }
												   }
												   else {
													   return "[c:red]Invalid parameter: " + *s + "[c]";
												   }
											   }
											   
											   c->UpdateProjectionMatrix();
											   return admin->mainCamera->str();
										   }catch(...){
											   return "cam_vars -pos=(x,y,z) -rot=(x,y,z) -static=(Bool) -nearZ=(Float) -farZ=(Float) -fov=(Float)";
										   }
									   }, "cam_vars", "cam_vars -pos=(x,y,z) -rot=(x,y,z) -static=(Bool) -nearZ=(Float) -farZ=(Float) -fov=(Float)");
}

void Console::AddConsoleCommands() {
	commands["listc"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
										std::string allcommands = "";
										
										for (std::pair<std::string, Command*> c : DengConsole->commands) {
											allcommands += c.first + "\n";
										}
										
										return allcommands;
									}, "listc", "lists all avaliable commands");
	
	commands["help"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
									   if (args.size() == 0 || (args.size() == 1 && args[0] == "")) {
										   return "help \nprints help about a specified command. \nuse listc to display avaliable commands";
									   }
									   else if (DengConsole->commands.find(args[0]) != DengConsole->commands.end()) {
										   Command* c = DengConsole->commands.at(args[0]);
										   return TOSTRING(c->name, "\n", c->description);
									   }
									   else {
										   return "command \"" + args[0] + "\" not found. \n use \"listc\" to list all commands.";
									   }
								   }, "help", "prints help about a specified command. \nignores any argument after the first.");
	commands["save"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
									   
									   admin->Save();
									   return "Saved.";
									   
								   }, "save", "saves the state of Entity Admin");
	
	commands["alias"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string 
									{
										
										if (args.size() == 0) {
											return "alias \nassign an alias to another command to call it with a different name\n alias (alias name) (command name)";
										}
										else if(args.size() == 1){
											return "you must specify a command to assign to this alias.";
										}
										else if (args.size() == 2) {
											Command* com;
											try {
												com = DengConsole->commands.at(args[1]);
												DengConsole->commands.emplace(args[0], com);
												
												std::string data = args[0] + " " + args[1] + "\n";
												std::vector<char> datav;
												
												for (auto c : data) {
													datav.push_back(c);
												}
												
												deshi::appendFile(deshi::getConfig("aliases.cfg"), datav, datav.size());
												
												return "[c:green]alias \"" + args[0] + "\" successfully assigned to command \"" + args[1] + "\"[c]";
											}
											catch (...) {
												return "[c:red]command \"" + args[1] + "\" not found in the commands list[c]";
											}
										}
										else {
											return "too many arguments specified.";
										}
										
									}, "alias", "assign an alias to another command to call it with a different name");
	
	commands["bind"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string 
								   {
									   
									   if (args.size() == 0) {
										   return "bind \nassign a command to a key\n bind (key) (command name)";
									   }
									   else if(args.size() == 1){
										   return "you must specify a command to assign to this bind.";
									   }
									   else {
										   std::string s = "";
										   for (int i = 1; i < args.size(); i++) {
											   s += args[i] + " ";
										   }
										   Key::Key key;
										   
										   try {
											   key = DengKeys.stk.at(args[0]);
											   DengInput->binds.push_back(std::pair<std::string, Key::Key>(s, key));
											   std::vector<char> datav;
											   for (auto c : args[0] + " " + s) {
												   datav.push_back(c);
											   }
											   datav.push_back('\n');
											   deshi::appendFile(deshi::getConfig("binds.cfg"), datav, datav.size());
											   return "[c:green]key \"" + args[0] + "\" successfully bound to \n" + s + "[c]";
										   }
										   catch(...){
											   return "[c:red]key \"" + args[0] + "\" not found in the key list.[c]";
										   }
										   
										   
									   }
									   
								   }, "bind", "bind a command to a key");
	
	
	
}

//TODO(delle,InPh) update entity movement commands to be based on EntityID
void Console::AddSelectedEntityCommands() {
	//// translation ////
	commands["reset_position"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												 if (admin->selectedEntity) {
													 if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														 p->acceleration = Vector3::ZERO;
														 p->velocity = Vector3::ZERO;
														 p->position = Vector3::ZERO;
													 }
												 }
												 return "";
											 }, "reset_position", "reset_position <EntityID> [String: xyz]");
	
	commands["reset_position_x"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												   if (admin->selectedEntity) {
													   if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														   p->acceleration = Vector3(0, p->acceleration.y, p->acceleration.z);
														   p->velocity = Vector3(0, p->velocity.y, p->velocity.z);
														   p->position = Vector3(0, p->position.y, p->position.z);
													   }
												   }
												   return "";
											   }, "reset_position_x", "temp");
	
	commands["reset_position_y"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												   if (admin->selectedEntity) {
													   if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														   p->acceleration = Vector3(p->acceleration.x, 0, p->acceleration.z);
														   p->velocity = Vector3(p->velocity.x, 0, p->velocity.z);
														   p->position = Vector3(p->position.x, 0, p->position.z);
													   }
												   }
												   return "";
											   }, "reset_position_y", "temp");
	
	commands["reset_position_z"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												   if (admin->selectedEntity) {
													   if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														   p->acceleration = Vector3(p->acceleration.x, p->acceleration.y, 0);
														   p->velocity = Vector3(p->velocity.x, p->velocity.y, 0);
														   p->position = Vector3(p->position.x, p->position.y, 0);
													   }
												   }
												   return "";
											   }, "reset_position_z", "temp");
	
	commands["reset_velocity"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												 if (admin->selectedEntity) {
													 if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														 p->acceleration = Vector3::ZERO;
														 p->velocity = Vector3::ZERO;
													 }
												 }
												 return "";
											 }, "reset_velocity", "reset_position <EntityID> [String: xyz]");
	
	commands["translate_right"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												  if (admin->selectedEntity) {
													  if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														  p->AddInput(Vector3::RIGHT);
													  }
												  }
												  return "";
											  }, "translate_right", "translate_right <EntityID> <amount> [speed]");
	
	commands["translate_left"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												 if (admin->selectedEntity) {
													 if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														 p->AddInput(Vector3::LEFT);
													 }
												 }
												 return "";
											 }, "translate_left", "translate_left <EntityID> <amount> [speed]");
	
	commands["translate_up"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
											   if (admin->selectedEntity) {
												   if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
													   p->AddInput(Vector3::UP);
												   }
											   }
											   return "";
										   }, "translate_up", "translate_up <EntityID> <amount> [speed]");
	
	commands["translate_down"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												 if (admin->selectedEntity) {
													 if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														 p->AddInput(Vector3::DOWN);
													 }
												 }
												 return "";
											 }, "translate_down", "translate_down <EntityID> <amount> [speed]");
	
	commands["translate_forward"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
													if (admin->selectedEntity) {
														if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
															p->AddInput(Vector3::FORWARD);
														}
													}
													return "";
												}, "translate_forward", "translate_forward <EntityID> <amount> [speed]");
	
	commands["translate_backward"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
													 if (admin->selectedEntity) {
														 if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
															 p->AddInput(Vector3::BACK);
														 }
													 }
													 return "";
												 }, "translate_backward", "translate_backward <EntityID> <amount> [speed]");
	
	//// rotation ////
	
	commands["reset_rotation"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												 if (admin->selectedEntity) {
													 if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														 p->rotAcceleration = Vector3::ZERO;
														 p->rotVelocity = Vector3::ZERO;
														 p->rotation = Vector3::ZERO;
													 }
												 }
												 return "";
											 }, "reset_rotation", "reset_rotation <EntityID> [String: xyz]");
	
	commands["reset_rotation_x"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												   if (admin->selectedEntity) {
													   if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														   p->rotAcceleration = Vector3(0, p->rotAcceleration.y, p->rotAcceleration.z);
														   p->rotVelocity = Vector3(0, p->rotVelocity.y, p->rotVelocity.z);
														   p->rotation = Vector3(0, p->rotation.y, p->rotation.z);
													   }
												   }
												   return "";
											   }, "reset_rotation_x", "temp");
	
	commands["reset_rotation_y"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												   if (admin->selectedEntity) {
													   if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														   p->rotAcceleration = Vector3(p->rotAcceleration.x, 0, p->rotAcceleration.z);
														   p->rotVelocity = Vector3(p->rotVelocity.x, 0, p->rotVelocity.z);
														   p->rotation = Vector3(p->rotation.x, 0, p->rotation.z);
													   }
												   }
												   return "";
											   }, "reset_rotation_y", "temp");
	
	commands["reset_rotation_z"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
												   if (admin->selectedEntity) {
													   if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
														   p->rotAcceleration = Vector3(p->rotAcceleration.x, p->rotAcceleration.y, 0);
														   p->rotVelocity = Vector3(p->rotVelocity.x, p->rotVelocity.y, 0);
														   p->rotation = Vector3(p->rotation.x, p->rotation.y, 0);
													   }
												   }
												   return "";
											   }, "reset_rotation_z", "temp");
	
	commands["reset_rotation_velocity"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
														  if (admin->selectedEntity) {
															  if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
																  p->rotAcceleration = Vector3::ZERO;
																  p->rotVelocity = Vector3::ZERO;
															  }
														  }
														  return "";
													  }, "reset_rotation_velocity", "reset_rotation_velocity <EntityID> [String: xyz]");
	
	commands["rotate_+x"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
											if (admin->selectedEntity) {
												if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
													p->rotVelocity += Vector3(5, 0, 0);
												}
											}
											return "";
										}, "rotate_+x", "rotate_+x <EntityID> <amount> [speed]");
	
	commands["rotate_-x"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
											if (admin->selectedEntity) {
												if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
													p->rotVelocity += Vector3(-5, 0, 0);
												}
											}
											return "";
										}, "rotate_-x", "rotate_-x <EntityID> <amount> [speed]");
	
	commands["rotate_+y"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
											if (admin->selectedEntity) {
												if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
													p->rotVelocity += Vector3(0, 5, 0);
												}
											}
											return "";
										}, "rotate_+y", "rotate_+y <EntityID> <amount> [speed]");
	
	commands["rotate_-y"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
											if (admin->selectedEntity) {
												if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
													p->rotVelocity += Vector3(0, -5, 0);
												}
											}
											return "";
										}, "rotate_-y", "rotate_-y <EntityID> <amount> [speed]");
	
	commands["rotate_+z"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
											if (admin->selectedEntity) {
												if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
													p->rotVelocity += Vector3(0, 0, 5);
												}
											}
											return "";
										}, "rotate_+z", "rotate_+z <EntityID> <amount> [speed]");
	
	commands["rotate_-z"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
											if (admin->selectedEntity) {
												if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
													p->rotVelocity += Vector3(0, 0, -5);
												}
											}
											return "";
										}, "rotate_-z", "rotate_-z <EntityID> <amount> [speed]");
	
	//// other ////
	/*
	commands["add_force"] = new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
													   //TODO( sushi,Re) implement ScreenToWorld for ortho projection
													   if (USE_ORTHO) {
													   LOG("\nWarning: ScreenToWorld not yet implemented for orthographic projection. World interaction with mouse will not work.\n");
												   }
												   else {
													   if (admin->selectedEntity) {
														   if (Physics* p = admin->selectedEntity->GetComponent<Physics>()) {
															   Vector3 pos = Math::ScreenToWorld(DengInput->mousePos, admin->mainCamera->projectionMatrix,
																								 admin->mainCamera->viewMatrix, DengWindow->dimensions);
															   //cant remember what this is doing and will fix later
															   //Vector3 clickPos = Math::ScreenToWorld(DengInput->mouseClickPos, admin->mainCamera->projectionMatrix,
															   //admin->mainCamera->viewMatrix, DengWindow->dimensions);
															   //TODO(delle,PhIn) test that you can add force to a selected entity
															   //Physics::AddForce(nullptr, p, (pos - clickPos).normalized() * 5);
														   }
													   }
												   }
												   return "";
												   }, "add_force", "add_force <EntityID> <force_vector> [constant_force?]");
	*/
}

void Console::AddWindowCommands() {
	NEWCOMMAND("quit", "exits the application", {
				   DengWindow->Close();
				   return("");
			   });
	
	commands["window_display_mode"] =
		new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
						if (args.size() != 1) { return "display_mode <mode: Int>"; }
						try {
							int mode = std::stoi(args[0]);
							switch (mode) {
								case(0): {
									DengWindow->UpdateDisplayMode(DisplayMode::WINDOWED);
									return "display_mode=windowed"; }
								case(1): {
									DengWindow->UpdateDisplayMode(DisplayMode::BORDERLESS);
									return "display_mode=borderless windowed"; }
								case(2): {
									DengWindow->UpdateDisplayMode(DisplayMode::FULLSCREEN);
									return "display_mode=fullscreen"; }
								default: {
									return "display_mode: 0=Windowed, 1=BorderlessWindowed, 2=Fullscreen"; }
							}
						}
						catch (...) {
							return "display_mode: 0=Windowed, 1=BorderlessWindowed, 2=Fullscreen";
						}
					}, "window_display_mode", "window_display_mode <mode:Int>");
	
	commands["window_cursor_mode"] =
		new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
						if (args.size() != 1) { return "cursor_mode <mode:Int>"; }
						try {
							int mode = std::stoi(args[0]);
							switch (mode) {
								case(0): {
									DengWindow->UpdateCursorMode(CursorMode::DEFAULT);
									return "cursor_mode=default"; }
								case(1): {
									DengWindow->UpdateCursorMode(CursorMode::FIRSTPERSON);
									return "cursor_mode=first person"; }
								case(2): {
									DengWindow->UpdateCursorMode(CursorMode::HIDDEN);
									return "cursor_mode=hidden"; }
								default: { return "cursor_mode: 0=Default, 1=FirstPerson, 2=Hidden"; }
							}
						}
						catch (...) {
							return "cursor_mode: 0=Default, 1=FirstPerson, 2=Hidden";
						}
					}, "window_cursor_mode", "window_cursor_mode <mode:Int>");
	
	commands["window_raw_input"] =
		new Command([](EntityAdmin* admin, std::vector<std::string> args) -> std::string {
						if (args.size() != 1) { return "raw_input <input:Boolean>"; }
						try {
							int mode = std::stoi(args[0]);
							switch (mode) {
								case(0): { DengWindow->UpdateRawInput(false); return "raw_input=false"; }
								case(1): { DengWindow->UpdateRawInput(true); return "raw_input=true"; }
								default: { return "raw_input: 0=false, 1=true"; }
							}
						}
						catch (...) {
							return "raw_input: 0=false, 1=true";
						}
					}, "window_raw_input", "raw_input <input:Boolean>; Only works in firstperson cursor mode");
	
	NEWCOMMAND("window_resizable", "window_raw_input <resizable:Boolean>", {
				   if (args.size() != 1) { return "window_resizable <resizable:Boolean>"; }
				   try {
					   int mode = std::stoi(args[0]);
					   switch (mode) {
						   case(0): { DengWindow->UpdateResizable(false); return "window_resizable=false"; }
						   case(1): { DengWindow->UpdateResizable(true); return "window_resizable=true"; }
						   default: { return "window_resizable: 0=false, 1=true"; }
					   }
				   }
				   catch (...) {
					   return "window_resizable: 0=false, 1=true";
				   }
			   });
	
	NEWCOMMAND("window_info", "Prints window variables", {
				   return DengWindow->str();
			   });
}

void Console::AddAliases() {
	std::ifstream aliases;
	
	if (deshi::getConfig("aliases.cfg") != "") {
		aliases = std::ifstream(deshi::getConfig("aliases.cfg"), std::ios::in);
		
		char* c = (char*)malloc(255);
		aliases.getline(c, 255);
		std::string s(c);
		
		std::string alias = s.substr(0, s.find_first_of(" "));
		std::string command = s.substr(s.find_first_of(" ") + 1, s.length());
		
		Command* com;
		
		try {
			com = commands.at(command);
			commands.emplace(alias, com);
		}
		catch (...) {
			ERROR("Unknown command \"", command, "\" was attempted to be aliased from aliases.cfg");
		}
	}
	else {
		LOG("Creating aliases file..");
		deshi::writeFile(deshi::dirConfig() + "aliases.cfg", "", 0);
		
		return;
	}
	
}





///////////////////////////////////////////////////////////////////////
// console init and update
//////////////////////////////////////////////////////////////////////

void Console::Init() {
	compstrmap.emplace("AudioSource", []() { return new AudioSource(); });
	compstrmap.emplace("Collider",    []() { return new Collider(); });
	compstrmap.emplace("MeshComp",    []() { return new MeshComp(); });
	compstrmap.emplace("Physics",     []() { return new Physics(); });
	
	AddLog("[c:dcyan]Deshi Console ver. 0.5.1[c]");
	AddLog("\"listc\" for a list of commands\n\"help {command}\" to view a commands help page");
	AddLog("see console_release_notes.txt for version information");
	AddLog("\n[c:dyellow]Console TODOS:[c]");
	AddLog(
		   "> implement argument completion for commands\n"
		   "> implement arguments for commands that need them\n"
		   "> add help to commands that don't have a descriptive help yet\n"
		   "> fix tabcompletion when trying to complete the first word\n"
		   "> (maybe) rewrite to use characters in the buffer rather than whole strings\n"
		   "> (maybe) implement showing autocomplete as you type");
	
	AddRandomCommands();
	AddRenderCommands();
	AddCameraCommands();
	AddConsoleCommands();
	AddSelectedEntityCommands();
	AddWindowCommands();
	AddAliases();
}

void Console::Update() {
	ZoneScoped;

	if (dispcon) {
		DrawConsole();
		IMGUI_KEY_CAPTURE = true;
	}else{
		IMGUI_KEY_CAPTURE = false;
	}
	
	if (buffersize >= 120000) {
		FlushBuffer();
		buffer.clear();
		buffersize = 0;
	}
	
	if (dispcon && cons_error_warn) cons_error_warn = false;
}

//Flush the buffer at program close and clean up commands
//TODO(sushi, Con) fix this once we have new command system
void Console::CleanUp() {
	FlushBuffer();
	//for (auto pair : commands) { 
	//	if (pair.second) {
	//		try {
	//			delete pair.second; 
	//			pair.second = nullptr; 
	//		}
	//		catch (...) {
	//			pair.second = nullptr;
	//		}
	//	}
	//} commands.clear();
}