#pragma once
#ifndef DESHI_MODEL_H
#define DESHI_MODEL_H

#include "../utils/defines.h"
#include "../math/Matrix.h"
#include "../math/Vector.h"
#include "Armature.h"

#include <vector>

enum TextureTypeBits : u32 { 
	TEXTURE_ALBEDO   = 0, 
	TEXTURE_NORMAL   = 1, 
	TEXTURE_LIGHT    = 2, 
	TEXTURE_SPECULAR = 4, 
	TEXTURE_CUBE     = 8, //not supported yet
	TEXTURE_SPHERE   = 16,//not supported yet
};
typedef u32 TextureTypes;

struct Texture {
	char filename[64];
	TextureTypes type;
	Texture() {}
	Texture(const char* filename, TextureTypes textureType = TEXTURE_ALBEDO);
};


enum ShaderFlagsBits : u32 {
	SHADER_FLAGS_NONE = 0,
};
typedef u32 ShaderFlags;

//TODO(sushi) find a nicer way to dynamically generate lists of shaders and other things like materials n such
enum Shader : u32 {
	//NOTE(delle) testing shaders should be removed on release
	FLAT, PHONG, TWOD, PBR, WIREFRAME, LAVALAMP, TESTING0, TESTING1
};

//is there maybe a better way of doing this than using 2 maps?
static std::map<u32, std::string> shadertostring = {
	{FLAT,      "FLAT"},
	{PHONG,     "PHONG"},
	{TWOD,      "TWOD"},
	{PBR,       "PBR"},
	{WIREFRAME, "WIREFRAME"},
	{LAVALAMP,  "LAVALAMP"},
	{TESTING0,  "TESTING0"},
	{TESTING1,  "TESTING1"}
};

//this is temporary i promise
//until i find a nicer way to dynamically get shader names n such
static std::map<int, std::string> shadertostringint = {
	{0,      "FLAT"},
	{1,     "PHONG"},
	{2,      "TWOD"},
	{3,       "PBR"},
	{4, "WIREFRAME"},
	{5,  "LAVALAMP"},
	{6,  "TESTING0"},
	{7,  "TESTING1"}
};

static std::map<std::string, Shader> stringtoshader = {
	{"FLAT",      FLAT},
	{"PHONG",     PHONG},
	{"TWOD",      TWOD},
	{"PBR",       PBR},
	{"WIREFRAME", WIREFRAME},
	{"LAVALAMP",  LAVALAMP},
	{"TESTING0",  TESTING0},
	{"TESTING1",  TESTING1}
};


struct Material{
	char name[64];
	u32 shader;
	u32 shaderFlags;
	std::vector<Texture> textureArray;
};

struct Vertex {
	Vector3 pos{};
	Vector2 uv;
	Vector3 color{1.f, 1.f, 1.f}; //between 0 and 1
	Vector3 normal{};
	//bone index		4tuple
	//bone weight		4tuple
	Vertex() {}
	Vertex(Vector3 pos, Vector2 uv, Vector3 color, Vector3 normal);
	
	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && uv == other.uv && normal == other.normal;
	}
};

//this is a hash function to compare vertices for a hash map
//pattern: OR unshifted and L-shifted, then R-shift the combo, 
//then OR that with L-shifted, then R-shift the combo and repeat
//until the last combo which is not R-shifted ((x^(y<<))>>)^(z<<)
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<Vector3>()(vertex.pos) ^ (hash<Vector2>()(vertex.uv) << 1)) >> 1) ^ (hash<Vector3>()(vertex.color) << 1);
		}
	};
};

//NOTE indices should be clockwise
struct Batch {
	char name[64];
	u32 vertexCount  = 0;
	u32 indexCount   = 0;
	u32 textureCount = 0;
	std::vector<Vertex>  vertexArray;
	std::vector<u32>     indexArray;
	std::vector<Texture> textureArray;
	
	u32      shader;
	ShaderFlags shaderFlags;
	
	Batch() {}
	Batch(const char* name, std::vector<Vertex> vertexArray, std::vector<u32> indexArray, std::vector<Texture> textureArray, Shader shader = Shader::FLAT, ShaderFlags shaderFlags = SHADER_FLAGS_NONE);
	
	void SetName(const char* name);
};

//primarily for caching triangles neighbors
//TODO(sushi, Op) maybe cache what edge a triangle shares with each of its neighbors too?
struct Triangle {
	Vector3 p[3];
	Triangle* nbr[3]{};

	Vector3 midpoint() {
		return Vector3(
			(p[0].x + p[1].x + p[2].x) / 3,
			(p[0].y + p[1].y + p[2].y) / 3,
			(p[0].z + p[1].z + p[2].z) / 3);
	}
};

struct Mesh {
	char name[64];
	
	u32 vertexCount  = 0;
	u32 indexCount   = 0;
	u32 textureCount = 0;
	
	u32 batchCount = 0;
	std::vector<Batch> batchArray;
	std::vector<Triangle*> triangles;

	Mesh() {}
	Mesh(const char* name, std::vector<Batch> batchArray);
	
	void SetName(const char* name);
	
	//filename: filename and extension, name: loaded mesh name, transform: pos,rot,scale of mesh
	static Mesh CreateMeshFromOBJ(std::string filename);
	static Mesh CreateBox(Vector3 halfDims, Color color = Color::WHITE);
	static Mesh CreatePlanarBox(Vector3 halfDims, Color color = Color::WHITE);
	static Mesh CreatePlanarBox(Vector3 halfDims, Texture texture);
};

//NOTE a model should not change after loading
struct Model{
	Armature armature;
	Mesh mesh;
	
	Model() {}
	Model(Mesh mesh);
};

#endif //DESHI_MODEL_H