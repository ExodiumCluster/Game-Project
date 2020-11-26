#pragma once

#include <GL/glew.h>
#include <GL/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <iostream>

#include "Shader.h"

class GameObject {
public:
	GameObject(glm::vec3 &entityPosition, GLuint entityTexture, GLint entityNumElements);

	// Updates the GameObject's state. Can be overriden for children
	virtual void update(double deltaTime);

	// Renders the GameObject using a shader
	virtual void render(Shader &shader, glm::vec3 scale);
	virtual float renderParticles(Shader& shader, float timer, float deltaTime);
	virtual float renderFireParticles(Shader& shader, float timer, float deltaTime, float angle, int numParticles);
	virtual float renderIceParticles(Shader& shader, float timer, float deltaTime, float angle, int numParticles);

	virtual void checkPowerUps(double deltaTime, GameObject* player);
	virtual int checkFuelStations(double deltaTime, GameObject* player);

	// Getters
	inline glm::vec3& getPosition() { return position; }
	inline glm::vec3& getVelocity() { return velocity; }
	inline GLuint getTexture() { return texture; }

	// Setters
	inline void setPosition(glm::vec3& newPosition) { position = newPosition; }
	inline void setVelocity(glm::vec3& newVelocity) { velocity = newVelocity; }
	inline void setTexture(GLuint t) { texture = t; }

	bool poweredUp;				// Checks if player has a power up
	bool exists;
	bool obtained;

protected:
	// Object's Transform Variables
	glm::vec3 position;
	glm::vec3 velocity;

	// Object's details
	GLint numElements;
	float objectSize; // Not currently being used (will be needed for collision detection when objects have a different scale)

	// Object's texture
	GLuint texture;
};