#ifndef PLANETOID_H
#define PLANETOID_H

#include <glad/glad.h>

#include <model.h>

using namespace std;

class Planetoid {
public:
	glm::vec3 position;
	bool turning;

	Planetoid(Model* model, const glm::vec3& startingPos, const vector<Texture>* textures, float radius, float size, float orbitSpeed, float rotationSpeed, bool light);

	void Draw(const Shader& shader, float& deltaTime, glm::vec3& origin, bool turning);
	void addPlanetoid(Planetoid* planet);

private:
	float orbitSpeed, rotationSpeed, radius, size;
	glm::mat4 planetTrans;
	glm::mat4 planetRot;
	glm::mat4 planetScale;
	bool light;
	Model* base;
	const vector<Texture>* textures;
	vector<Planetoid*> children;
};
#endif