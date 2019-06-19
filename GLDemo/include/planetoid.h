#ifndef PLANETOID_H
#define PLANETOID_H

#include <glad/glad.h>

#include <model.h>

using namespace std;

enum PlanetType { P_SUN, P_PLANET };

class Planetoid {
public:
	glm::vec3 position;
	bool turning;

	Planetoid(Model* model, const glm::vec3& startingPos, vector<Texture>* textures, float radius, float size, float orbitSpeed, float rotationSpeed, PlanetType type) {
		this->base = model;
		this->textures = textures;
		this->orbitSpeed = orbitSpeed;
		this->rotationSpeed = rotationSpeed;
		this->radius = radius;
		this->type = type;
		this->size = size;

		position = glm::vec3(radius + startingPos.x, startingPos.y, startingPos.z);

		instanceMatrix = glm::mat4(1.0f);
		instanceMatrix = glm::scale(instanceMatrix, glm::vec3(size));
		instanceMatrix = glm::translate(instanceMatrix, position);
		position = glm::vec3(instanceMatrix[3]);
	}

	void Draw(const Shader& shader, const GLuint& depthCubemap, float& deltaTime, glm::vec3& origin) {

		if (type == P_SUN) {
			instanceMatrix = glm::rotate(instanceMatrix, glm::radians(rotationSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
			shader.setBool("isSun", true);
		} else {
			instanceMatrix[3] = glm::vec4(origin, 1.0f);
			instanceMatrix = glm::rotate(instanceMatrix, glm::radians(orbitSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
			instanceMatrix = glm::translate(instanceMatrix, glm::vec3(radius, 0, 0));
			//instanceMatrix = glm::rotate(instanceMatrix, glm::radians(rotationSpeed * deltaTime), glm::vec3(0, 1, 0));
			position = glm::vec3(instanceMatrix[3]);
			shader.setBool("isSun", false);
		}
		shader.setMat4("model", instanceMatrix);
		base->Draw(shader, depthCubemap, textures);

		for (Planetoid* planet : children) {
			planet->Draw(shader, depthCubemap, deltaTime, position);
		}
	}

	void addPlanetoid(Planetoid* planet) {
		children.push_back(planet);
	}

private:
	float orbitSpeed, rotationSpeed, radius, size;
	glm::mat4 instanceMatrix;
	PlanetType type;
	vector<Planetoid*> children;
	Model* base;
	vector<Texture>* textures;
};
#endif