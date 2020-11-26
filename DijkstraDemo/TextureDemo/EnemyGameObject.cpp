#include <chrono>

#include "EnemyGameObject.h"
#include "Window.h"
#include "Graph.h"
/*
	EnemyGameObject inherits from GameObject
	It overrides GameObject's update method, so that you can check for input to change the velocity of the enemy
*/

EnemyGameObject::EnemyGameObject(glm::vec3& entityPos, GLuint entityTexture, GLuint barrelTex, GLint entityNumElements, GLuint type)
	: GameObject(entityPos, entityTexture, entityNumElements) {

	barrelTexture = barrelTex;
	enemyType = type;
	srand((unsigned int)time(0));

	// SETUP ENEMY SPECIFIC ATTRIBUTES
	float vx;
	if (enemyType == TURRET_ENEMY) {
		numSegments = 0;
		scrapValue = 25;
		firingCooldown = 1;
		bulletDamage = 3;
		health = 75;
		vx = 0.0f;
	}
	else if (enemyType == MOVING_ENEMY) {
		numSegments = 0;
		scrapValue = 15;
		firingCooldown = 0.5;
		bulletDamage = 1.5;
		health = 100;
		if (rand() % 2 == 1) {
			vx = 0.03f;
		}
		else {
			vx = -0.03f;
		}
	}
	else if (enemyType == INSECT_ENEMY) {
		numSegments = 4;
		scrapValue = 10;
		firingCooldown = 0;
		bulletDamage = 1;
		health = 100;
	}
	else if (enemyType == PLANET_CORE) {
		numSegments = 0;
		scrapValue = 200;
		firingCooldown = 0;
		bulletDamage = 1;
		health = 300;
	}
	else {
		vx = 0.0f;
	}

	velocity = glm::vec3(vx, 0.0f, 0.0);
	aimingAngle = -90.0f;

	isOnFire = false;
	burnTimer = 0.0f;

	isFrozen = false;
	frozenTimer = 0.0f;

	fireTimer = 0.0f;
	burnSpot = new GameObject(entityPos, FIRE_TEX, numElements);

	elapsedTime = 0.0f;
	latchedOn = false;
}

