#include <glad/glad.h>

#include <planetoid.h>

Planetoid::Planetoid(Model* model, const glm::vec3& startingPos, const vector<Texture>* textures, float radius, float size, float orbitSpeed, float rotationSpeed, bool light) {
	//set up all properties of the planetoid
	this->base = model;
	this->textures = textures;
	this->orbitSpeed = orbitSpeed;
	this->rotationSpeed = rotationSpeed;
	this->radius = radius;
	this->light = light;
	this->size = size;

	planetRot = glm::mat4(1.0f);
	planetTrans = glm::mat4(1.0f);
	planetScale = glm::mat4(1.0f);
	planetScale = glm::scale(planetScale, glm::vec3(size));
}

void Planetoid::Draw(const Shader& shader, float& deltaTime, glm::vec3& origin, bool turning) {
	if (!light) {
		planetRot = glm::rotate(planetRot, glm::radians(rotationSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		shader.setBool("isSun", true);
	}
	else {
		if (turning) {
			planetTrans[3] = glm::vec4(origin, 1.0f);
			planetTrans = glm::rotate(planetTrans, glm::radians(orbitSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
			planetTrans = glm::translate(planetTrans, glm::vec3(radius, 0, 0));
			planetRot = glm::rotate(planetRot, glm::radians(rotationSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else {
			planetRot = glm::rotate(planetRot, glm::radians(rotationSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		position = glm::vec3(planetTrans[3]);

		shader.setBool("isSun", false);
	}
	shader.setMat4("model", (planetTrans * planetRot * planetScale));
	base->Draw(shader, textures);

	for (Planetoid* planet : children) {
		planet->Draw(shader, deltaTime, position, turning);
	}
}

void Planetoid::addPlanetoid(Planetoid* planet) {
	children.push_back(planet);
}