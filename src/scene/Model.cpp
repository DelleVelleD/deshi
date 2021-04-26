#include "Model.h"
#include "../core/assets.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../external/tinyobjloader/tiny_obj_loader.h"

#include <unordered_map>

/////////////////
//// Texture ////
/////////////////

Texture::Texture(const char* filename, TextureTypes type) {
	cpystr(this->filename, filename, 63);
	this->type = type;
}

////////////////
//// Vertex ////
////////////////

Vertex::Vertex(Vector3 pos, Vector2 uv, Vector3 color, Vector3 normal) {
	this->pos = pos; this->color = color; this->uv = uv; this->normal = normal;
}

///////////////
//// Batch ////
///////////////

Batch::Batch(const char* name, std::vector<Vertex> vertexArray, std::vector<u32> indexArray, std::vector<Texture> textureArray, Shader shader, ShaderFlags shaderFlags) {
	cpystr(this->name, name, 63);
	this->shader = shader; this->shaderFlags = shaderFlags;
	this->vertexArray = vertexArray;   this->vertexCount = vertexArray.size();
	this->indexArray = indexArray;     this->indexCount = indexArray.size();
	this->textureArray = textureArray; this->textureCount = textureArray.size();
}

void Batch::SetName(const char* name){
	cpystr(this->name, name, 63);
}

//////////////
//// Mesh ////
//////////////

Mesh::Mesh(const char* name, std::vector<Batch> batchArray) {
	cpystr(this->name, name, 63);
	this->batchArray = batchArray; this->batchCount = batchArray.size();
	for (Batch& batch : batchArray) {
		this->vertexCount += batch.vertexCount;
		this->indexCount += batch.indexCount;
		this->textureCount += batch.textureCount;
	}
}

void Mesh::SetName(const char* name){
	cpystr(this->name, name, 63);
}

inline Vector3 VertToVec3(const Vertex& v){
	return Vector3(v.pos.x, v.pos.y, v.pos.z);
}

std::vector<Triangle*> FindTriangleNeighbors(Mesh* m) {
	std::vector<Triangle*> triangles;

	//gather all triangles out of batch arrays
	for (auto& b : m->batchArray) {
		for (int i = 0; i < b.indexArray.size(); i += 3) {
			Triangle* t = new Triangle(); 
			t->p[0] = VertToVec3(b.vertexArray[b.indexArray[i]]);
			t->p[1] = VertToVec3(b.vertexArray[b.indexArray[i + 1]]);
			t->p[2] = VertToVec3(b.vertexArray[b.indexArray[i + 2]]);
			triangles.push_back(t);
		}
	}

	auto eqtoany = [](std::vector<Vector3> v, Vector3 t) {
		for (Vector3 a : v) if (t == a) return true;
		return false;
	};

	//find and mark neighbors
	Triangle* ti;
	Triangle* to;
	for (int i = 0; i < triangles.size(); i++) {
		ti = triangles[i];

		PRINTLN("test iterate~~~~ " << i);

		if (!(ti->nbr[0] && ti->nbr[1] && ti->nbr[2])) {
			std::vector<Vector3> tip{ ti->p[0], ti->p[1], ti->p[2] };

			for (int o = 0; o < triangles.size(); o++) {
				if (ti->nbr[0] && ti->nbr[1] && ti->nbr[2]) break;
				if (o != i) {
					to = triangles[o];
					if (to != ti->nbr[0] && to != ti->nbr[1] && to != ti->nbr[2]) {
						if (eqtoany(tip, to->p[0]) && eqtoany(tip, to->p[1]) ||
							eqtoany(tip, to->p[1]) && eqtoany(tip, to->p[2]) ||
							eqtoany(tip, to->p[2]) && eqtoany(tip, to->p[0])) {

							auto setnbr = [&](int index) {
								ti->nbr[index] = to;
								if (!to->nbr[0]) to->nbr[0] = ti;
								else if (!to->nbr[1]) to->nbr[1] = ti;
								else if (!to->nbr[2]) to->nbr[2] = ti;
								else ASSERT(false, "Triangle found too many neighbors.");
							};

							if      (!ti->nbr[0]) setnbr(0);
							else if (!ti->nbr[1]) setnbr(1);
							else if (!ti->nbr[2]) setnbr(2);

							int breakhere = 0;
						}
					}
				}
			}
		}
	}

	//temp debug assert
	if (triangles.size() > 4) {
		for (Triangle* t : triangles) {
			ASSERT(t->nbr[0] && t->nbr[1] && t->nbr[2], "triangle without 3 neighbors found");
		}
	}

	//m->triangles = triangles;

	return triangles;

}