// Update function for moving the enemy object around
void EnemyGameObject::update(double deltaTime, std::vector<std::vector<int>> map, glm::vec3 playerPos, int& playerDmg) {
	canShoot = true;

	if (isFrozen && frozenTimer > 0) {
		frozenTimer -= deltaTime;
		return;
	}
	else if (frozenTimer <= 0) {
		frozenTimer = 0.0f;
		isFrozen = false;
	}

	elapsedTime += deltaTime;

	// GET AIMING ANGLE
	int player_dist = pow(playerPos.x - position.x - velocity.x, 2) + pow(playerPos.y - position.y - velocity.y, 2);
	if (enemyType == MOVING_ENEMY) {
		player_dist = pow(playerPos.x - position.x - velocity.x, 2) + 6*pow(playerPos.y - position.y - velocity.y, 2);
	}
	else if (player_dist < pow(8, 2) && enemyType == TURRET_ENEMY) {
		canShoot = false;

		if (mountAngle == 0) { aimingAngle = (atan2(playerPos.x - position.x, playerPos.y - position.y)) * 180 / 3.14159265359f; }
		if (mountAngle == 90) { 
			aimingAngle = (atan2(playerPos.x - position.x, playerPos.y - position.y)) * 180 / 3.14159265359f;
			if (aimingAngle < 0 && aimingAngle > -180) { canShoot = true; } 
			else if (aimingAngle < 180 && aimingAngle > 90) { aimingAngle = -180.0f; }
			else { aimingAngle = 0.0f; }
		}
		if (mountAngle == 180) { 
			aimingAngle = (atan2(playerPos.x - position.x, playerPos.y - position.y)) * 180 / 3.14159265359f; 
			if (aimingAngle < 0) {
				aimingAngle = 90 - (atan2(playerPos.y - position.x, playerPos.x - position.y)) * 180 / 3.14159265359f;
				if (aimingAngle < 270 && aimingAngle > 180) { canShoot = true; } 
				else { aimingAngle = 270.0f; }
			}
		}
		if (mountAngle == 270) { 
			aimingAngle = (atan2(playerPos.x - position.x, playerPos.y - position.y)) * 180 / 3.14159265359f; 
			if (aimingAngle < 180 && aimingAngle > 0) { canShoot = true; }
			else { aimingAngle = 180.0f; }
		}
		if (!canShoot) {
			if (aimingAngle < mountAngle + 90.0f) { canShoot = true; }
			else { aimingAngle = mountAngle + 90.0f; }
			if (aimingAngle > mountAngle - 90.0f) { canShoot = true; }
			else { aimingAngle = mountAngle - 90.0f; }
		}

	}
	else if (player_dist < pow(7, 2) && enemyType == INSECT_ENEMY) {
		if ((float) pow(playerPos.x - position.x - velocity.x, 2) + pow(playerPos.y - position.y - velocity.y, 2) < 0.01f) {
			velocity = glm::vec3(0.0f);
			position = playerPos;
			latchedOn = true;
		}
		else {
			aimingAngle = (atan2(playerPos.y - position.y, playerPos.x - position.x));
			velocity.x = 0.03f * cos(aimingAngle);
			velocity.y = 0.03f * sin(aimingAngle);
			canShoot = false;
			latchedOn = false;
		}
	}
	else {
		canShoot = false;
		aimingAngle = mountAngle + 180;
	}

	// ENEMY SPECIFIC BEHAVIOUR
	if (enemyType == TURRET_ENEMY) {
		//Check tiles relative to enemy's pos
		float offset = map[0].size() / 4;
		int j = round((position.x) / 2) + offset;
		int i = -1 * round((position.y) / 2) + offset;

		// Setup mount angle for turret
		// Turret mounts to first wall it can
		if (map[i + 1][j] == WALL_TILE) {
			mountAngle = 0;
		}
		else if (map[i][j + 1] == WALL_TILE) {
			mountAngle = 90;
		}
		else if (map[i - 1][j] == WALL_TILE) {
			mountAngle = 180;
		}
		else if (map[i][j - 1] == WALL_TILE) {
			mountAngle = 270;
		}
	}

	if (enemyType == MOVING_ENEMY) {
		mountAngle = 180.0f;

		//Check tiles relative to enemy's pos
		float offset = map[0].size() / 4;
		int j = round((position.x + velocity.x) / 2) + offset;
		int i = -1 * round((position.y + velocity.y) / 2) + offset;

		if (player_dist < pow(8, 2)) {
			aimingAngle = (atan2(playerPos.x - position.x, playerPos.y - position.y)) * 180 / 3.14159265359f;
			if (player_dist < pow(4, 2) || (i > 0 && j > 0 && (map[i + 1][j] != WALL_TILE || map[i][j] == WALL_TILE || map[i + 1][j] != BRK_TILE || map[i][j] == BRK_TILE))) {
				velocity.x = 0.0f;
			} else {
				canShoot = false;
				float angleToPlayer = atan2(playerPos.y - position.y - velocity.y, playerPos.x - position.x - velocity.x);
				if (-2 <= aimingAngle && aimingAngle <= 2) {
					velocity.x = 0.0f;
				}
				else if (cos(angleToPlayer) > 0) { velocity.x = 0.03f; }
				else if (cos(angleToPlayer) < 0) { velocity.x = -0.03f; }
			}
		} else if (player_dist > pow(8, 2)) {
			if (i > 0 && j > 0 && (map[i + 1][j] != WALL_TILE || map[i][j] == WALL_TILE)) {
				velocity.x *= -1;
			}
			canShoot = false;
		}
		else {
			aimingAngle = -90.0f;
			if (map[i + 1][j] != WALL_TILE || map[i + 1][j] != BRK_TILE) {
				if (map[i + 1][j - 1] == WALL_TILE || map[i + 1][j - 1] != BRK_TILE) {
					velocity.x = -0.03f;
				}
				else if (map[i + 1][j + 1] == WALL_TILE || map[i + 1][j + 1] == BRK_TILE) {
					velocity.x = 0.03f;
				}
			}
		}

		position.x += velocity.x;
		position.y += velocity.y;
	}

	if (enemyType == INSECT_ENEMY) {
		position.x += velocity.x;
		position.y += velocity.y;
	}

	// ENEMY NON-SPECIFIC BEHAVIOUR
	if (firingCooldown > 0) {
		firingCooldown -= deltaTime;
	} else if (firingCooldown <= 0 && canShoot) {
		
		glm::vec3 bulletOffset = glm::vec3();
		if (enemyType == MOVING_ENEMY) {
			bulletOffset = glm::vec3(0.0f, -0.5f, 0.0f);
		} else if (enemyType == TURRET_ENEMY) {
			if (mountAngle == 0) { bulletOffset = glm::vec3(0.0f, -0.5f, 0.0f); }
			if (mountAngle == 90) { bulletOffset = glm::vec3(0.5f, 0.0f, 0.0f); }
			if (mountAngle == 180) { bulletOffset = glm::vec3(0.0f, 0.5f, 0.0f); }
			if (mountAngle == 270) { bulletOffset = glm::vec3(-0.5f, 0.0f, 0.0f); }
		}
		float angleToPlayer = -atan2(playerPos.y - (position.y), playerPos.x - (position.x)) * 180 / 3.14159265359f + 90;
		bullets.push_back(new ProjectileObject(position+bulletOffset, BULLET_TEX, numElements, angleToPlayer, MACHINE_GUN));

		if (enemyType == TURRET_ENEMY) { firingCooldown = 1; }
		if (enemyType == MOVING_ENEMY) { firingCooldown = 0.5; }
		if (enemyType == INSECT_ENEMY) { firingCooldown = 0.0; }
	}

	if (isOnFire && burnTimer >= 0) {
		health -= 10*deltaTime;
		burnTimer -= deltaTime;
	}
	if (burnTimer <= 0) {
		burnTimer = 0.0f;
		isOnFire = false;
	}

	// UPDATE ENEMY'S BULLETS
	for (int bullet = 0; bullet < bullets.size(); bullet++) {
		bullets[bullet]->update(deltaTime);
		float bulletPosX = bullets[bullet]->getPosition().x;
		float bulletPosY = bullets[bullet]->getPosition().y;
		//Check bullet's pos relative to the player
		float playerBulletDist = (pow(bulletPosX-playerPos.x,2) + pow(bulletPosY-playerPos.y,2));
		if (playerBulletDist < 1) {
			bullets.erase(bullets.end() - 1);
			playerDmg = bulletDamage;
			continue;
		}

		//Check tiles relative to enemy's pos
		float offset = map[0].size() / 4;
		int j = round((bulletPosX) / 2) + offset;
		int i = -1 * round((bulletPosY) / 2) + offset;

		if (i > 0 && i < map.size() && j > 0 && j < map[0].size()) {
			if (map[i][j] == 0) {
				bullets.erase(bullets.end() - 1);
			}
		}
	}
}

