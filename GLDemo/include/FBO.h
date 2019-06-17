#ifndef FBO_H
#define FBO_H

#include <glad/glad.h>
#include <shader_m.h>

#include <vector>

class FBO {
public:
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint m_depth;
	GLuint m_shadowMap;
	float far_plane;

	FBO();
	~FBO();

	void init(GLuint windowWidth, GLuint windowHeight, glm::vec3& lightPos);
	void renderToDepthCubemap(Shader& shader, glm::vec3& lightPos);
	void drawTextureQuad(Shader& shader);
	void enable();

private:
	GLuint m_FBO;
	GLuint m_scrVAO;
	GLuint m_scrVBO;
	GLuint m_TCB;
	GLuint m_RBO;
	std::vector<glm::mat4> shadowTransforms;
};

#endif