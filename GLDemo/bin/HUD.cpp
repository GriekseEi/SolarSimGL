#include <HUD.h>

//load and initialize the font for the HUD
HUD::HUD(const char * fontPath) {
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		std::cout << "Could not init FreeType library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, fontPath, 0, &face))
		std::cout << "Failed to load font" << std::endl;

	FT_Set_Pixel_Sizes(face, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (GLubyte c = 0; c < 128; c++) { //just load the first 128 glyphs of the font which will contain all numbers and letters and most-used symbols
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "Failed to load glyph" << std::endl;
			continue;
		}

		//generate a texture for each glyph in the font
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//store Character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};

		//pair the texture data of the font glyphs with the byte value of the actual character they represent
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}

	//clear resources after we're done loading the font
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	//set up the VAO and VBO and set it up to expect the font data
	glGenVertexArrays(1, &hud_VAO);
	glGenBuffers(1, &hud_VBO);
	glBindVertexArray(hud_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, hud_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//render the given string at position x,y with the given color and scale
void HUD::RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
	shader.use();

	//set the text color in the shader
	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(hud_VAO);

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		//for each character in the string, grab the respective Character struct that belongs to the character
		Character ch = Characters[*c];

		//calculate the actual position and size of the character
		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;

		//update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		//render glypth texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		//update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, hud_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//render the quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//increase the x-value by the offset of the current glyph so the next glyph is drawn correctly next to the current glyph
		x += (ch.offset >> 6) * scale; //the offset is 1/64th value of a pixel, so bitshift it by 6 (2^6 = 64) so we can get a value in pixels
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}