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
#include <skybox.h>

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
vector<vector<Texture>> loadTextureAtlas(string path);

//gets called when setting up the window goes wrong
static void glfwError(int id, const char* description)
{
	std::cout << description << std::endl;
}

//set window properties
const GLuint WINDOW_WIDTH = 1024;
const GLuint WINDOW_HEIGHT = 768;
const char * WINDOW_TITLE = "GLDemo";

//file paths for loading textures, fonts, music, etc.
const char * MUSIC_PATH = "./bin/audio/foregonedestruction.mp3";
const char * FONT_PATH = "./bin/fonts/arial.ttf";
const char * PLANET_TEXTURES_PATH = "./bin/textures/planets/";

const std::vector<std::string> SKYBOX_FACES
{
	"./bin/textures/skybox/bkg1_right.png",
	"./bin/textures/skybox/bkg1_left.png",
	"./bin/textures/skybox/bkg1_top.png",
	"./bin/textures/skybox/bkg1_bot.png",
	"./bin/textures/skybox/bkg1_front.png",
	"./bin/textures/skybox/bkg1_back.png"
};

//set up camera to be above the Sun
Camera camera(0.0f, 61.0f, 0.0f, 0, 1, 0, -88.9f, 180.6);
float lastX = (float)WINDOW_WIDTH / 2.0;
float lastY = (float)WINDOW_HEIGHT / 2.0;
bool firstMouse = true;

