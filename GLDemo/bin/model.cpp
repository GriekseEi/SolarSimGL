#include <glad/glad.h>

#include <model.h>

Model::Model(std::string const &path) {
	loadModel(path);
}

Model::~Model() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Model::Draw(const Shader& shader, const std::vector<Texture>* textures) {
	// bind appropriate textures
	GLuint i = 0;
	for (i; i < textures->size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		std::string name;

		//get the shader uniform variable to set depending on the type of the current texture
		switch (textures->at(i).type) {
		case TEX_DIFFUSE:
			name = "material.diffuse";
			break;
		case TEX_NORMAL:
			name = "material.normal";
			break;
		case TEX_SPECULAR:
			name = "material.specular";
			break;
		}

		// now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(shader.ID, name.c_str()), i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, textures->at(i).id);
	}

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	//set current active texture back to default
	glActiveTexture(GL_TEXTURE0);
}

//imports a model file into an aiMesh object
void Model::loadModel(std::string const &path) {
	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace); //get the tangents of the model for normal mapping
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	aiMesh* mesh = scene->mMeshes[scene->mRootNode->mChildren[0]->mMeshes[0]];
	processMesh(mesh, scene);
}

//read all the data from the aiMesh object and write it to vectors where it can be later used to set up the vertex array objects and buffers
void Model::processMesh(aiMesh *mesh, const aiScene *scene) {
	// Walk through each of the mesh's vertices
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector; //assimp uses its own vector class which doesn't automatically translate to glm's vec3, so the data has to be transferred manually
		// positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		// texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		// tangent
		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		vertex.Tangent = vector;
		// bitangent
		vector.x = mesh->mBitangents[i].x;
		vector.y = mesh->mBitangents[i].y;
		vector.z = mesh->mBitangents[i].z;
		vertex.Bitangent = vector;
		vertices.push_back(vertex);
	}

	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	setupMesh();
}

//set up the vertex array objects and buffers for the model
void Model::setupMesh() {

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//The size in bytes for each vertex in the buffer is defined by the size of each member in the Vertex struct.
	//Each Vertex member is a matrix of floats, and each float is read as a fixed bytesize.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	//the indices vector consists of only GLuints, so only the size of a GLuint needs to be passed to determine the stride
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	//The byte offset for each property in the vertex is calculated with the offsetof() macro which automatically determines the amount of bytes after which
	//a given member begins in the given struct, allowing us to easily pass the offset for each vertex member per vertex attribute pointer.

	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	glBindVertexArray(0);
}