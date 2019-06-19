#ifndef HUD_H
#define HUD_H

#include <map>
#include <string>

#include <shader_m.h>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
	GLuint textureID;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint offset;
};

class HUD {
public:
	HUD(const char * fontPath) {
		init(fontPath);
	}

	~HUD() {
		glDeleteVertexArrays(1, &hud_VAO);
		glDeleteBuffers(1, &hud_VBO);
	}

	void RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

private:
	std::map<GLchar, Character> Characters;
	GLuint hud_VAO, hud_VBO;

	void init(const char * fontPath);
};

#endif // !HUD_H
