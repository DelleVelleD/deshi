#pragma once
#include "../utils/defines.h"
#include "../utils/Debug.h"

#if defined(_MSC_VER)
#pragma comment(lib,"vulkan-1.lib")
#pragma comment(lib,"glfw3.lib")
#endif
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <boost/optional.hpp>

struct Window;
struct Mesh;
struct Texture;
struct Matrix4;
struct Triangle;
struct Vector3;
struct ImDrawData;
typedef uint8 stbi_uc;

enum struct RenderAPI{
	VULKAN
};

enum struct RenderShader {
	DEFAULT, TWOD, METAL, WIREFRAME
};

enum struct RenderAlphaMode{
	OPAQUE, BLEND//, CLIP, HASHED
};

struct Renderer{
	virtual void Init(Window* window) = 0;
	virtual void Render() = 0;
	virtual void Present() = 0;
	virtual void Cleanup() = 0;
	
	//2d interface
	virtual uint32 AddTriangle(Triangle* triangle) = 0;
	virtual void   RemoveTriangle(uint32 triangleID) = 0;
	virtual void   UpdateTriangleColor(uint32 triangleID, Color color) = 0;
	virtual void   UpdateTrianglePosition(uint32 triangleID, Vector3 position) = 0;
	virtual void   TranslateTriangle(uint32 triangleID, Vector3 translation) = 0;
	virtual std::vector<uint32> AddTriangles(std::vector<Triangle*> triangles) = 0;
	virtual void   RemoveTriangles(std::vector<uint32> triangleIDs) = 0;
	virtual void   UpdateTrianglesColor(std::vector<uint32> triangleIDs, Color color) = 0;
	virtual void   TranslateTriangles(std::vector<uint32> triangleIDs, Vector3 translation) = 0;
	
	//mesh interface
	virtual uint32 LoadMesh(Mesh* mesh) = 0;
	virtual void   UnloadMesh(uint32 meshID) = 0;
	virtual void   ApplyTextureToMesh(uint32 textureID, uint32 meshID) = 0;
	virtual void   RemoveTextureFromMesh(uint32 textureID, uint32 meshID) = 0;
	virtual void   UpdateMeshMatrix(uint32 meshID, Matrix4 matrix) = 0;
	
	//texture interface
	virtual uint32 LoadTexture(Texture* texure) = 0;
	virtual void   UnloadTexture(uint32 textureID) = 0;
	
	//scene interface
	virtual void UpdateViewMatrix(Matrix4 matrix) = 0;
	virtual void UpdatePerspectiveMatrix(Matrix4 matrix) = 0;
	
	//imgui interface
	virtual void RenderImguiDrawData(ImDrawData* data) = 0;
};

////////////////////////////////
//// vulkan support structs ////
////////////////////////////////
/*TODO(r,delle) update vulkan rendering
In real implementation: 
https://vulkan-tutorial.com/en/Vertex_buffers/Staging_buffer#page_Using-a-staging-buffer:~:text=You%20may
https://vulkan-tutorial.com/en/Vertex_buffers/Staging_buffer#page_Conclusion:~:text=It%20should
https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer#page_Using-an-index-buffer:~:text=The%20previous
https://vulkan-tutorial.com/en/Uniform_buffers/Descriptor_layout_and_buffer#page_Updating-uniform-data:~:text=Using%20a%20UBO
https://vulkan-tutorial.com/en/Texture_mapping/Combined_image_sampler#page_Updating-the-descriptors:~:text=determined.-,It%20is
https://vulkan-tutorial.com/en/Generating_Mipmaps#page_Generating-Mipmaps:~:text=Beware%20if%20you
https://vulkan-tutorial.com/en/Generating_Mipmaps#page_Linear-filtering-support:~:text=There%20are%20two
https://vulkan-tutorial.com/en/Multisampling#page_Conclusion:~:text=features%2C-,like
*/

