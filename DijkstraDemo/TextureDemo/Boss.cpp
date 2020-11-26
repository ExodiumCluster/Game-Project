#include "Boss.h"

#include "Window.h"

/*
	PlayerGameObject inherits from GameObject
	It overrides GameObject's update method, so that you can check for input to change the velocity of the player
*/

Boss::Boss(glm::vec3& entityPos, GLint entityNumElements)
	: GameObject(entityPos, BODY_TEX, entityNumElements) {

	texture = BODY_TEX;
	//BOSS ATTRIBUTES
	health = 300.0f;
	machineGunAngle = 0.0f;

	searchCooldown = 5;
	firingCooldown = 0.2;

	acceleration = glm::vec3(0.05f);
}

// Update function for moving the player object around
void Boss::update(double deltaTime, std::vector<std::vector<int>>& map, Graph* gameworld, glm::vec3 playerPos, int& playerDmg) {
	
	if (health <= 0) {
		return;
	}

	// decrement the search cooldown 
	if (searchCooldown > 0) {
		searchCooldown -= deltaTime;
	}
	else if (searchCooldown <= 0) {
		// search for path between boss and player
		pathNodes = gameworld->pathfind(position);
		pathNodeIndex = 0;

		// reset the cooldown
		searchCooldown = 5;
	}

	// Get the directional velocity based on the next node's position and the player's position
	if (pathNodes.size() > 0 && pathNodeIndex < pathNodes.size()) {
		double nodeX = pathNodes[pathNodeIndex]->getX();
		double nodeY = pathNodes[pathNodeIndex]->getY();
		double angle = atan2f(position.y - nodeY, position.x - nodeX);

		// Set that velocity 
		velocity = glm::vec3(-0.05*cos(angle), -0.05*sin(angle), 0.0f);
		
		// When the boss is within a certain distance of the player, then fire at them
		float playerDist = pow(position.x - playerPos.x, 2) + pow(position.y - playerPos.y, 2);
		if (playerDist < pow(8,2)) {
			isFiring = true;
		} else {
			isFiring = false;
		}

		// If the player has reached close to the node in questions, change the target to be the next node in line
		if (abs(position.x - nodeX) < 0.1 && abs(position.y - nodeY) < 0.1) {
			pathNodeIndex++;
		}
		// When they reach the end of the path
		if (pathNodeIndex == pathNodes.size()) {
			velocity = glm::vec3(0.0f);
		}
	}

	if (firingCooldown > 0) {
		firingCooldown -= deltaTime;
	} else if (isFiring && firingCooldown <= 0) {
		float angleToPlayer = -atan2(playerPos.y - (position.y), playerPos.x - (position.x)) * 180 / 3.14159265359f + 90;
		bullets.push_back(new ProjectileObject(position, BULLET_TEX, numElements, angleToPlayer, MACHINE_GUN));

		firingCooldown = 0.2;
	}
	// UPDATE BOSS'S BULLETS
	for (int bullet = 0; bullet < bullets.size(); bullet++) {

		bullets[bullet]->update(deltaTime);
		float bulletPosX = bullets[bullet]->getPosition().x;
		float bulletPosY = bullets[bullet]->getPosition().y;
		//Check bullet's pos relative to the player
		float playerBulletDist = (pow(bulletPosX-playerPos.x,2) + pow(bulletPosY-playerPos.y,2));
		if (playerBulletDist < 1) {
			bullets.erase(bullets.end() - 1);
			playerDmg = 5;
			continue;
		}

		bullets[bullet]->update(deltaTime);
		//Check tiles relative to enemy's pos
		float offset = map[0].size() / 4;
		int j = round((bullets[bullet]->getPosition().x) / 2) + offset;
		int i = -1 * round((bullets[bullet]->getPosition().y) / 2) + offset;

		if (i > 0 && i < map.size() && j > 0 && j < map[0].size()) {
			if (map[i][j] == 0) {
				bullets.erase(bullets.end() - 1);
			}
		}
	}

	position.x += velocity.x;
	position.y += velocity.y;
}

glm::mat4 Boss::renderBoss(Shader& shader) {
	/*===================BASE TRANSFORMATIONS===================*/
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, BODY_TEX);

	// Setup the transformation matrix for the shader
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	glm::mat4 parentTransform = transformationMatrix;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);

	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

	return parentTransform;
}

void Boss::renderHPBar(Shader& shader, glm::mat4 parentTransform) {
	// RENDER BOSS HP BAR
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, HPBAR_TEX);
	// Setup transformations for boss's hp bar

	glm::mat4 scaleMatrixHP = glm::scale(glm::mat4(), glm::vec3(0.1f, health / 300.0f, 0.0f));
	glm::mat4 orbitMatrixHP = glm::rotate(glm::mat4(), 90.0f, glm::vec3(0, 0, -1));
	glm::mat4 transMatrixHP = glm::translate(glm::mat4(), glm::vec3(-1.0, 0.0, 0.0));

	// Set the transformation matrix in the shader
	glm::mat4 localTransformHP = orbitMatrixHP * transMatrixHP * scaleMatrixHP;
	glm::mat4 transformationMatrix = parentTransform * localTransformHP;
	// pass matrix to shader and draw:
	shader.setUniformMat4("transformationMatrix", transformationMatrix);

	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
}

void Boss::renderIndicator(Shader& shader, glm::vec3 playerPos) {
	// RENDER PLAYER INDICATOR
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, INDICATOR_TEX);

	// Setup the transformation matrix for the shader
	float indicatorAngle = atan2(position.y - playerPos.y, position.x - playerPos.x);
	float indicatorX = 10 * cos(indicatorAngle);
	if (indicatorX > 6.67) { indicatorX = 6.67; }
	if (indicatorX < -6.67) { indicatorX = -6.67; }
	float indicatorY = 8 * sin(indicatorAngle);
	if (indicatorY > 4) { indicatorY = 4; }
	if (indicatorY < -5) { indicatorY = -5; }
	glm::vec3 indicatorOffset = glm::vec3(indicatorX, indicatorY, 0.0f);
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), playerPos + indicatorOffset);
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), indicatorAngle * 180 / 3.14159265359f + 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));

	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);

	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
}

// Render function to display the player
void Boss::render(Shader& shader, Shader& psShader, double deltaTime, glm::vec3 playerPos, glm::mat4 viewMatrix) {
	if (health <= 0) {
		return;
	}

	shader.enable();
	shader.SetAttributes_sprite();
	shader.setUniformMat4("viewMatrix", viewMatrix);

	glm::mat4 parentTransform = renderBoss(shader);

	renderHPBar(shader, parentTransform);

	if (1.2*pow(playerPos.x - position.x, 2) + pow(playerPos.y - position.y, 2) > pow(8,2)) {
		renderIndicator(shader, playerPos);
	}

	// RENDER BOSS'S BULLETS
	for (int i = 0; i < bullets.size(); i++) {
		bullets[i]->render(shader, psShader, deltaTime, viewMatrix);
	}
}
