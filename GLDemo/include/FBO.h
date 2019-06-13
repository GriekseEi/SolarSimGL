#ifndef FBO_H
#define FBO_H

#include <glad/glad.h>
#include <shader_m.h>

class FBO {
public:
	FBO();
	~FBO();

	void init(GLuint windowWidth, GLuint windowHeight);
	void bindForWriting(GLenum cubeFace);
	void bindForReading(GLenum textureUnit);
	void drawTextureQuad(Shader& shader);
	void enable();

private:
	GLuint m_FBO;
	GLuint m_shadowMap;
	GLuint m_depth;
	GLuint m_scrVAO;
	GLuint m_scrVBO;
	GLuint m_TCB;
	GLuint m_RBO;
};

#endif