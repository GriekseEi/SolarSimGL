#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <IK/irrKlang.h>

#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <planetoid.h>
#include <FBO.h>
#include <HUD.h>

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

//pre-emptively set up function templates so GLFW can reference them
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
Texture loadTexture(const char *path);
vector<vector<Texture>> loadTextureAtlas(string path);
unsigned int loadCubemap(std::vector<std::string> faces);

//when setting up the window goes wrong
static void glfwError(int id, const char* description)
{
	OutputDebugString(description);
	std::cout << description << std::endl;
}

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

Camera camera(0.0f, 61.0f, 0.0f, 0, 1, 0, -88.9f, 180.6);
float lastX = (float)WINDOW_WIDTH / 2.0;
float lastY = (float)WINDOW_HEIGHT / 2.0;
bool firstMouse = true;

bool shadows = true;
bool shadowsPressed = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
	glfwSetErrorCallback(&glfwError);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "GLDemo", NULL, NULL);
	if (window == NULL) {
		std::cout << "Couldn't create window" << std::endl;
		OutputDebugStringW(L"Couldn't create window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(1); //enable vsync

	//capture mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		OutputDebugStringW(L"Failed to initialize GLAD");
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//compile shaders
	Shader sphereShader("./bin/shaders/sphere_vs.glsl", "./bin/shaders/sphere_fs.glsl");
	Shader depthShader("./bin/shaders/depthshader_vs.glsl", "./bin/shaders/depthshader_fs.glsl", "./bin/shaders/depthshader_gs.glsl");
	Shader skyboxShader("./bin/shaders/skybox_vs.glsl", "./bin/shaders/skybox_fs.glsl");
	Shader screenShader("./bin/shaders/screen_vs.glsl", "./bin/shaders/screen_fs.glsl");
	Shader hudShader("./bin/shaders/hud_vs.glsl", "./bin/shaders/hud_fs.glsl");

	Model base("./bin/models/newsphere.obj");
	Model saturn_ring("./bin/models/ring.obj");
	Model phobos_base("./bin/models/phobos.3DS");
	Model deimos_base("./bin/models/deimos.3ds");
	vector<vector<Texture>> textureAtlas = loadTextureAtlas("./bin/textures/planets/");

	glm::vec3 origin = glm::vec3(0);
	Planetoid sun = Planetoid(&base, origin, &textureAtlas[0], 0, 3.0f, 0.0f, 5.0f, P_SUN);
	Planetoid mercury = Planetoid(&base, sun.position, &textureAtlas[1], 20.0f, 0.3f, 10.0f, 90.0f, P_PLANET);
	Planetoid venus = Planetoid(&base, sun.position, &textureAtlas[2], 28.0f, 0.4f, 13.0f, 80.0f, P_PLANET);
	Planetoid earth = Planetoid(&base, sun.position, &textureAtlas[3], 25.0f, 0.7f, 15.0f, 50.0f, P_PLANET);
	Planetoid moon = Planetoid(&base, earth.position, &textureAtlas[4], 10.0f, 0.1f, 20.0f, 70.0f, P_PLANET);
	Planetoid mars = Planetoid(&base, sun.position, &textureAtlas[5], 40.0f, 0.6f, 23.0f, 50.0f, P_PLANET);
	Planetoid deimos = Planetoid(&deimos_base, mars.position, &textureAtlas[6], 10.0f, 0.04f, 30.0f, 60.0f, P_PLANET);
	Planetoid phobos = Planetoid(&phobos_base, mars.position, &textureAtlas[7], 15.0f, 0.03f, 24.0f, 35.0f, P_PLANET);
	Planetoid jupiter = Planetoid(&base, sun.position, &textureAtlas[8], 20.0f, 2.0f, 8.0f, 45.0f, P_PLANET);
	Planetoid saturn = Planetoid(&base, sun.position, &textureAtlas[9], 25.0f, 1.5f, 6.0f, 30.0f, P_PLANET);
	Planetoid saturnRing = Planetoid(&saturn_ring, saturn.position, &textureAtlas[10], 0.0f, 4.0f, 0.0f, 0.0f, P_PLANET);
	Planetoid uranus = Planetoid(&base, sun.position, &textureAtlas[11], 30.0f, 1.4f, 5.0f, 20.0f, P_PLANET);
	Planetoid neptune = Planetoid(&base, sun.position, &textureAtlas[13], 35.0f, 1.5f, 4.0f, 25.0f, P_PLANET);

	sun.addPlanetoid(&mercury);
	sun.addPlanetoid(&venus);
	sun.addPlanetoid(&earth);
	earth.addPlanetoid(&moon);
	sun.addPlanetoid(&mars);
	mars.addPlanetoid(&deimos);
	mars.addPlanetoid(&phobos);
	sun.addPlanetoid(&jupiter);
	sun.addPlanetoid(&saturn);
	saturn.addPlanetoid(&saturnRing);
	sun.addPlanetoid(&uranus);
	sun.addPlanetoid(&neptune);
	
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<std::string> faces
	{
		"./bin/textures/skybox/bkg1_right.png",
		"./bin/textures/skybox/bkg1_left.png",
		"./bin/textures/skybox/bkg1_top.png",
		"./bin/textures/skybox/bkg1_bot.png",
		"./bin/textures/skybox/bkg1_front.png",
		"./bin/textures/skybox/bkg1_back.png"
	};
	GLuint cubemapTex = loadCubemap(faces);

	skyboxShader.setInt("skybox", 0);

	FBO frameBuffer;
	frameBuffer.init(WINDOW_WIDTH, WINDOW_HEIGHT, sun.position);

	irrklang::ISoundEngine *SoundEngine = irrklang::createIrrKlangDevice();
	SoundEngine->play2D("./bin/audio/foregonedestruction.mp3", GL_TRUE);
	
	glm::mat4 hud_projection = glm::ortho(0.0f, static_cast<GLfloat>(WINDOW_WIDTH), 0.0f, static_cast<GLfloat>(WINDOW_HEIGHT));
	hudShader.use();
	glUniformMatrix4fv(glGetUniformLocation(hudShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(hud_projection));
	HUD hud("./bin/fonts/arial.ttf");

	float lastTime = glfwGetTime();
	int frameCount = 0;
	int oldFrameCount = 1;

	//main render loop
	while (!glfwWindowShouldClose(window)) {

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		frameCount++;

		processInput(window);

		frameBuffer.enable();
		//frameBuffer.renderToDepthCubemap(depthShader, sun.position);
		//sun.Draw(depthShader, frameBuffer.m_depth, deltaTime, origin);

		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
		sphereShader.use();
		
		//set lighting properties
		sphereShader.setVec3("light.position", glm::vec3(1));
		sphereShader.setVec3("viewPos", camera.Position);

		sphereShader.setVec3("light.ambient", 0.03f, 0.03f, 0.03f);
		sphereShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
		sphereShader.setFloat("light.constant", 1.0f);
		sphereShader.setFloat("light.linear", 0.0056f);
		sphereShader.setFloat("light.quadratic", 0.000014f);
		sphereShader.setFloat("material.shininess", 8.0f);
		sphereShader.setInt("shadows", shadows);
		sphereShader.setFloat("far_plane", frameBuffer.far_plane);

		sphereShader.setMat4("projection", projection);
		sphereShader.setMat4("view", view);

		//draw the Sun and all its children
		sun.Draw(sphereShader, 0, deltaTime, origin);

		//skybox rendering
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		if (currentFrame - lastTime >= 1.0) {
			oldFrameCount = frameCount;
			frameCount = 0;
			lastTime += 1.0;
		}
		hud.RenderText(hudShader,
			std::to_string(oldFrameCount) + " FPS, " + std::to_string(1000.0 / double(oldFrameCount)) + " ms/frame",
			5.0f, 5.0f, 0.25f, glm::vec3(0.5, 0.8, 0.2f)
		);

		frameBuffer.drawTextureQuad(screenShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//deallocate resources on program closure
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

//if window gets resized, resize OpenGL viewport as well with new dimensions
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

//Check if the relevant keys have been pressed/released
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		camera.LookAt(glm::vec3(0, 0, 0));

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsPressed) {
		shadows = !shadows;
		shadowsPressed = true;
	} 
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
		shadowsPressed = false;
}

Texture loadTexture(char const * path)
{
	Texture texture;
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load from: " << path << std::endl;
		OutputDebugString(path);
		stbi_image_free(data);
	}

	texture.id = textureID;
	return texture;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			OutputDebugStringW(L"Failed to load cubemap");
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

vector<vector<Texture>> loadTextureAtlas(string path) {
	vector<vector<Texture>> textureAtlas;

	for (const auto & dir : fs::directory_iterator(path)) {
		if (dir.is_directory()) {
			vector<Texture> textures;

			for (const auto & item : fs::directory_iterator(dir.path())) {
				string path = item.path().string();

				Texture texture;
				GLuint textureID;
				glGenTextures(1, &textureID);

				int width, height, nrComponents;
				unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
				if (data)
				{
					GLenum format;
					if (nrComponents == 1)
						format = GL_RED;
					else if (nrComponents == 3)
						format = GL_RGB;
					else if (nrComponents == 4)
						format = GL_RGBA;

					glBindTexture(GL_TEXTURE_2D, textureID);
					glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
					glGenerateMipmap(GL_TEXTURE_2D);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

					stbi_image_free(data);

					texture.id = textureID;

					int n = path.find("_") + 1;
					string type = path.substr(n, 1);

					if (type == "d") {
						texture.type = TEX_DIFFUSE;
					} else if (type == "n") {
						texture.type = TEX_NORMAL;
					} else if (type == "s") {
						texture.type = TEX_SPECULAR;
					} else {
						cout << "Failed to assign type to texture (" << path << ") of type: " << type << endl;
					}

					textures.push_back(texture);
				} else {
					std::cout << "Texture failed to load at path: " << path << std::endl;
					stbi_image_free(data);
				}
			}
			textureAtlas.push_back(textures);
		}
	}

	return textureAtlas;
}