//https://github.com/tinyobjloader/tinyobjloader
Mesh Mesh::CreateMeshFromOBJ(std::string filename){
	Mesh mesh; mesh.SetName(filename.c_str());
	int totalVertexCount = 0;
	int totalIndexCount = 0;
	int totalTextureCount = 0;
	
	//setup tinyobj
	tinyobj::ObjReaderConfig reader_config;
	reader_config.triangulate = true;
	reader_config.vertex_color = true;
	reader_config.mtl_search_path = deshi::dirModels(); // Path to material files
	tinyobj::ObjReader reader;
	if (!reader.ParseFromFile(deshi::dirModels() + filename, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		ASSERT(false, "failed to read OBJ file");
	}
	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}
	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();
	ASSERT(shapes[0].mesh.num_face_vertices[0] == 3, "OBJ must be triangulated");
	
	//check which features it has
	bool hasMaterials = materials.size() > 0;
	bool hasNormals = attrib.normals.size() > 0;
	bool hasUVs = attrib.texcoords.size() > 0;
	bool hasColors = attrib.colors.size() > 0;
	
	//fill batches
	mesh.batchArray.reserve(shapes.size());
	for (auto& shape : shapes) {
		Batch batch; batch.SetName(shape.name.c_str());
		
		//fill batch texture array
		if(hasMaterials && shape.mesh.material_ids.size() > 0){
			const tinyobj::material_t* mat = &materials[shape.mesh.material_ids[0]];
			if(mat->diffuse_texname.length() > 0){
				if(mat->diffuse_texopt.type == 0){
					Texture tex(mat->diffuse_texname.substr(mat->diffuse_texname.find_last_of('\\')+1).c_str(), TEXTURE_ALBEDO);
					batch.textureArray.push_back(tex);
				}
			}
			if(mat->specular_texname.length() > 0){
				if(mat->specular_texopt.type == 0){
					Texture tex(mat->specular_texname.substr(mat->specular_texname.find_last_of('\\')+1).c_str(), TEXTURE_SPECULAR);
					batch.textureArray.push_back(tex);
				}
			}
			if(mat->bump_texname.length() > 0){
				if(mat->bump_texopt.type == 0){
					Texture tex(mat->bump_texname.substr(mat->bump_texname.find_last_of('\\')+1).c_str(), TEXTURE_NORMAL);
					batch.textureArray.push_back(tex);
				}
			}
			if(mat->ambient_texname.length() > 0){
				if(mat->ambient_texopt.type == 0){
					Texture tex(mat->ambient_texname.substr(mat->ambient_texname.find_last_of('\\')+1).c_str(), TEXTURE_LIGHT);
					batch.textureArray.push_back(tex);
				}
			}
		}
		batch.textureCount = batch.textureArray.size();
		totalTextureCount += batch.textureCount;
		
		std::unordered_map<Vertex, u32> uniqueVertices{};

		//fill batch vertex and index arrays
		size_t faceCount = shape.mesh.num_face_vertices.size();
		batch.vertexArray.reserve(faceCount/3);
		batch.indexArray.reserve(shape.mesh.indices.size());
		for(auto& idx : shape.mesh.indices) { //loop over indices
			Vertex vertex;
			vertex.pos.x = attrib.vertices[3 * idx.vertex_index + 0];
			vertex.pos.y = attrib.vertices[3 * idx.vertex_index + 1];
			vertex.pos.z = attrib.vertices[3 * idx.vertex_index + 2];
			if(hasNormals){
				vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
				vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
				vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
			}
			if(hasUVs){
				vertex.uv.x = attrib.texcoords[2 * idx.texcoord_index + 0];
				vertex.uv.y = attrib.texcoords[2 * idx.texcoord_index + 1];
			}
			if(hasColors){
				vertex.color.x = attrib.colors[3 * idx.vertex_index + 0];
				vertex.color.y = attrib.colors[3 * idx.vertex_index + 1];
				vertex.color.z = attrib.colors[3 * idx.vertex_index + 2];
			}
			
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = u32(batch.vertexArray.size());
				batch.vertexArray.push_back(vertex);
			}
			else {
				batch.vertexArray[uniqueVertices[vertex]].normal = (vertex.normal + batch.vertexArray[uniqueVertices[vertex]].normal).normalized();
			}
			batch.indexArray.push_back(uniqueVertices[vertex]);
		}
		batch.vertexCount = batch.vertexArray.size();
		totalVertexCount += batch.vertexCount;
		batch.indexCount = batch.indexArray.size();
		totalIndexCount += batch.indexCount;
		
		//TODO(delle,Re) parse different shader options here based on texture count
		batch.shader = Shader::FLAT;
		batch.shaderFlags = SHADER_FLAGS_NONE;
		mesh.batchArray.push_back(batch);
	}
	
	mesh.vertexCount = totalVertexCount;
	mesh.indexCount = totalIndexCount;
	mesh.textureCount = totalTextureCount;
	mesh.batchCount = mesh.batchArray.size();
	mesh.triangles = FindTriangleNeighbors(&mesh);
	return mesh;
}