//TODO(cr,delle) remove boost stuff and do it manually 
struct QueueFamilyIndices {
	boost::optional<uint32> graphicsFamily;
	boost::optional<uint32> presentFamily;
	bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct VertexVk{
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 color;
	glm::vec3 normal;
	
	static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	bool operator==(const VertexVk& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
	}
};

//pattern: OR unshifted and L-shifted, then R-shift the combo, 
//then OR that with L-shifted, then R-shift the combo and repeat
//until the last combo which is not R-shifted ((x^(y<<))>>)^(z<<)
namespace std {
	template<> struct hash<VertexVk> {
		size_t operator()(VertexVk const& vertex) const {
			return (((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1) ^ (hash<glm::vec3>()(vertex.color) << 1) >> 1) ^ (hash<glm::vec3>()(vertex.normal) << 1);
		}
	};
};

struct TextureVk {
	Texture* texture;
	uint32 textureID;
	int width, height, channels;
	stbi_uc* pixels;
	uint32 mipLevels;
	
	VkBuffer       stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	VkImage        image;
	VkDeviceMemory imageMemory;
	VkDeviceSize   imageSize;
	
	VkDescriptorImageInfo imageInfo;
};

//a primitive contains the information for one draw call
struct PrimitiveVk{
	uint32 indexOffset;
	uint32 indexCount;
	uint32 materialID;
};

struct MeshVk{
	Mesh* mesh;
	uint32 MeshID;
	glm::mat4 modelMatrix;
	std::vector<PrimitiveVk> primitives;
};

struct MaterialVk{
	uint32 materialID;
	uint32 albedoTextureID   = 0xFFFFFFFF;
	uint32 normalTextureID   = 0xFFFFFFFF;
	uint32 specularTextureID = 0xFFFFFFFF;
	uint32 lightTextureID    = 0xFFFFFFFF;
	
	float           alphaThreshold; //A pixel is rendered only if its alpha value is above this threshold
	RenderAlphaMode alphaMode; //Blend Mode for Transparent Faces
	VkDescriptorSet descriptorSet;
	//VkPipeline      pipeline;
	
};

struct SceneVk{
	std::vector<TextureVk>  textures;
	std::vector<MeshVk>     meshes;
	std::vector<MaterialVk> materials;
	
	struct{
		uint32         count = 0;
		VkBuffer       buffer;
		VkDeviceMemory bufferMemory;
		VkDeviceSize   bufferSize;
	} vertices;
	
	struct{
		uint32         count = 0;
		VkBuffer       buffer;
		VkDeviceMemory bufferMemory;
		VkDeviceSize   bufferSize;
	} indices;
	
	VkDescriptorSet  descriptorSet;
	
	inline VkDescriptorImageInfo getTextureDescriptorInfo(size_t index);
};

struct FrameVk{
	VkCommandPool   commandPool;
	VkCommandBuffer commandBuffer;
	VkFence         fence;
	VkImage         image;
	VkImageView     imageView;
	VkFramebuffer   framebuffer;
};

struct FrameSemaphoreVk{
	VkSemaphore imageAcquiredSemaphore;
	VkSemaphore renderCompleteSemaphore;
};

struct FramebufferAttachmentsVk {
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
};

//////////////////////////////
//// vulkan delcarations  ////
//////////////////////////////

struct Renderer_Vulkan : public Renderer{
	///////////////////////////////
	//// user config variables ////
	///////////////////////////////
	
	//TODO(r,delle) INSERT VIDEO SETTINGS HERE
	
	
	//////////////////////////////
	//// vulkan api variables ////
	//////////////////////////////
	VkAllocationCallbacks* allocator = 0;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	GLFWwindow* window;
	
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	QueueFamilyIndices physicalQueueFamilies;
	
	VkDevice device;
	VkQueue  graphicsQueue;
	VkQueue  presentQueue;
	
	VkDescriptorPool descriptorPool;
	VkPipelineCache  graphicsPipelineCache = VK_NULL_HANDLE;
	
	SceneVk scene;
	
