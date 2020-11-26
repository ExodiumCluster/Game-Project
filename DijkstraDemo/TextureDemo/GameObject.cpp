#include "GameObject.h"

/*
	GameObject is responsible for handling the rendering and updating of objects in the game world
	The update method is virtual, so you can inherit from GameObject and override the update functionality (see PlayerGameObject for reference)
*/

GameObject::GameObject(glm::vec3 &entityPosition, GLuint entityTexture, GLint entityNumElements) {
	position = entityPosition;
	texture = entityTexture;
	numElements = entityNumElements;
	velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	exists = true;
	poweredUp = false;
	obtained = false;
}

// Updates the GameObject's state. Can be overriden for children
void GameObject::update(double deltaTime) {
	// Update object position
	position += velocity * (float)deltaTime;
}

void GameObject::checkPowerUps(double deltaTime, GameObject* player) {
	glm::vec3 pos = player->getPosition();
	if (pos[0] > position[0] && pos[0] < position[0] + 1) {
		if (pos[1] > position[1] && pos[1] < position[1] + 1) {
			this->exists = false;
			this->obtained = true;
			player->poweredUp = true;
		}
	}
}

int GameObject::checkFuelStations(double deltaTime, GameObject* player) {
	float playerDist = pow(player->getPosition().x - position.x, 2) + pow(player->getPosition().y - position.y, 2);
	if (playerDist < 3) {
		return 1;
	} 
	return 0;
}

// Renders the GameObject using a shader
void GameObject::render(Shader &shader, glm::vec3 scale) {
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// Setup the transformation matrix for the shader
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);


	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	//transformationMatrix = rotationMatrix * translationMatrix  * scaleMatrix;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);

	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
}

float GameObject::renderParticles(Shader& shader, float deltaTime, float mytime) {
	mytime += 1.05*deltaTime;
	// Bind the particle texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// Set the transformation matrix for the shader
	glm::mat4 scaling = glm::scale(glm::mat4(1.0), glm::vec3(0.2f));
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	
	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = translationMatrix * scaling;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	shader.setUniform1f("time", mytime);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, 2000, GL_UNSIGNED_INT, 0);

	return mytime;
}

float GameObject::renderFireParticles(Shader& shader, float mytime, float deltaTime, float angle, int numParticles) {
	mytime += 1.5*deltaTime;
	// Bind the particle texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// Set the transformation matrix for the shader
	glm::mat4 scaling = glm::scale(glm::mat4(1.0), glm::vec3(0.2f));
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(), angle, glm::vec3(0, 0, 1));
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaling;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	shader.setUniform1f("time", mytime);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numParticles, GL_UNSIGNED_INT, 0);

	return mytime;
}

float GameObject::renderIceParticles(Shader& shader, float mytime, float deltaTime, float angle, int numParticles) {
	mytime += 1.5 * deltaTime;
	// Bind the particle texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// Set the transformation matrix for the shader
	glm::mat4 scaling = glm::scale(glm::mat4(1.0), glm::vec3(0.2f));
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(), angle, glm::vec3(0, 0, 1));
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaling;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	shader.setUniform1f("time", mytime);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numParticles, GL_UNSIGNED_INT, 0);

	return mytime;
}