glm::mat4 EnemyGameObject::renderBaseTurret(Shader& shader) {
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, TURRET_ENEMY_TEX);

	glm::vec3 offsetBase;
	if (mountAngle == 0) { offsetBase = glm::vec3(0.0f, -0.5f, 0.0f); }
	if (mountAngle == 90) { offsetBase = glm::vec3(0.5f, 0.0f, 0.0f); }
	if (mountAngle == 180) { offsetBase = glm::vec3(0.0f, 0.5f, 0.0f); }
	if (mountAngle == 270) { offsetBase = glm::vec3(-0.5f, 0.0f, 0.0f); }
	burnSpot->setPosition(position+glm::vec3(offsetBase.x*1.5, offsetBase.y*1.5, 0.0f));

	glm::mat4 offsetTurretBase = glm::translate(glm::mat4(1.0f), offsetBase);
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), mountAngle * 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = offsetTurretBase * translationMatrix * rotationMatrix * scaleMatrix;
	glm::mat4 parentTransform = transformationMatrix;

	// pass matrix to shader and draw:
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

	return parentTransform;
}

glm::mat4 EnemyGameObject::renderBaseMoving(Shader& shader) {
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, MOVING_ENEMY_TEX);
	
	if (-2 <= aimingAngle && aimingAngle <= 2) {
		flipFactor = 1;
	}
	else if (velocity.x > 0) {
		flipFactor = -1;
	}
	else if (velocity.x < 0) {
		flipFactor = 1;
	}

	glm::mat4 offsetToGround = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, 0.0f));
	burnSpot->setPosition(position + glm::vec3(0.0f, -0.3f, 0.0f));
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(flipFactor * 1.5f, 1.5f, 1.0f));
	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = offsetToGround * translationMatrix * rotationMatrix * scaleMatrix;
	glm::mat4 parentTransform = transformationMatrix;

	//transformationMatrix = rotationMatrix * translationMatrix  * scaleMatrix;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

	return parentTransform;
}

