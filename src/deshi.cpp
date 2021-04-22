/* deshi
TODO Tags:
As  Assets    Cl  Clean Up Code    Cmd Command         Co  Core        Con Console
En  Entity    Fu  Fun              Fs  Filesystem      Ge  Geometry    In  Input
Ma  Math      Oth Other            Op  Optimization    Ph  Physics     Re  Render
Sh  Shader    So  Sound            Ui  UI              Vu  Vulkan      Wi  Window

TODO Style: TODO(person,tags) description
	  eg: TODO(delle) no tag or date 
eg: TODO(sushi,ReOp) render,optimization tags for sushi

The person listed doesn't necessarily have to be you, and can be someone else
if you feel they would handle the problem better. It should generally be you though.

Major Ungrouped TODOs
---------------------
add shaders: PBR (4textures)
settings file(s) [keybinds, video, audio, etc]
create a demo level
add player movement and player entity
figure out why selecting sometimes selects outside of an object and sometimes doesnt select inside of an object
implement string returns, better descriptions, and parameter parsing on every command (use spawn_box as reference)

Minor Ungrouped TODOs
---------------------
create a hot-loadable global vars file
add a general logging system with log levels and locations
add a component_state command to print state of a component (add str methods to all components/systems)
make our own unordered_map and map that is contiguous (array of pairs basically, hash mapped keys)
add device info command (graphics card, sound device, monitor res, etc)
pool/arena components and entities for better performance
replace/remove external dependencies/includes were possible (glm, tinyobj)
add Qol (quality of life) tag to TODOP
add Camera tag to TODOP
look into integrating TODOP with Discord
add yaxis line
begin reimplementing sound system and maybe rethink its design a bit

Render TODOs
------------
redo MeshVk so its only child meshes
add lighting and shadows
add 2D shader and interface functions
add vertex editing
fix texture transparency
add RenderSettings loading and usage
check those vulkan-tutorial links for the suggestions
avoid having 3 copies of a mesh (model, meshVK, vulkan)
add instancing
look into adding volk for faster loading/function calls
add buffer pre-allocation and arenas for vertices/indices/textures/etc
multi-threaded command buffers, shader loading, image loading
find a way to forward declare vulkan stuff and move the include to the cpp

Level Editor TODOs
------------
orbitting camera for rotating around objects
context menu when right clicking on an object 
scaling objects
copy/pasting objects
typing numbers while grabbing/rotating/scaling for precise manipulation (like in Blender)
adding components
manipulating component/entity data through UI
changing materials/textures through UI
highlighing selected object (maybe AABB for now but figure out precise outlining later)
implement grabbing/rotating/scaling with a visual tool thing (like in Unreal)
world axis in top right (like we used to have)
orthographic side views
(maybe) multiple viewports
add showing axis lines through object when axis grabbing once we have lines in Vulkan
show all textures/shaders on an object instead of just the first one
keybind to move camera to object (like Blender's NPMINUS)
implement orthographic grabbing 

Physics/Atmos TODOs
-------------
finish imlementing AABB vs AABB collisions
make colliders obey scale
redo main physics loop
add physics collision sweeping
add physics based collision resolution for remaining collider primitives
add physics interaction functions
implement collision manifold generation
implement Complex Colliders 

Console TODOs
-------------
!!!implement console popout
fix console color parsing regex to be able to match with brackets inside
OR change how we format color to something less common than brackets
reformat to use a single character buffer rather than the line/subline thing its doing now
implement buffer clipping using ImGui's clipper
implement command chaining
(maybe) add auto complete for arguments of commands
add console flag for showing text in bottom right message bar like error does
command to print all avaliable keys for binding
command to print all keybinds, with (maybe) an option for printing only contextual keybinds
if multiple errors are sent to the console before the user opens it, show a number in the debug bar indicating how many
make binds and aliases check if one already exists for a key or a command. if a key already exists probably just overwrite it?
implement filtering console buffer by function and file name (add __FILENAME__ and __FUNCTION__ or whatever it is to the defines)

UI TODOs
--------
2D shader
add a UI popup when reloading shaders
add UI color palettes for easy color changing
renaming entities from entity list
redo debug bar to be more informative and have different modes
(maybe) implement a way to push data to something in the DebugLayer, sort of how we had before with BufferLog so you can see it without opening console

Math TODOs
----------
cleanup math library (remove redundant/old things, make functions more consistent, etc.)
add quaternions and converions between them and other linear algebra primitives
replace glm :)

Fun TODOs
---------
look into implementing Lua 
look into making a function that takes in the types on a component and formats binary for saving and what not like what were currently doing for typeHeader in EntityAdmin Save()

Bug Board
---------
dragging the console scroll bar doesn't work
after spawning a decent amount of objects and clicking, HandleSelectEntity throws an exception and 
____the batchArray size of whatever mesh its checking is something like 400000000000
	____it looks like some sort of corrupt mesh makes its way in there somehow?
____TODO(sushi) check if ^this^ is still an issue, i added a NOTE on MeshComp::Init() saying what maybe fixed it
look into scaling not rotating (scaling is probably being done in world not local)
console scrolls past top and bottom
calling a components destructor causes a crash related the component layers
selecting scaled meshes doesnt work; i think because we dont scale the vertices when checking for an intersection
AABB collider generation from Mesh* is wrong
sometimes MeshComp is assigned a nonexistant mesh

*/


