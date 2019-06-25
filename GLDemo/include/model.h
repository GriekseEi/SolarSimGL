#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <shader_m.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

//define all possible texture types
enum TexType { TEX_DIFFUSE, TEX_NORMAL, TEX_SPECULAR };

//every vertex of a model holds data for its position, normals, texture coordinates, and (bi)tangents
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

//every texture will contain the texture data in the id-variable and its type in the type-variable
struct Texture {
	GLuint id;
	TexType type;
};

class Model
{
public:
	//needs to be initialized with a filepath to the 3D model
	Model(std::string const &path);
	~Model();

	// draws the model
	void Draw(const Shader& shader, const std::vector<Texture>* textures);

private:
	GLuint VAO, VBO, EBO;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	void loadModel(std::string const &path);
	void processMesh(aiMesh *mesh, const aiScene *scene);
	void setupMesh();
};
#endif