glm::mat4 EnemyGameObject::renderInsectEnemy(Shader& shader) {
	burnSpot->setPosition(position + glm::vec3(0.0f, -0.3f, 0.0f));

	// RENDER HEAD
	glBindTexture(GL_TEXTURE_2D, INSECT_ENEMY_TEX_HEAD);

	// Set up the transformation matrix for the shader
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f));
	glm::mat4 orbitMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 0, 1));
	glm::mat4 rotMatrix = glm::rotate(glm::mat4(), aimingAngle*180/3.1415f + 180, glm::vec3(0, 0, 1));

	// using position for angle control right now, so comment this out
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

	// Set the transformation matrix in the shader TORSO
	glm::mat4 offset = glm::translate(glm::mat4(), glm::vec3(-0.5, -0.5, 0));
	glm::mat4 offsetundo = glm::translate(glm::mat4(), glm::vec3(0.5, 0.5, 0));
	glm::mat4 transformationMatrix = translationMatrix * offsetundo * orbitMatrix * offset * rotMatrix * scaleMatrix;

	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

	glm::mat4 parentTransform = transformationMatrix;
	glm::mat4 localtransform = glm::mat4();

	// RENDER HP BAR
	glBindTexture(GL_TEXTURE_2D, HP_BAR_TEX);
	// Setup transformations for the health bar
	shader.setUniformMat4("transformationMatrix", glm::translate(glm::mat4(), position + glm::vec3(0.0f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(), 90.0f, glm::vec3(0, 0, -1)) * glm::scale(glm::mat4(), glm::vec3(0.05f, 0.5f * health / 100.0f, 0.0f)));
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

	// RENDER BODY SEGMENTS
	scaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.7f, 0.7f, 0.0f));
	for (int i = 0; i < numSegments; i++) {

		glBindTexture(GL_TEXTURE_2D, INSECT_ENEMY_TEX_BODY);
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f,0.2*sin(6*elapsedTime + i),0.0f));

		// TORSO:
		if (i == numSegments - 1) {
			scaleMatrix = glm::scale(glm::mat4(), glm::vec3(2.0f,0.8f,1.0f));
		}
		localtransform = translationMatrix * scaleMatrix;
		scaleMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f));
		transformationMatrix = parentTransform * localtransform;

		// pass matrix to shader and draw:
		shader.setUniformMat4("transformationMatrix", transformationMatrix);
		glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

		parentTransform = transformationMatrix;
	}

	return parentTransform;
}

glm::mat4 EnemyGameObject::renderPlanetCore(Shader& shader) {
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, 45);

	burnSpot->setPosition(position + glm::vec3(0.0f, -0.3f, 0.0f));
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.0f));
	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	glm::mat4 parentTransform = transformationMatrix;

	//transformationMatrix = rotationMatrix * translationMatrix  * scaleMatrix;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

	return parentTransform;
}

void EnemyGameObject::renderWeapon(Shader& shader, glm::mat4 parentTransform) {
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, barrelTexture);
	// Setup transformations for the enemy's weapon

	glm::mat4 transformationMatrix, localTransform, transMatrix, offsetundo, orbitMatrix, offset, rotationMatrix, scaleMatrix;
	if (enemyType == MOVING_ENEMY) {
		if (aimingAngle < 0) {
			flipFactor = 1;
		}
		else {
			flipFactor = -1;
		}
		transMatrix = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));
		offsetundo = glm::translate(glm::mat4(), glm::vec3(0.2, -0.5, 0.0));
		orbitMatrix = glm::rotate(glm::mat4(), aimingAngle * flipFactor, glm::vec3(0, 0, -1));
		offset = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));
		rotationMatrix = glm::rotate(glm::mat4(), 90.0f, glm::vec3(0, 0, -1));
		scaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.9f, 0.5f, 0.0f));
	}

	if (enemyType == TURRET_ENEMY) {
		transMatrix = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));
		offsetundo = glm::translate(glm::mat4(), glm::vec3(0.0, -0.8, 0.0));
		orbitMatrix = glm::rotate(glm::mat4(), mountAngle + aimingAngle, glm::vec3(0, 0, -1));
		offset = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));
		rotationMatrix = glm::rotate(glm::mat4(), 90.0f, glm::vec3(0, 0, -1));
		scaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.8f, 0.4f, 0.0f));
	}

	// TORSO:
	localTransform = transMatrix * offsetundo * orbitMatrix * offset * rotationMatrix * scaleMatrix;
	// final transformation is parent * local
	transformationMatrix = parentTransform * localTransform;

	// pass matrix to shader and draw:
	shader.setUniformMat4("transformationMatrix", transformationMatrix);

	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

}

