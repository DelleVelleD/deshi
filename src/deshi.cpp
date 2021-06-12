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
create a demo level
sushi->rewrite the events menu
____also, triggers need to be able to filter what causes them to activate

Minor Ungrouped TODOs
---------------------
custom String8 and String16 types as a replacement to std::string
create a hot-loadable global vars file
detach camera from the renderer so that the camera component isnt calling the renderer
deshi or admin callback function that allows for displaying some sort of indicator that stuff is loading
____the call back function could be on deshi, which updates imgui and/or renderer only and then calls on entity admin
____to update it's canvas system.
think of a way to implement different events being sent between comps as right now it's only one
____is this necessary though? we can define these events at run time, but the connections must be made through UI
____so maybe have a UI option that allows the comps update function to handle it and only connects them.
____actually having an option for anything other than collider is kind of useless soooo maybe 
____get rid of event on every component or just only let u choose that event on colliders
change undo's to never use pointers and have undos that can act like linked lists to chain them
figure out why selecting sometimes selects outside of an object and sometimes doesnt select inside of an object
settings file(s) [keybinds, video, audio, etc]
add a general logging system with log levels and locations (for filtering)
add a component_state command to print state of a component (add str methods to all components/systems)
make our own unordered_map and map that is contiguous (array of pairs basically, hash mapped keys)
____also allow it to store up to 3 types
add device_info command (graphics card, sound device, monitor res, etc)
pool/arena components and entities for better performance
replace/remove external dependencies/includes (glm, tinyobj, std)
add Qol (quality of life) tag to TODOP
add Camera tag to TODOP
look into integrating TODOP with Discord
begin reimplementing sound system and maybe rethink its design a bit
replace std::pair with pair throughout the project
move admin times from time.h to admin.h (and the format function)
remove extra collider component types and use the collider one instead
____ComponentType_AABBCollider vs ComponentType_Collider
add 2d mesh component and component type for image and UI drawing
rename isStatic in physics.h to staticPosition
fix DESH material and event saving/loading

Render TODOs
------------
get debugPrintf extension to work
debug normals
____https://github.com/SaschaWillems/Vulkan/blob/master/examples/geometryshader/geometryshader.cpp
____https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/glsl/geometryshader/normaldebug.geom
____you can test it through PHONG shader for now
change UpdateMaterialTexture to take in a textureType
look into getting info from shaders, or setting up compute shaders
____the primary reason being that we need to optimize outlining objects, which will
____involve clipping triangles and stuff
redo MeshVk so its only child meshes
____avoid having 3 copies of a mesh (model, meshVK, vulkan)
ability to do transparency in a fragment shader eg. we can do outColor = vec4(1,1,1,0.5)
___this would be for experimenting with volumetrics, making a window shader w/o need for textures, etc.
add vertex editing interface functions
add lighting and shadows
add 2D shader and interface functions
add face normal and tangents to vertex buffer
fix texture transparency
add RenderSettings loading and usage
check those vulkan-tutorial links for the suggestions and optimizations
add instancing
look into adding volk for faster loading/function calls
add buffer pre-allocation and arenas for vertices/indices/textures/etc
multi-threaded command buffers, shader loading, image loading
find a way to forward declare vulkan stuff and move the include to the cpp
SSBOs in shaders so we can pass variable length arrays to it 
geometry shaders

Level Editor and Inspector TODOs
------------------
combine create tab into entities tab
add transfering the player pointer between entities that have an actor comp (combo in Global Tab)
orbitting camera for rotating around objects
context menu when right clicking on an object 
scaling objects
copy/pasting objects
typing numbers while grabbing/rotating/scaling for precise manipulation (like in Blender)
implement grabbing/rotating/scaling with a visual tool thing (like in Unreal)
world axis in top right (like we used to have)
orthographic side views
(maybe) multiple viewports
add showing axis lines through object when axis grabbing once we have lines in Vulkan
implement orthographic grabbing 
entity filtering in entity list
combine undo manager into editor file

Physics/Atmos TODOs
-------------
make collider trigger latching a boolean, so it can continuous trigger an event while an obj is in it
____dont forget that it's in a physics tick though, so you have to make sure it doesn't do it 300 times per frame
figure out how to handle multi events being set for a collider. for other components its not really an issue
___because u just choose what event to send at run time, but i don't want to do this inside of the collider
___functions themselves, as it would be way too much clutter. maybe a helper function on collider to choose what to do 
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
fix regex match variable to be smatch instead of cmatch, so we don't have to keep turning every string into a c string


