
#include "ProjectileObject.h"
#include "Window.h"

/*
	PlayerGameObject inherits from GameObject
	It overrides GameObject's update method, so that you can check for input to change the velocity of the player
*/

ProjectileObject::ProjectileObject(glm::vec3& entityPos, GLuint entityTexture, GLint entityNumElements, GLfloat direction, std::string type)
	: GameObject(entityPos, entityTexture, entityNumElements) {
	bulletType = type;
	dirAngle = direction;
	decayTimer = 500;

	if (bulletType == MACHINE_GUN) {
		bulletDamage = 1.0f;
	}
	if (bulletType == GRENADE_LAUNCHER) {
		bulletDamage = 150.0f;
	}
	elapsedTime = 0.0f;
	bounceDir = 1;
	bounceSpeed = 5.0f;
	isExploding = false; 
	grenadeIsDamaging = false;
	explosion = new GameObject(glm::vec3(position), entityTexture, entityNumElements);
	explosionTimer = 0.0f;
}

// Update function for moving the bullet object around
void ProjectileObject::update(double deltaTime) {

	if (decayTimer > 0) {
		decayTimer -= 1;
	}

	if (bulletType == MACHINE_GUN) {
		velocity.x = 40.0f * sin(dirAngle * 0.01745329);
		velocity.y = 40.0f * cos(dirAngle * 0.01745329);
	}
	if (bulletType == GRENADE_LAUNCHER) {
		float gravity = -9.81f;
		elapsedTime += deltaTime;
		velocity.x = bounceDir * bounceSpeed * sin(dirAngle * 0.01745329);
		velocity.y = bounceDir * bounceSpeed * cos(dirAngle * 0.01745329) + 0.5*gravity*pow(elapsedTime,2);
	}
	
	// Update object position
	position += velocity * (float)deltaTime;
	explosion->setPosition(position);
}

// Update function for moving the bullet object around
void ProjectileObject::render(Shader& shader, Shader& psShader, double deltaTime, glm::mat4 viewMatrix) {

	if (!isExploding) {
		shader.enable();
		shader.SetAttributes_sprite();
		shader.setUniformMat4("viewMatrix", viewMatrix);
		// Bind the entities texture
		glBindTexture(GL_TEXTURE_2D, texture);

		// Setup the transformation matrix for the shader
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
		float displayAngle;
		if (bulletType == MACHINE_GUN) { displayAngle = -dirAngle; }
		if (bulletType == GRENADE_LAUNCHER) { displayAngle = -dirAngle * elapsedTime; }
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), displayAngle, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.6f, 1.0f));

		// Set the transformation matrix in the shader
		glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
		//transformationMatrix = rotationMatrix * translationMatrix  * scaleMatrix;
		shader.setUniformMat4("transformationMatrix", transformationMatrix);

		// Draw the entity
		glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
	}

	if (isExploding) {
		psShader.enable();
		psShader.SetAttributes_particle(1);
		psShader.setUniformMat4("viewMatrix", viewMatrix);
		explosionTimer = explosion->renderParticles(psShader, explosionTimer, deltaTime);
	}
}
