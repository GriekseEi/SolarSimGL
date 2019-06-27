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

	//set up transformation matrices
	planetRot = glm::mat4(1.0f);
	planetTrans = glm::mat4(1.0f);
	planetScale = glm::mat4(1.0f);
	planetScale = glm::scale(planetScale, glm::vec3(size));
}

//Draws the planetoid into space using the given shader and the point of origin it needs to orbit around
void Planetoid::Draw(const Shader& shader, float& deltaTime, glm::vec3& origin, bool turning) {

	if (!light) { //planetoid is not affected by any lighting (used exclusively for the sun)
		planetRot = glm::rotate(planetRot, glm::radians(rotationSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		shader.setBool("isSun", true);
	} else {
		if (turning) { //Should the planetoid orbit around its parent?
			/**
			To orbit the planetoid around its origin, the planetoid is first moved toward the origin,
			is rotated around the Y-axis with the orbitSpeed property, and then moved back by the distance of the
			given radius, so the distance between the planetoid center and the origin is always equal to the radius.
			**/
			planetTrans[3] = glm::vec4(origin, 1.0f);
			planetTrans = glm::rotate(planetTrans, glm::radians(orbitSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
			planetTrans = glm::translate(planetTrans, glm::vec3(radius, 0, 0));
			//Rotate the planetoid around its own center
			planetRot = glm::rotate(planetRot, glm::radians(rotationSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		} else {
			planetRot = glm::rotate(planetRot, glm::radians(rotationSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		//set the position to the third vec4 in the planetTrans mat4 where the XYZ position is located (and the fourth variable is always 1.0)
		position = glm::vec3(planetTrans[3]);

		shader.setBool("isSun", false);
	}

	//pass the model matrix to the shader
	shader.setMat4("model", (planetTrans * planetRot * planetScale));
	//call the draw command of the model with the textures of this Planetoid
	base->Draw(shader, textures);

	//draw all of the children Planetoids with the origin position of this Planetoid
	for (Planetoid* planet : children) {
		planet->Draw(shader, deltaTime, position, turning);
	}
}

void Planetoid::addPlanetoid(Planetoid* planet) {
	children.push_back(planet);
}