Mesh Mesh::CreateBox(Vector3 halfDims, Color color) {
	Vector3 p = halfDims;
	Vector3 c = Vector3(color.r, color.g, color.b) / 255.f;
	std::vector<Vertex> vertices = {
		{Vector3(-p.x, p.y, p.z), Vector2(0.f, 0.f), c, Vector3::ZERO}, //-x, y, z	0
		{Vector3(-p.x,-p.y, p.z), Vector2(0.f, 0.f), c, Vector3::ZERO}, //-x,-y, z	1
		{Vector3(-p.x, p.y,-p.z), Vector2(0.f, 0.f), c, Vector3::ZERO}, //-x, y,-z	2
		{Vector3(-p.x,-p.y,-p.z), Vector2(0.f, 0.f), c, Vector3::ZERO}, //-x,-y,-z	3
		{Vector3( p.x, p.y, p.z), Vector2(0.f, 0.f), c, Vector3::ZERO}, // x, y, z	4
		{Vector3( p.x,-p.y, p.z), Vector2(0.f, 0.f), c, Vector3::ZERO}, // x,-y, z	5
		{Vector3( p.x, p.y,-p.z), Vector2(0.f, 0.f), c, Vector3::ZERO}, // x, y,-z	6
		{Vector3( p.x,-p.y,-p.z), Vector2(0.f, 0.f), c, Vector3::ZERO}  // x,-y,-z	7
	};
	std::vector<u32> indices = {
		4,2,0,    4,6,2,	//top face
		2,7,3,    2,6,7,	//-z face
		6,5,7,    6,4,5,	//right face
		1,7,5,    1,3,7,	//bottom face
		0,3,1,    0,2,3,	//left face
		4,1,5,    4,0,1,	//+z face
	};
	
	Batch batch("box_batch", vertices, indices, {});
	Mesh mesh = Mesh("default_box", { batch });
	mesh.vertexCount = 8;
	mesh.indexCount = 36;
	mesh.textureCount = 0;
	mesh.batchCount = 1;
	return mesh;
}

Mesh Mesh::CreatePlanarBox(Vector3 halfDims, Color color) {
	float x = halfDims.x; float y = halfDims.y; float z = halfDims.z;
	Vector2 tl = Vector2(0.f, 0.f);   Vector2 tr = Vector2(1.f, 0.f); 
	Vector2 bl = Vector2(0.f, 1.f);   Vector2 br = Vector2(1.f, 1.f); 
	Vector3 c = Vector3(color.r, color.g, color.b) / 255.f;
	std::vector<Vertex> vertices = {
		//top face
		{Vector3(-x, y, z), tl, c, Vector3::UP},      //0
		{Vector3( x, y, z), tr, c, Vector3::UP},      //1
		{Vector3( x, y,-z), br, c, Vector3::UP},      //2
		{Vector3(-x, y,-z), bl, c, Vector3::UP},      //3
		//back face
		{Vector3(-x, y,-z), tl, c, Vector3::BACK},    //4
		{Vector3( x, y,-z), tr, c, Vector3::BACK},    //5
		{Vector3( x,-y,-z), br, c, Vector3::BACK},    //6
		{Vector3(-x,-y,-z), bl, c, Vector3::BACK},    //7
		//right face
		{Vector3( x, y,-z), tl, c, Vector3::RIGHT},   //8
		{Vector3( x, y, z), tr, c, Vector3::RIGHT},   //9
		{Vector3( x,-y, z), br, c, Vector3::RIGHT},   //10
		{Vector3( x,-y,-z), bl, c, Vector3::RIGHT},   //11
		//bottom face
		{Vector3(-x,-y,-z), tl, c, Vector3::DOWN},    //12
		{Vector3( x,-y,-z), tr, c, Vector3::DOWN},    //13
		{Vector3( x,-y, z), br, c, Vector3::DOWN},    //14
		{Vector3(-x,-y, z), bl, c, Vector3::DOWN},    //15
		//front face
		{Vector3( x, y, z), tl, c, Vector3::FORWARD}, //16
		{Vector3(-x, y, z), tr, c, Vector3::FORWARD}, //17
		{Vector3(-x,-y, z), br, c, Vector3::FORWARD}, //18
		{Vector3( x,-y, z), bl, c, Vector3::FORWARD}, //19
		//left face
		{Vector3(-x, y, z), tl, c, Vector3::LEFT},    //20
		{Vector3(-x, y,-z), tr, c, Vector3::LEFT},    //21
		{Vector3(-x,-y,-z), br, c, Vector3::LEFT},    //22
		{Vector3(-x,-y, z), bl, c, Vector3::LEFT},    //23
	};
	std::vector<u32> indices = {
		0, 1, 2,  0, 2, 3,   //top face
		4, 5, 6,  4, 6, 7,   //back face
		8, 9, 10, 8, 10,11, //right face
		12,13,14, 12,14,15,//bottom face
		16,17,18, 16,18,19,//front face
		20,21,22, 20,22,23,//left face
	};
	
	Batch batch("planarbox_batch", vertices, indices, {});
	Mesh mesh = Mesh("default_planarbox", { batch });
	mesh.vertexCount = 24;
	mesh.indexCount = 36;
	mesh.textureCount = 0;
	mesh.batchCount = 1;
	return mesh;
}