#include "core.h"
#include "EntityAdmin.h"

Time* g_time;
Window* g_window;
Input* g_input;
Console* g_console;
Renderer* g_renderer;
EntityAdmin* g_admin;

struct DeshiEngine {
	Time time;
	Window window;
	Input input;
	EntityAdmin admin;
	Console console;
	Renderer renderer;
	deshiImGui imgui;
	TIMER_START(t_d); TIMER_START(t_f);
	
	//TODO(delle,Fs) setup loading a config file to a config struct
	void LoadConfig() {
		
	}
	
	void Start() {

		//enforce deshi file system
		deshi::enforceDirectories();
		
		//init core
		LoadConfig();
		time.Init(300); //300 tps for physics
		g_time = &time;
		window.Init(&input, 1280, 720); //inits input as well
		g_window = &window;
		g_input = &input;
		console.Init();
		g_console = &console;
		renderer.Init(&time, &input, &window, &imgui); //inits imgui as well
		g_renderer = &renderer;
		
		//init game admin
		admin.Init();
		g_admin = &admin;
		
		LOG("Finished deshi initialization in ",TIMER_END(t_d),"ms");
		
		//start main loop
		while (!glfwWindowShouldClose(window.window) && !window.closeWindow) {
			ZoneScoped;

			glfwPollEvents();
			Update();
			FrameMark;
		}
		
		//admin.Save();
		
		//cleanup
		imgui.Cleanup();
		renderer.Cleanup();
		window.Cleanup();
		console.CleanUp();
	}
	
	bool Update() {
		TIMER_RESET(t_d); time.Update();     time.timeTime = TIMER_END(t_d);
		TIMER_RESET(t_d); window.Update();   time.windowTime = TIMER_END(t_d);
		TIMER_RESET(t_d); input.Update();    time.inputTime = TIMER_END(t_d);
		imgui.NewFrame();                                                            //place imgui calls after this
		TIMER_RESET(t_d); admin.Update();    time.adminTime = TIMER_END(t_d);
		TIMER_RESET(t_d); console.Update();  time.consoleTime = TIMER_END(t_d);
		TIMER_RESET(t_d); renderer.Render(); time.renderTime = TIMER_END(t_d);       //place imgui calls before this
		//entityAdmin.PostRenderUpdate();
		
		time.frameTime = TIMER_END(t_f); TIMER_RESET(t_f);


		return true;
	}
};

int main() {
	DeshiEngine engine;
	engine.Start();
	
	int debug_breakpoint = 0;
}