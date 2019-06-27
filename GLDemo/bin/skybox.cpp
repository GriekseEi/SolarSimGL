#include <glad/glad.h>
#include <stb_image.h>

#include <skybox.h>
#include <shader_m.h>

/*
The skybox is a cubemap texture which is wrapped along the insides of the world space to give the world a starry background.
A cubemap itself consists of six individual textures which are passed to Skybox as a vector of string filepaths. 
Then a VAO and VBO are created using hard-coded vertex positions in skybox.h 
*/
Skybox::Skybox(const std::vector<std::string>& faces) {
	glGenTextures(1, &cubemapTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			//There are six possible cubemap orientations, which we can just loop through by incrementing the cubemap orientation enum for every loaded face
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//Set up a VAO and VBO using skyboxVertices in skybox.h
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

Skybox::~Skybox() {
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
}

/*
Since drawing the skybox as the background first and then drawing all other planetoids over it results in a lot of wasted fragments
which aren't visible because they're behind the planetoids, it would instead be more efficient if the planetoids were drawn first and
the skybox last so the skybox fragments are only drawn where there isn't a planetoid in front of it.
As the depth value of a cubemapped skybox will always be 1.0, the depth test function needs to be set to pass values that are less than or equal
to the current value in the depth buffer (see skybox_vs.glsl for more details)
*/
void Skybox::draw(Shader& skyboxShader, glm::mat4& view, glm::mat4& projection) {

	glDepthFunc(GL_LEQUAL);
	skyboxShader.use();

	/*
	By converting the view matrix into mat3 the translation component of the matrix is removed, so the skybox will never move and always appear
	to be a massive distance away from the viewer. However, the skybox will still be affected when the camera rotates or scales.
	*/
	skyboxShader.setMat4("view", glm::mat3(view));
	skyboxShader.setMat4("projection", projection);

	//draw the cubemap texture
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}