#ifndef FBO_H
#define FBO_H

#include <glad/glad.h>
#include <shader_m.h>

class FBO {
public:
	FBO(const GLuint& windowWidth, const GLuint& windowHeight, glm::vec3& lightPos);
	~FBO();

	void drawTextureQuad(Shader& shader);
	void enable();

private:
	GLuint m_FBO;
	GLuint m_scrVAO;
	GLuint m_scrVBO;
	GLuint m_TCB;
	GLuint m_RBO;

	float quadVertices[24] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions   // texCoords
	-1.0f,  1.0f,   0.0f, 1.0f,
	-1.0f, -1.0f,   0.0f, 0.0f,
	 1.0f, -1.0f,   1.0f, 0.0f,

	-1.0f,  1.0f,   0.0f, 1.0f,
	 1.0f, -1.0f,   1.0f, 0.0f,
	 1.0f,  1.0f,   1.0f, 1.0f
	};
};

#endif