UI TODOs
--------
look into easier hover checking and input intercepting for imgui
____https://github.com/ocornut/imgui/issues/52
2D shader (and handle ImGui ourselves)
add a UI popup when reloading shaders
add UI color palettes for easy color changing
redo debug bar to be more informative and have different modes
(maybe) implement a way to push data to something in the DebugLayer
____sort of how we had before with BufferLog so you can see it without opening console

Math TODOs
----------
add functions and members similar to what glsl/glm has where you can do stuff like 
____v.xy, v.yz, as well as operators for these things if possible. Prefer them to be member variables and not functions :)
____you probably just need to add a Vector2/3 for each permutation of each vector
cleanup math library (remove redundant/old things, make functions more consistent, etc.)
add quaternions and converions between them and other linear algebra primitives
replace glm :)

Fun TODOs
---------
look into implementing Lua 
look into making a function that takes in the types on a component and formats binary for saving and what not 
____like what were currently doing for typeHeader in EntityAdmin Save()
write a shader that displays textures like it would on a monitor, so like you have an array of
____rgb lights that make up a single pixel of a texture and stuff 
write a preprocessing/postprocessing compiler that makes saving easier

Bug Board
---------
after spawning a decent amount of objects and clicking, HandleSelectEntity throws an exception and 
____the batchArray size of whatever mesh its checking is something like 400000000000
	____it looks like some sort of corrupt mesh makes its way in there somehow?
look into scaling not rotating (scaling is probably being done in world not local)
sometimes MeshComp is assigned a nonexistant mesh
____temp fix by checking if minimized, but need to find root cause
program breakpoints when pressing F12 in a .dll on a different thread than main (even when we have no F12 binds)
____read this to try to fix: http://www.debuginfo.com/tips/userbpntdll.html
some UI can be clicked thru and select the entity
the program crashes if default asset files are not present
____we can store the text in the actual code and create the file from the code, like keybinds.cfg

*/

#include "core.h"
#include "game/admin.h"

Time*		 g_time;
Window*		 g_window;
Input*		 g_input;
Console*	 g_console;
Renderer*	 g_renderer;
EntityAdmin* g_admin;
Debug*       g_debug;


struct DeshiEngine {
	Time time;
	Window window;
	Input input;
	EntityAdmin admin;
	Console console;
	Renderer renderer;
	deshiImGui imgui;
	Debug debug;
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
		g_debug = &debug;
		
		//init game admin
		admin.Init();
		g_admin = &admin;
		
		LOG("Finished deshi initialization in ", TIMER_END(t_d), "ms");
		
		//start main loop
		while (!glfwWindowShouldClose(window.window) && !window.closeWindow) {
			glfwPollEvents();
			Update();
		}
		
		//cleanup
		admin.Cleanup();
		imgui.Cleanup();
		renderer.Cleanup();
		window.Cleanup();
		console.CleanUp();
	}
	
	bool Update() {
		TIMER_RESET(t_d); time.Update();            time.timeTime = TIMER_END(t_d);
		TIMER_RESET(t_d); window.Update();          time.windowTime = TIMER_END(t_d);
		TIMER_RESET(t_d); input.Update();           time.inputTime = TIMER_END(t_d);
		imgui.NewFrame();                                                              //place imgui calls after this
		TIMER_RESET(t_d); admin.Update();           time.adminTime = TIMER_END(t_d);
		TIMER_RESET(t_d); console.Update();         time.consoleTime = TIMER_END(t_d);
		TIMER_RESET(t_d); renderer.Render();        time.renderTime = TIMER_END(t_d);  //place imgui calls before this
		TIMER_RESET(t_d); admin.PostRenderUpdate(); time.adminTime += TIMER_END(t_d);
		g_debug->Update(); //TODO(sushi) put a timer on this
		time.frameTime = TIMER_END(t_f); TIMER_RESET(t_f);
		return true;
	}
};

int main() {
	DeshiEngine engine;
	engine.Start();
	
	int debug_breakpoint = 0;
}