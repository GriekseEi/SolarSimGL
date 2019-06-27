#include <FBO.h>
#include <glad/glad.h> 
#include <string>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

FBO::FBO(const GLuint& windowWidth, const GLuint& windowHeight, glm::vec3& lightPos) {

	//create framebuffer
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	//set up array and buffer objects for the screen quad
	glGenVertexArrays(1, &m_scrVAO);
	glGenBuffers(1, &m_scrVBO);
	glBindVertexArray(m_scrVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_scrVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	//generate the texture to which the render output will be bound to
	glGenTextures(1, &m_TCB);
	glBindTexture(GL_TEXTURE_2D, m_TCB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TCB, 0);

	//create render-buffer object
	glGenRenderbuffers(1, &m_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

	//check if framebuffer setup was successful
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Primary framebuffer is not complete" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//clear the screen and draw the screen texture stored in the framebuffer
void FBO::drawTextureQuad(Shader& shader) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	shader.use();
	glBindVertexArray(m_scrVAO);
	glBindTexture(GL_TEXTURE_2D, m_TCB);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//set the framebuffer to be active and enable depth testing again
void FBO::enable() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glEnable(GL_DEPTH_TEST);
}

FBO::~FBO() {
	glDeleteVertexArrays(1, &m_scrVAO);
	glDeleteBuffers(1, &m_scrVBO);
}