bool turning = true;
bool turnPressed = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
	//The GPU must support OpenGL 3.3+ to be able to run this application, else the program will automatically exit
	glfwSetErrorCallback(&glfwError);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (window == NULL) {
		std::cout << "Couldn't create window" << std::endl;
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
		return -1;
	}

	//Enable depth testing for skybox support and transparency blending for text rendering
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//load and compile shaders
	Shader sphereShader("./bin/shaders/sphere_vs.glsl", "./bin/shaders/sphere_fs.glsl");
	Shader skyboxShader("./bin/shaders/skybox_vs.glsl", "./bin/shaders/skybox_fs.glsl");
	Shader screenShader("./bin/shaders/screen_vs.glsl", "./bin/shaders/screen_fs.glsl");
	Shader hudShader("./bin/shaders/hud_vs.glsl", "./bin/shaders/hud_fs.glsl");

	//load all necessary textures and models
	Model base("./bin/models/newsphere.obj");
	Model saturn_ring("./bin/models/ring.obj");
	Model phobos_base("./bin/models/phobos.3DS");
	Model deimos_base("./bin/models/deimos.3ds");
	vector<vector<Texture>> textureAtlas = loadTextureAtlas(PLANET_TEXTURES_PATH);

	//initialize all planets in the solar system, assign the proper models, textures, and properties
	Planetoid sun = Planetoid(&base, glm::vec3(0), &textureAtlas[0], 0, 6.0f, 0.0f, 5.0f, false);
	Planetoid mercury = Planetoid(&base, sun.position, &textureAtlas[1], 10.0f, 0.3f, 10.0f, 30.0f, true);
	Planetoid venus = Planetoid(&base, sun.position, &textureAtlas[2], 16.0f, 0.7f, 13.0f, 40.0f, true);
	Planetoid earth = Planetoid(&base, sun.position, &textureAtlas[3], 25.0f, 0.8f, 10.0f, 40.0f, true);
	Planetoid moon = Planetoid(&base, earth.position, &textureAtlas[4], 1.3f, 0.05f, 20.0f, 50.0f, true);
	Planetoid mars = Planetoid(&base, sun.position, &textureAtlas[5], 35.0f, 0.7f, 15.0f, 30.0f, true);
	Planetoid deimos = Planetoid(&deimos_base, mars.position, &textureAtlas[6], 1.5f, 0.01f, 40.0f, 30.0f, true);
	Planetoid phobos = Planetoid(&phobos_base, mars.position, &textureAtlas[7], 2.0f, 0.01f, 50.0f, 35.0f, true);
	Planetoid jupiter = Planetoid(&base, sun.position, &textureAtlas[8], 50.0f, 2.0f, 8.0f, 25.0f, true);
	Planetoid saturn = Planetoid(&base, sun.position, &textureAtlas[9], 60.0f, 1.8f, 6.0f, 20.0f, true);
	Planetoid saturnRing = Planetoid(&saturn_ring, saturn.position, &textureAtlas[10], 0.0f, 4, 0.0f, 0.0f, true);
	Planetoid uranus = Planetoid(&base, sun.position, &textureAtlas[11], 68.0f, 1.9f, 5.0f, 20.0f, true);
	Planetoid neptune = Planetoid(&base, sun.position, &textureAtlas[12], 76.0f, 1.8f, 4.0f, 25.0f, true);

	//establish parent-child relationships between planetoids
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

	//load skybox
	Skybox skybox(SKYBOX_FACES);
	//load framebuffer
	FBO frameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT, sun.position);
	//load sound engine
	irrklang::ISoundEngine *SoundEngine = irrklang::createIrrKlangDevice();
	SoundEngine->play2D(MUSIC_PATH, GL_TRUE);
	//load HUD
	glm::mat4 hud_projection = glm::ortho(0.0f, static_cast<GLfloat>(WINDOW_WIDTH), 0.0f, static_cast<GLfloat>(WINDOW_HEIGHT));
	hudShader.use();
	glUniformMatrix4fv(glGetUniformLocation(hudShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(hud_projection));
	HUD hud(FONT_PATH);

	//set variables for calculating frames per second
	float lastTime = glfwGetTime();
	int frameCount = 0;
	int oldFrameCount = 1;

	//main render loop
	while (!glfwWindowShouldClose(window)) {

		/**deltaTime is the time interval between the current and the last frame. 
		Each calculation which is executed each frame is multiplied by deltaTime in order to prevent inconsistencies from happening
		when the frames per second dip in numbers (f.e. planets moving slower at a lower FPS)**/
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		frameCount++;

		//Check if any inputs are given
		processInput(window);

		//Set the framebuffer to read input
		frameBuffer.enable();
		//refresh the GPU color and depth buffers so they can be rewritten
		//glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get the view and projection matrix from the camera
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
		
		//set lighting properties
		sphereShader.use();
		sphereShader.setVec3("light.position", sun.position);
		sphereShader.setVec3("viewPos", camera.Position);

		sphereShader.setVec3("light.ambient", 0.03f, 0.03f, 0.03f);
		sphereShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
		sphereShader.setFloat("light.constant", 1.0f);
		sphereShader.setFloat("light.linear", 0.0056f);
		sphereShader.setFloat("light.quadratic", 0.000014f);
		sphereShader.setFloat("material.shininess", 100.0f);

		sphereShader.setMat4("projection", projection);
		sphereShader.setMat4("view", view);

		//draw the Sun and all its children
		sun.Draw(sphereShader, deltaTime, sun.position, turning);
		//draw skybox
		skybox.draw(skyboxShader, glm::mat4(glm::mat3(camera.GetViewMatrix())), projection);

		//Draw the FPS on the HUD every second
		if (currentFrame - lastTime >= 1.0) {
			oldFrameCount = frameCount;
			frameCount = 0;
			lastTime += 1.0;
		}
		hud.RenderText(hudShader,
			std::to_string(oldFrameCount) + " FPS, " + std::to_string(1000.0 / double(oldFrameCount)) + " ms/frame",
			5.0f, 5.0f, 0.25f, glm::vec3(0.5, 0.8, 0.2f)
		);

		//Have the framebuffer convert everything on screen into a texture that's drawn on a quad the size of the window
		frameBuffer.drawTextureQuad(screenShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//destroy the window when a closing request is sent
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

//if window gets resized, resize OpenGL viewport as well with new dimensions
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//calculate the change in mouse position per frame and pass it to the camera
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
	//Close the program when ESC is pressed
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//Camera controls with WASD
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	//disable/enable planetoids from orbiting around their parents
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !turnPressed) {
		turning = !turning;
		turnPressed = true;
	} 
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
		turnPressed = false;
}

/*
Loads all the planet textures into a vector of vectors containing planet textures.

Every planetoid has a vector of textures for diffuse, normal and specular maps, which are saved in their own folders.
The folder names all begin with a number representing the rank of each planetoid for how close it is to the sun (the Sun itself is 0, Mercury is 1, Earth is 3, the Moon is 4, etc.)
The vectors for each planetoid are also sorted by these ranks, so the textures for the Sun will be in textureAtlas[0], the Moons' textures in textureAtlas[4], etc.
*/
vector<vector<Texture>> loadTextureAtlas(std::string path) {
	vector<vector<Texture>> textureAtlas;

	for (const auto & dir : fs::directory_iterator(path)) {
		if (dir.is_directory()) {
			//for every subdirectory in path, create a new vector for textures
			vector<Texture> textures;

			//for every item in the subdirectory, generate a new texture
			for (const auto & item : fs::directory_iterator(dir.path())) {
				std::string path = item.path().string();

				Texture texture;
				GLuint textureID;
				glGenTextures(1, &textureID);

				//Images are loaded with the STB library and automatically have their attributes ascertained
				int width, height, nrComponents;
				unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
				if (data)
				{
					//check whether we're dealing with monochrome images, JPEGs, or PNGs
					GLenum format;
					if (nrComponents == 1)
						format = GL_RED;
					else if (nrComponents == 3)
						format = GL_RGB;
					else if (nrComponents == 4)
						format = GL_RGBA;

					//set texture properties, wrapping, and mipmapping
					glBindTexture(GL_TEXTURE_2D, textureID);
					glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
					glGenerateMipmap(GL_TEXTURE_2D);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

					stbi_image_free(data);

					texture.id = textureID;

					//every texture image filename ends in _d, _n or _s to signify it represents a diffuse, normal or specular map respectively
					//the correct enum representing the texture type is then set for each texture
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
			//add the new vector into the main texture atlas
			textureAtlas.push_back(textures);
		}
	}
	return textureAtlas;
}