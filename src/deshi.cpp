/* deshi
TODO Tags:

As  Assets    Cl  Clean Up Code    Cmd Command         Co  Core        Con Console
En  Entity    Fu  Fun              Fs  Filesystem      Ge  Geometry    In  Input
Ma  Math      Oth Other            Op  Optimization    Ph  Physics     Re  Render
Sh  Shader    So  Sound            Ui  UI              Vu  Vulkan      Wi  Window

TODO Style: TODO(person,tags) description
Rules: tags can be empty but still requires a comma, date can be empty
	  eg: TODO(03/12/2021,,delle) no tag or date; TODO(ro,sushi) render,optimization tags for sushi made on that date

The person listed doesn't necessarily have to be you, and can be someone else
if you feel they would handle the problem better.
It should generally be you though.

Major Ungrouped TODOs
---------------------
add device info debug window (graphics card, sound device, monitor res, etc)
add a component_state command to print state of a component
fix mesh rotations begin global
add shaders: PBR (4textures)
settings file(s) [keybinds, video, audio, etc]
fix program stalling when Keybinds cant find the keybind file
make our own unordered_map and map that is contiguous (array of pairs basically, hash mapped keys)

Minor Ungrouped TODOs
---------------------
cleanup Triangle and remove unused things
create templated component tuple iterator that loops thru a vector and returns an iterator of components of type
pool/arena components and entities for better performance
implement string returns, better descriptions, and parameter parsing on every command (use spawn_box as reference)
replace/remove external dependencies/includes were possible (glm, boost, sascha, tinyobj)
(maybe) make TODOs script in misc to locally generate TODOs.txt rather than TODOP bot
cleanup compile warnings
investigate program closing slowly
add Qol (quality of life) tag to TODOP
get mouse scrolling input to work
look into integrating TODOP with Discord

Render TODOs
------------
add instancing [https://learnopengl.com/Advanced-OpenGL/Instancing]
add vertex editing
add texture transparency support
add 2D shader and interface functions
add lighting and shadows
add RenderSettings loading and usage
check those vulkan-tutorial links for the suggestions
avoid having 3 copies of a mesh (model, meshVK, vulkan)
add buffer pre-allocation and arenas for vertices/indices/textures/etc
multi-threaded command buffers, shader loading, image loading
  convert prints to go thru the in-game console rather than other console
find a way to forward declare vulkan stuff and move the include to the cpp
(maybe) remove renderer polymorphism and replace it with defines that are checked on program start
(maybe) add specific shader reloading rather than all of them
add support for empty scene
add orthographic pipeline for side views etc.

Physics/Atmos TODOs
-------------
redo main physics loop
add physics collision sweeping
add physics based collision resolution for remaining collider primitives
add physics interaction functions

UI TODOs
--------
Restore the spawning/inspector menus
2D shader
add a UI popup when reloading shaders
add UI color palettes for easy color changing
renaming entities from entity list

Math NOTEs: Row-Major matrices, Left-Handed coordinate system (clockwise rotation when looking down axis)
Math TODOs
----------
add quaternions and converions between them and other linear algebra primitives
finish AxisAngleRotationMatrix function
review the projection functions for correctness
(maybe) move conversion functions to thier own file
(maybe) move transform/affine matrix functions to their own file
replace glm :)

*/

#include "core.h"
#include "EntityAdmin.h"

struct DeshiEngine {
	EntityAdmin entityAdmin;
	RenderAPI renderAPI;
	Renderer* renderer;
	Input input;
	Window window;
	deshiImGui* imgui;
	Time time;
	Console console;
	
	//TODO(delle,Fs) setup loading a config file to a config struct
	//TODO(delle,Re) change render API to a define rather than variable so it doesnt have to be a pointer
	void LoadConfig() {
		//render api
		renderAPI = RenderAPI::VULKAN;
		switch (renderAPI) {
			case(RenderAPI::VULKAN):default: {
				renderer = new Renderer_Vulkan;
				imgui = new vkImGui;
			}break;
		}
	}
	
	void Start() {
		//init
		
		//enforce deshi file system
		deshi::enforceDirectories();
		
		//start console
		

		LoadConfig();
		time.Init(300);
		window.Init(&input, 1280, 720); //inits input as well
		console.Init(&time, &input, &window, &entityAdmin);
		renderer->time = &time;
		renderer->Init(&window, imgui, &console); //inits imgui as well
		
		
		
		//start entity admin
		entityAdmin.Init(&input, &window, &time, renderer, &console, renderer->scene);
		
		//start main loop
		while (!glfwWindowShouldClose(window.window) && !window.closeWindow) {
			glfwPollEvents();
			Update();
		}
		
		//cleanup
		imgui->Cleanup(); delete imgui;
		renderer->Cleanup(); delete renderer;
		window.Cleanup();
		console.CleanUp();
	}
	
	bool Update() {
		time.Update();
		window.Update();
		input.Update();
		imgui->NewFrame();            //place imgui calls after this
		
		entityAdmin.Update();
		console.Update();      //not sure where we want this
		renderer->Render();           //place imgui calls before this
		renderer->Present();
		//entityAdmin.PostRenderUpdate();
		return true;
	}
};

int main() {
	DeshiEngine engine;
	engine.Start();
	
	int debug_breakpoint = 0;
}