	//swapchain specifics
	int32                    width = 0;
	int32                    height = 0;
	VkSwapchainKHR           swapchain = 0;
	VkSurfaceKHR             surface;
	SwapChainSupportDetails  supportDetails;
	VkPhysicalDeviceFeatures deviceFeatures;
	VkSurfaceFormatKHR       surfaceFormat;
	VkPresentModeKHR         presentMode;
	VkExtent2D               extent;
	VkRenderPass             renderPass = 0;
	//VkPipeline               pipeline = 0;
	uint32                   imageCount = 0;
	bool                     clearEnable = 0;
    VkClearValue*            clearValues = 0;
	uint32                   frameIndex = 0;
	uint32                   semaphoreIndex = 0;
	FrameVk*                 frames = 0;
	FrameSemaphoreVk*        frameSemaphores = 0;
	FramebufferAttachmentsVk attachments = {0};
	
	VkPipelineLayout pipelineLayout;
	VkPipelineCache  pipelineCache = VK_NULL_HANDLE;
	
	//uniform buffer for the shaders
	struct ShaderData{
		VkBuffer        uniformBuffer;
		VkDeviceMemory  uniformBufferMemory;
		
		struct Values{
			glm::mat4 view;     //camera view matrix
			glm::mat4 proj;     //camera projection matrix
			glm::vec4 lightPos; //main light pos
			glm::vec4 viewPos;  //camera pos
		} values;
	} shaderData;
	
	//descriptor set layouts for pipelines
	struct {
		VkDescriptorSetLayout matrices;
		VkDescriptorSetLayout textures;
	} descriptorSetLayouts;
	
	//pipelines for the different shaders
	struct {
		VkPipeline DEFAULT   = VK_NULL_HANDLE;
		VkPipeline TWOD      = VK_NULL_HANDLE;
		VkPipeline METAL     = VK_NULL_HANDLE;
		VkPipeline WIREFRAME = VK_NULL_HANDLE;
	} pipelines;
	
	//list of shader modules created (stored for cleanup)
	std::vector<VkShaderModule> shaderModules;
	
	int32 minImageCount = 0;
	bool framebufferResized = false;
	
	//////////////////////////
	//// render interface ////
	//////////////////////////
	
	//runs the vulkan functions necessary to start rendering
	virtual void Init(Window* window) override;
	
	//acquires the next image from vulkan, resets the command buffers, 
	//updates uploaded information, begins the command buffers, begins the render pass, 
	//runs the different shader draw methods, ends the render pass
	virtual void Render() override;
	
	//places the swapchain in the presentation queue, iterates the frame index
	virtual void Present() override;
	
	//cleans up memory created in Init
	virtual void Cleanup() override;
	
	//adds a triangle to the 2d shader's vertex and index buffers
	//returns the ID of the triangle
	virtual uint32 AddTriangle(Triangle* triangle) override;
	
	//removes the triangle with triangleID from the 2d shader's vertex buffer
	virtual void RemoveTriangle(uint32 triangleID) override;
	
	virtual void UpdateTriangleColor(uint32 triangleID, Color color) override;
	
	virtual void UpdateTrianglePosition(uint32 triangleID, Vector3 position) override;
	
	virtual void TranslateTriangle(uint32 triangleID, Vector3 translation) override;
	
	//adds an array of triangles to the 2d shader's vertex and index buffers
	//returns an array of the triangle's IDs
	virtual std::vector<uint32> AddTriangles(std::vector<Triangle*> triangles) override;
	
	//removes the triangles with triangleID from the 2d shader's vertex buffer
	virtual void RemoveTriangles(std::vector<uint32> triangleIDs) override;
	
	virtual void UpdateTrianglesColor(std::vector<uint32> triangleIDs, Color color) override;
	
	virtual void TranslateTriangles(std::vector<uint32> triangleIDs, Vector3 translation) override;
	
	//loads a mesh to the different shaders specified in its batches
	//returns the ID of the mesh
	virtual uint32 LoadMesh(Mesh* mesh) override;
	
	virtual void UnloadMesh(uint32 meshID) override;
	
	//loads a texture onto the GPU
	//returns the texture's id
	virtual uint32 LoadTexture(Texture* texure) override;
	