//TODO(delle) make this a texture array and make each face its own batch for multi-textured boxes
Mesh Mesh::CreatePlanarBox(Vector3 halfDims, Texture texture) {
	float x = halfDims.x; float y = halfDims.y; float z = halfDims.z;
	Vector2 tl = Vector2(0.f, 0.f);   Vector2 tr = Vector2(1.f, 0.f); 
	Vector2 bl = Vector2(0.f, 1.f);   Vector2 br = Vector2(1.f, 1.f); 
	Vector3 c = Vector3(Color::WHITE.r, Color::WHITE.g, Color::WHITE.b) / 255.f;
	std::vector<Vertex> vertices = {
		//top face
		{Vector3(-x, y, z), tl, c, Vector3::UP},      //0
		{Vector3( x, y, z), tr, c, Vector3::UP},      //1
		{Vector3( x, y,-z), br, c, Vector3::UP},      //2
		{Vector3(-x, y,-z), bl, c, Vector3::UP},      //3
		//back face
		{Vector3(-x, y,-z), tl, c, Vector3::BACK},    //4
		{Vector3( x, y,-z), tr, c, Vector3::BACK},    //5
		{Vector3( x,-y,-z), br, c, Vector3::BACK},    //6
		{Vector3(-x,-y,-z), bl, c, Vector3::BACK},    //7
		//right face
		{Vector3( x, y,-z), tl, c, Vector3::RIGHT},   //8
		{Vector3( x, y, z), tr, c, Vector3::RIGHT},   //9
		{Vector3( x,-y, z), br, c, Vector3::RIGHT},   //10
		{Vector3( x,-y,-z), bl, c, Vector3::RIGHT},   //11
		//bottom face
		{Vector3(-x,-y,-z), tl, c, Vector3::DOWN},    //12
		{Vector3( x,-y,-z), tr, c, Vector3::DOWN},    //13
		{Vector3( x,-y, z), br, c, Vector3::DOWN},    //14
		{Vector3(-x,-y, z), bl, c, Vector3::DOWN},    //15
		//front face
		{Vector3( x, y, z), tl, c, Vector3::FORWARD}, //16
		{Vector3(-x, y, z), tr, c, Vector3::FORWARD}, //17
		{Vector3(-x,-y, z), br, c, Vector3::FORWARD}, //18
		{Vector3( x,-y, z), bl, c, Vector3::FORWARD}, //19
		//left face
		{Vector3(-x, y, z), tl, c, Vector3::LEFT},    //20
		{Vector3(-x, y,-z), tr, c, Vector3::LEFT},    //21
		{Vector3(-x,-y,-z), br, c, Vector3::LEFT},    //22
		{Vector3(-x,-y, z), bl, c, Vector3::LEFT},    //23
	};
	std::vector<u32> indices = {
		0, 1, 2,  0, 2, 3,  //top face
		4, 5, 6,  4, 6, 7,  //back face
		8, 9, 10, 8, 10,11, //right face
		12,13,14, 12,14,15, //bottom face
		16,17,18, 16,18,19, //front face
		20,21,22, 20,22,23, //left face
	};
	
	Batch batch("planarbox_batch", vertices, indices, { texture }, Shader::PBR);
	Mesh mesh = Mesh("textured_planarbox", { batch });
	mesh.vertexCount = 24;
	mesh.indexCount = 36;
	mesh.textureCount = 1;
	mesh.batchCount = 1;
	return mesh;
}

///////////////
//// Model ////
///////////////

Model::Model(Mesh mesh){
	this->mesh = mesh;
}