//RENDER ENEMY'S HEALTH BAR
void EnemyGameObject::renderHPBar(Shader& shader, glm::mat4 parentTransform) {

	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, HP_BAR_TEX);
	// Setup transformations for the health bar

	glm::mat4 transMatrix, rotationMatrix, scaleMatrix;
	if (enemyType == MOVING_ENEMY) {
		transMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.5f, 0.0f));
		rotationMatrix = glm::rotate(glm::mat4(), 90.0f, glm::vec3(0, 0, -1));
		scaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.05f, 0.5f * health / 100.0f, 0.0f));
	}
	if (enemyType == TURRET_ENEMY) {
		float displayAngle;
		if (mountAngle == 0) {  displayAngle = mountAngle + 90.0f; }
		if (mountAngle == 90) { displayAngle = mountAngle * 1.0f; }
		if (mountAngle == 180) { displayAngle = mountAngle - 90.0f; }
		if (mountAngle == 270) { displayAngle = mountAngle * 1.0f; }
		transMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.5f, 0.0f));
		rotationMatrix = glm::rotate(glm::mat4(), displayAngle, glm::vec3(0, 0, -1));
		scaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.1f, health / 100.0f, 0.0f));
	}
	if (enemyType == PLANET_CORE) {
		transMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.5f, 0.0f));
		rotationMatrix = glm::rotate(glm::mat4(), 90.0f, glm::vec3(0, 0, -1));
		scaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.1f, health / 300.0f, 0.0f));
	}

	glm::mat4 localTransform = transMatrix * rotationMatrix * scaleMatrix;
	glm::mat4 transformationMatrix = parentTransform * localTransform;

	// pass matrix to shader and draw:
	shader.setUniformMat4("transformationMatrix", transformationMatrix);

	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

}
// Render function to display the enemy
void EnemyGameObject::render(Shader& shader, Shader& psShader, Shader& fireShader, double deltaTime, glm::mat4 viewMatrix) {
	if (isOnFire) {
		fireShader.enable();
		fireShader.SetAttributes_particle(2);
		fireShader.setUniformMat4("viewMatrix", viewMatrix);
		fireTimer = burnSpot->renderFireParticles(fireShader, fireTimer, deltaTime, 0.0f, 400);
	}

	shader.enable();
	shader.SetAttributes_sprite();
	shader.setUniformMat4("viewMatrix", viewMatrix);

	if (isFrozen) {
		shader.setUniform3f("colorMod", glm::vec3(0.48f, 0.96f, 1.0f));
	}
	else {
		shader.setUniform3f("colorMod", glm::vec3(1.0f));
	}

	// Set the transformation matrix in the shader
	glm::mat4 parentTransform;

	// RENDER TURRET ENEMY
	if (enemyType == TURRET_ENEMY) {
		parentTransform = renderBaseTurret(shader);
	}
	
	// RENDER MOVING ENEMY
	else if (enemyType == MOVING_ENEMY) {
		parentTransform = renderBaseMoving(shader);
	}

	// RENDER INSECT ENEMY
	else if (enemyType == INSECT_ENEMY) {
		parentTransform = renderInsectEnemy(shader);
	}
	else if (enemyType == PLANET_CORE) {
		parentTransform = renderPlanetCore(shader);
	}

	if (enemyType != PLANET_CORE) { // Planet core enemy does not have a useable weapon
		renderWeapon(shader, parentTransform);
	}
	shader.setUniform3f("colorMod", glm::vec3(1.0f));
	if (enemyType != INSECT_ENEMY) {  // HP BAR is rendered on the insect enemy seperately
		renderHPBar(shader, parentTransform); 
	}

	// RENDER ENEMY'S BULLETS
	for (int i = 0; i < bullets.size(); i++) {
		bullets[i]->render(shader, psShader, deltaTime, viewMatrix);
	}

}