	//unloads a texture from the GPU
	//NOTE the previously used texture ID will not be used again
	virtual void UnloadTexture(uint32 textureID) override;
	
	//attempts to apply the texture to the mesh, 
	//replaces the previous texture of the same type
	//NOTE does not unload the texture from the GPU
	virtual void ApplyTextureToMesh(uint32 textureID, uint32 meshID) override;
	
	//removes the texture from the mesh
	//NOTE if no textures remain on the mesh, it will use the null texture
	virtual void RemoveTextureFromMesh(uint32 textureID, uint32 meshID) override;
	
	//updates a mesh's model matrix: translation, rotation, scale
	virtual void UpdateMeshMatrix(uint32 meshID, Matrix4 matrix) override;
	
	//updates the GPU camera's view matrix
	virtual void UpdateViewMatrix(Matrix4 matrix) override;
	
	//updates the GPU camera's perspective matrix
	virtual void UpdatePerspectiveMatrix(Matrix4 matrix) override;
	
	//TODO(r,delle) create render imgui functionality, ref: ImGui_ImplVulkan_RenderDrawData
	virtual void RenderImguiDrawData(ImDrawData* data) override;
	
	//////////////////////////
	//// vulkan functions ////
	//////////////////////////
	
	void CreateInstance();
	
	void SetupDebugMessenger();
	
	//https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface
	void CreateSurface();
	
	//https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
	void PickPhysicalDevice();
	
	//creates an interface between the actual GPU device and a virtual device for interaction
	//https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues
	void CreateLogicalDevice();
	
	//creates a pool of descriptors of different types to be sent to shaders
	//https://vulkan-tutorial.com/en/Uniform_buffers/Descriptor_pool_and_sets
	void CreateDescriptorPool();
	
	void CreateUniformBuffer();
	
	void UpdateUniformBuffer();
	
	//create descriptor set layouts and a push constant for shaders,
	//create pipeline layout, allocate and write to descriptor sets
	void CreateLayouts();
	
	void CreateOrResizeWindow(int w, int h);
	
	//destroy old swap chain and in-flight frames, create a new swap chain with desired dimensions
	void CreateWindowSwapChain(int w, int h);
	
	void CreatePipelines();
	
	void CreateWindowCommandBuffers();
	
	///////////////////////////
	//// utility functions ////
	///////////////////////////
	
	void DestroyFrame(FrameVk* frame);
	void DestroyFrameSemaphore(FrameSemaphoreVk* sema);
	int GetMinImageCountFromPresentMode(VkPresentModeKHR mode);
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
														VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	
	//checks whether the graphics card supports swapchains
	bool isDeviceSuitable(VkPhysicalDevice device);
	
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	
	//https://vulkan-tutorial.com/en/Multisampling
	VkSampleCountFlagBits getMaxUsableSampleCount();
	
	//this controls color formats
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	
	//this controls vsync/triple buffering
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	
	//returns the drawable dimensions of the window
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	
	//creates an image view specifying how to use an image
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32 mipLevels);
	
	//searches the device a format matching the arguments
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	
	//tries to find a viable depth format
	VkFormat findDepthFormat();
	
	//creates a pipeline shader stage from the shader bytecode
	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage) ;
	
	//finds which memory types the graphics card offers
	uint32 findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties);
	
	//creates and binds a vulkan image to the GPU
	void createImage(uint32 width, uint32 height, uint32 mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, 
					 VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	
	//creates a buffer of defined usage and size on the device
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	
	//returns a command buffer that will only execute once
	VkCommandBuffer beginSingleTimeCommands();
	
	//ends a command buffer and frees that buffers memory
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	
	//converts a VkImage from one layout to another using an image memory barrier
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32 mipLevels);
	
	//uses commands to copy a buffer to an image
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height);
	
	//scans an image for max possible mipmaps and generates them
	//https://vulkan-tutorial.com/en/Generating_Mipmaps
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32 texWidth, int32 texHeight, uint32 mipLevels);
	
	//copies a buffer, we use this to copy from CPU to GPU
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	
	
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
