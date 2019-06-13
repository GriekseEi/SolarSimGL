#ifndef PLANETOID_H
#define PLANETOID_H

#include <glad/glad.h>

#include <model.h>

using namespace std;

enum PlanetType { P_SUN, P_PLANET };

class Planetoid {
public:
	Model* base;
	vector<Texture> textures;
	glm::mat4 instanceMatrix;
	glm::vec3 position;
	vector<Planetoid> children;
	bool turning;

	Planetoid(Model* model, glm::vec3& startingPos, vector<Texture>& textures, float radius, float size, float orbitSpeed, float rotationSpeed, PlanetType type) {
		this->base = model;
		this->textures = textures;
		this->orbitSpeed = orbitSpeed;
		this->rotationSpeed = rotationSpeed;
		this->radius = radius;
		this->type = type;

		position = glm::vec3(radius + startingPos.x, startingPos.y, startingPos.z);

		instanceMatrix = glm::mat4(1.0f);
		instanceMatrix = glm::scale(instanceMatrix, glm::vec3(size));
		instanceMatrix = glm::translate(instanceMatrix, position);
	}

	void Draw(Shader& shader, GLuint depthCubemap, float& deltaTime, glm::vec3& origin) {

		instanceMatrix = glm::translate(instanceMatrix, glm::vec3(-radius - origin.x, -origin.y, -origin.z));
		instanceMatrix = glm::rotate(instanceMatrix, glm::radians(orbitSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		instanceMatrix = glm::translate(instanceMatrix, glm::vec3(radius + origin.x, origin.y, origin.z));
		position = glm::vec3(instanceMatrix[3]);

		shader.setMat4("model", instanceMatrix);
		if (type == P_SUN) {
			shader.setBool("isSun", true);
		} else {
			shader.setBool("isSun", false);
		}
		base->Draw(shader, depthCubemap, textures);

		for (Planetoid planet : children) {
			planet.Draw(shader, depthCubemap, deltaTime, position);
		}

	}

private:
	float orbitSpeed, rotationSpeed, radius;
	PlanetType type;
};
#endif