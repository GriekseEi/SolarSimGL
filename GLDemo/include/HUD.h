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

//each loaded character must contain a texture, its size, the distance between other characters, and its bearing
struct Character {
	GLuint textureID;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint offset;
};

class HUD {
public:
	HUD(const char * fontPath);

	~HUD() {
		glDeleteVertexArrays(1, &hud_VAO);
		glDeleteBuffers(1, &hud_VBO);
	}

	void RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

private:
	//the data for each character is mapped here to the actual written character they represent
	std::map<GLchar, Character> Characters;
	GLuint hud_VAO, hud_VBO;
};

#endif // !HUD_H
