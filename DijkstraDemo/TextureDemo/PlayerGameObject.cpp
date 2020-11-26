#include "PlayerGameObject.h"

#include "Window.h"
/*
	PlayerGameObject inherits from GameObject
	It overrides GameObject's update method, so that you can check for input to change the velocity of the player
*/

PlayerGameObject::PlayerGameObject(glm::vec3& entityPos, GLuint entityTexture, GLint entityNumElements)
	: GameObject(entityPos, entityTexture, entityNumElements) {

	fuel = 100.0f;
	health = 150.0f;
	maxFuel = 100.0f;
	maxHealth = 150.0f;
	scrap = 534;

	bodyTiltAngle = 0.0f;
	poweredUp = false;
	powerUpTime = 0;

	selectedWeapon = "machineGun";
	availableWeapons = { "machineGun" };
	weaponSwapCooldown = 0.5;

	weaponAngle = 0.0f;
	isFiring = false;
	firingCooldown = 0.2;
	flame1 = new GameObject(position, FIRE_TEX, numElements);
	flame2 = new GameObject(position, FIRE_TEX, numElements);
	flameTimer1 = 0.0f;
	flameTimer2 = 5.0f;

	ice1 = new GameObject(position, ICE_TEX, numElements);
	ice2 = new GameObject(position, ICE_TEX, numElements);
	iceTimer1 = 0.0f;
	iceTimer2 = 5.0f;

	drillFacingAngle = 180.0f;
	isDrilling = false;

	acceleration = glm::vec3(0.05f);
}

/*====================================================================================
									PLAYER INPUT
====================================================================================*/
void PlayerGameObject::getInput(double deltaTime) {
	// Checking for player input and changing velocity
	if (glfwGetKey(Window::getWindow(), GLFW_KEY_W) == GLFW_PRESS && velocity.y < MAX_VELOCITY_Y) {
		velocity.y += acceleration.y / 2;
	}
	else if (glfwGetKey(Window::getWindow(), GLFW_KEY_S) == GLFW_PRESS && velocity.y > -1 * MAX_VELOCITY_Y) {
		velocity.y -= acceleration.y / 2;
	}
	else {
		if (velocity.y > 0.0f) { velocity.y -= acceleration.y; }
		if (velocity.y < 0.0f) { velocity.y += acceleration.y; }
	}
	if (glfwGetKey(Window::getWindow(), GLFW_KEY_D) == GLFW_PRESS && velocity.x < MAX_VELOCITY_X) {
		velocity.x += acceleration.x;
		if (selectedWeapon == GRENADE_LAUNCHER) {
			weaponAngle = -45.0f;
		}
	}
	else if (glfwGetKey(Window::getWindow(), GLFW_KEY_A) == GLFW_PRESS && velocity.x > -1 * MAX_VELOCITY_X) {
		velocity.x -= acceleration.x;
		if (selectedWeapon == GRENADE_LAUNCHER) {
			weaponAngle = 45.0f;
		}
	}
	else {
		if (velocity.x > 0.0f) { velocity.x -= acceleration.x; }
		if (velocity.x < 0.0f) { velocity.x += acceleration.x; }
	}

	// Fire weapon
	if (glfwGetKey(Window::getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS) {
	//if (glfwGetMouseButton(Window::getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		isFiring = true;
	}
	else {
		isFiring = false;
		flameTimer1 = 0.0f;
		flameTimer2 = 5.0f;
	}

	if (weaponSwapCooldown <= 0) {
		// Switch selected weapon
		if (glfwGetKey(Window::getWindow(), GLFW_KEY_TAB) == GLFW_PRESS) {
			selectedWeaponIndex++;
			if (selectedWeaponIndex == availableWeapons.size()) {
				selectedWeaponIndex = 0;
			}
			weaponSwapCooldown = 0.5;
			if (selectedWeapon == GRENADE_LAUNCHER) {
				weaponAngle = 45.0f;
			}
			selectedWeapon = availableWeapons[selectedWeaponIndex];
		}
	}
	else {
		weaponSwapCooldown -= deltaTime;
	}

	if (selectedWeapon == MACHINE_GUN || selectedWeapon == FLAME_THROWER || selectedWeapon == CRYOLATOR) {
		//Barrel Updates based on mouse cursor
		double xpos, ypos;
		glfwGetCursorPos(Window::getWindow(), &xpos, &ypos);
		float vecPosX = position.x + ((xpos + 0.5f) / 800.0f) * 2.0f - 1.0f;
		float vecPosY = position.y + 1.0f - ((ypos + 0.5f) / 600.0f) * 2.0f;
		weaponAngle = atan2(vecPosX - position.x, vecPosY - position.y) * -180 / 3.14159265359f;
	}
}

/*====================================================================================
									UPDATE FUEL
====================================================================================*/
void PlayerGameObject::updateFuel(double deltaTime) {
	//if fuel is 0 then reduce the velocity
	if (fuel <= 0) {
		// Decrement health
		health -= 2 * deltaTime;
		// Reduce speed
		if (velocity[0] > MAX_VELOCITY_X / 2) {
			velocity[0] = MAX_VELOCITY_X / 2;
		}
		else if (velocity[0] < -MAX_VELOCITY_X / 2) {
			velocity[0] = -MAX_VELOCITY_X / 2;
		}

		if (velocity[1] > MAX_VELOCITY_Y / 2) {
			velocity[1] = MAX_VELOCITY_Y / 2;
		}
		else if (velocity[1] < -MAX_VELOCITY_Y / 2) {
			velocity[1] = -MAX_VELOCITY_Y / 2;
		}
	}
	else {
		fuel -= 1.5*deltaTime;
	}
}

/*====================================================================================
									COLLISION DETECTION
====================================================================================*/
void PlayerGameObject::collisionDetection(std::vector<std::vector<int>> map, Graph* gameworld) {
	//Detect relative to player
	float offset = map[0].size() / 4;
	int playerX = round((position.x) / 2) + offset;
	int playerY = -1 * round((position.y) / 2) + offset;
	int playerXWeighted = round((position.x + 0.5 * velocity.x / MAX_VELOCITY_X) / 2) + offset;
	int playerYWeighted = -1 * round((position.y + 0.5 * velocity.y / MAX_VELOCITY_Y) / 2) + offset;

	//Player is out of bounds
	if (playerX < 0 || playerX > map[0].size() - 1 || playerY < 0 || playerY > map.size() - 1) {
		setVelocity(glm::vec3(-0.5 * velocity.x, -0.5 * velocity.y, 0.0f));
	}
	//Player is out of bounds
	else if (playerXWeighted < 0 || playerXWeighted > map[0].size() - 1 || playerYWeighted < 0 || playerYWeighted > map.size() - 1) {
		setVelocity(glm::vec3(-0.5 * velocity.x, -0.5 * velocity.y, 0.0f));
	}
	else {
		//Player collided with a wall
		if (map[playerY][playerXWeighted] == WALL_TILE || map[playerY][playerXWeighted] == BRK_TILE) {
			setVelocity(glm::vec3(-0.5 * velocity.x, velocity.y, 0.0f));
		}
		//Player collided with a wall
		if (map[playerYWeighted][playerX] == WALL_TILE || map[playerYWeighted][playerX] == BRK_TILE) {
			setVelocity(glm::vec3(velocity.x, -0.5 * velocity.y, 0.0f));
		}

		// UDPATE DRILL FACING ANGLE
		if (playerY > 0 && map[playerY - 1][playerX] == BRK_TILE && !isDrilling) {
			float tileX = gameworld->getNodes().at(playerY - 1)->at(playerX)->getX();
			float tileY = -1 * gameworld->getNodes().at(playerY - 1)->at(playerX)->getY() - map.size();
			float playerTileDist = pow(position.x - tileX, 2) + pow(position.y - 2 - tileY, 2);
			if (playerTileDist <= 2) {
				setDrillFacingAngle(0.0f);
				canDrill = true;
			}
			else {
				canDrill = false;
			}
		}
		else if (playerY < map.size() - 1 && map[playerY + 1][playerX] == BRK_TILE && !isDrilling) {
			float tileX = gameworld->getNodes().at(playerY + 1)->at(playerX)->getX();
			float tileY = -1 * gameworld->getNodes().at(playerY + 1)->at(playerX)->getY() - map.size();
			float playerTileDist = pow(position.x - tileX, 2) + pow(position.y - 2 - tileY, 2);
			if (playerTileDist <= 2) {
				setDrillFacingAngle(180.0f);
				canDrill = true;
			}
			else {
				canDrill = false;
			}
		}
		if (playerX > 0 && map[playerY][playerX - 1] == BRK_TILE && !isDrilling) {
			float tileX = gameworld->getNodes().at(playerY)->at(playerX - 1)->getX();
			float tileY = -1 * gameworld->getNodes().at(playerY)->at(playerX - 1)->getY() - map.size();
			float playerTileDist = pow(position.x - tileX, 2) + pow(position.y - 2 - tileY, 2);
			if (playerTileDist <= 2) {
				setDrillFacingAngle(270.0f);
				canDrill = true;
			}
			else {
				canDrill = false;
			}
		}
		else if (playerX < map[0].size() - 1 && map[playerY][playerX + 1] == BRK_TILE && !isDrilling) {
			float tileX = gameworld->getNodes().at(playerY)->at(playerX + 1)->getX();
			float tileY = -1 * gameworld->getNodes().at(playerY)->at(playerX + 1)->getY() - map.size();
			float playerTileDist = pow(position.x - tileX, 2) + pow(position.y - 2 - tileY, 2);
			if (playerTileDist <= 2) {
				setDrillFacingAngle(90.0f);
				canDrill = true;
			}
			else {
				canDrill = false;
			}
		}
	}
}

/*====================================================================================
									UPDATE BULLETS
====================================================================================*/
void PlayerGameObject::updateBullets(double deltaTime, std::vector<std::vector<int>> map, Graph* gameworld, Boss* boss) {
	std::vector<EnemyGameObject*>* enemies = gameworld->getEnemies();

	if (firingCooldown > 0) {
		firingCooldown -= deltaTime;
	}
	if (isFiring) {
		if (firingCooldown <= 0) {
			if (selectedWeapon == MACHINE_GUN) {
				bullets.push_back(new ProjectileObject(position, BULLET_TEX, numElements, -weaponAngle - bodyTiltAngle, selectedWeapon));
				firingCooldown = 1;
			}
			if (selectedWeapon == GRENADE_LAUNCHER) {
				bullets.push_back(new ProjectileObject(position, GRENADE_TEX, numElements, -weaponAngle - bodyTiltAngle, selectedWeapon));
				firingCooldown = 100;
			}
		}
	}
	for (int e = enemies->size() - 1; e >= 0; e--) {
		if ((*enemies)[e]->getHealth() <= 0) {
			scrap += (*enemies)[e]->scrapValue;
			if ((*enemies)[e]->enemyType != PLANET_CORE) {
				enemies->erase(enemies->begin() + e);
				continue;
			}
		}
		if (selectedWeapon == FLAME_THROWER) {
			float playerEnemyDist = (pow(position.x - (*enemies)[e]->getPosition().x, 2) + pow(position.y - (*enemies)[e]->getPosition().y, 2));
			float playerEnemyAngle = atan2f(position.y - (*enemies)[e]->getPosition().y, position.x - (*enemies)[e]->getPosition().x) * 180 / 3.14159265359f;
			if (playerEnemyAngle < 0) {
				playerEnemyAngle += 360;
			}
			float appWeaponAngle = weaponAngle;
			if (weaponAngle < 0) {
				appWeaponAngle += 360;
			}
			appWeaponAngle = ((int)appWeaponAngle - 90);
			if (appWeaponAngle < 0) {
				appWeaponAngle += 360;
			}

			if (isFiring && playerEnemyDist <= 6 && appWeaponAngle - 15 <= playerEnemyAngle <= appWeaponAngle + 15) {
				if (!(*enemies)[e]->isOnFire) {
					(*enemies)[e]->isOnFire = true;
					(*enemies)[e]->burnTimer = 5;
				}
			}
		}

		if (selectedWeapon == CRYOLATOR) {
			float playerEnemyDist = (pow(position.x - (*enemies)[e]->getPosition().x, 2) + pow(position.y - (*enemies)[e]->getPosition().y, 2));
			float playerEnemyAngle = atan2f(position.y - (*enemies)[e]->getPosition().y, position.x - (*enemies)[e]->getPosition().x) * 180 / 3.14159265359f;
			if (playerEnemyAngle < 0) {
				playerEnemyAngle += 360;
			}
			float appWeaponAngle = weaponAngle;
			if (weaponAngle < 0) {
				appWeaponAngle += 360;
			}
			appWeaponAngle = ((int)appWeaponAngle - 90);
			if (appWeaponAngle < 0) {
				appWeaponAngle += 360;
			}

			if (isFiring && playerEnemyDist <= 10 && appWeaponAngle - 15 <= playerEnemyAngle <= appWeaponAngle + 15) {
				if (!(*enemies)[e]->isFrozen) {
					(*enemies)[e]->isFrozen = true;
					(*enemies)[e]->frozenTimer = 7;
				}
			}
		}
	}
	// UPDATE PLAYER'S BULLETS
	for (int bullet = bullets.size()-1; bullet >= 0; bullet--) {
		bullets[bullet]->update(deltaTime);
		//Check tiles relative to enemy's pos
		float offset = map[0].size() / 4;
		int j = round((bullets[bullet]->getPosition().x + 0.005*bullets[bullet]->getVelocity().x) / 2) + offset;
		int i = -1 * round((bullets[bullet]->getPosition().y + 0.005*bullets[bullet]->getVelocity().y) / 2) + offset;

		if (i > 0 && i < map.size() && j > 0 && j < map[0].size()) {
			
			//Collsion between bullet and wall
			if (map[i][j] == WALL_TILE || map[i][j] == BRK_TILE) {
				if (bullets[bullet]->bulletType == MACHINE_GUN) {
					bullets.erase(bullets.begin());
					continue;
				}
				if (bullets[bullet]->bulletType == GRENADE_LAUNCHER) {
					bullets[bullet]->bounceGrenade();
					float grenadeSpeed = pow(bullets[bullet]->getVelocity().x, 2) + pow(bullets[bullet]->getVelocity().y, 2);
					if (grenadeSpeed <= 1 && !bullets[bullet]->isExploding) {
						bullets[bullet]->isExploding = true;
						bullets[bullet]->grenadeIsDamaging = true;
						bullets[bullet]->setVelocity(glm::vec3(0.0f));
					}
					else if (grenadeSpeed <= 1 && bullets[bullet]->explosionTimer >= 1) {
						bullets[bullet]->explosionTimer = 0.0f;
						bullets[bullet]->isExploding = false;
						bullets.erase(bullets.begin());
						continue;
					}
				}
			}
			bool destroyed = false;
			//Collision between bullet and enemy
			bool setToExplode = false;
			for (int e = enemies->size()-1; e >= 0; e--) {
				if ((*enemies)[e]->getHealth() <= 0) {
					scrap += (*enemies)[e]->scrapValue;
					if ((*enemies)[e]->enemyType != PLANET_CORE) {
						enemies->erase(enemies->begin() + e);
						continue;
					}
				}
				float enemyDist = 1.0f;
				if ((*enemies)[e]->enemyType == TURRET_ENEMY) {
					glm::vec3 offsetBase;
					if ((*enemies)[e]->mountAngle == 0) { offsetBase = glm::vec3(0.0f, -0.9f, 0.0f); }
					if ((*enemies)[e]->mountAngle == 90) { offsetBase = glm::vec3(0.9f, 0.0f, 0.0f); }
					if ((*enemies)[e]->mountAngle == 180) { offsetBase = glm::vec3(0.0f, 0.9f, 0.0f); }
					if ((*enemies)[e]->mountAngle == 270) { offsetBase = glm::vec3(-0.9f, 0.0f, 0.0f); }
					enemyDist = pow((*enemies)[e]->getPosition().x + offsetBase.x - bullets[bullet]->getPosition().x, 2) + pow((*enemies)[e]->getPosition().y + offsetBase.y - bullets[bullet]->getPosition().y, 2);
				}
				if ((*enemies)[e]->enemyType == MOVING_ENEMY) {
					enemyDist = pow((*enemies)[e]->getPosition().x - bullets[bullet]->getPosition().x, 2) + pow((*enemies)[e]->getPosition().y - bullets[bullet]->getPosition().y, 2);
				}
				else if ((*enemies)[e]->enemyType == INSECT_ENEMY) {
					enemyDist = pow((*enemies)[e]->getPosition().x - bullets[bullet]->getPosition().x, 2) + pow((*enemies)[e]->getPosition().y - bullets[bullet]->getPosition().y, 2);
				}
				else if ((*enemies)[e]->enemyType == PLANET_CORE) {
					enemyDist = pow((*enemies)[e]->getPosition().x - bullets[bullet]->getPosition().x, 2) + pow((*enemies)[e]->getPosition().y - bullets[bullet]->getPosition().y, 2);
				}
				
				if ((((*enemies)[e]->enemyType == INSECT_ENEMY && (*enemies)[e]->latchedOn) || enemyDist < 0.5f) && selectedWeapon == MACHINE_GUN) {
					(*enemies)[e]->damage(bullets[bullet]->bulletDamage);
					if ((*enemies)[e]->getHealth() <= 0) {
						destroyed = true;
						scrap += (*enemies)[e]->scrapValue;
						if ((*enemies)[e]->enemyType != PLANET_CORE) {
							enemies->erase(enemies->begin() + e);
						}
					}
				}
				if (enemyDist < 4 && selectedWeapon == GRENADE_LAUNCHER) {
					if (bullets[bullet]->isExploding && bullets[bullet]->grenadeIsDamaging) {
						setToExplode = true;
						(*enemies)[e]->damage(bullets[bullet]->bulletDamage);
						if ((*enemies)[e]->getHealth() <= 0) {
							scrap += (*enemies)[e]->scrapValue;
							if ((*enemies)[e]->enemyType != PLANET_CORE) {
								enemies->erase(enemies->begin() + e);
							}
						}
					}
				}
			}
			if (destroyed) {
				bullets.erase(bullets.begin());
				continue;
			}

			// Collision between bullet and boss
			float bossDist = pow(boss->getPosition().x - bullets[bullet]->getPosition().x, 2) + pow(boss->getPosition().y - bullets[bullet]->getPosition().y, 2);
			if (bossDist < 1 && (selectedWeapon == MACHINE_GUN || selectedWeapon == FLAME_THROWER || selectedWeapon == CRYOLATOR)) {
				boss->damage(bullets[bullet]->bulletDamage);
				bullets.erase(bullets.begin());
			}
			if (bossDist < 4 && selectedWeapon == GRENADE_LAUNCHER) {
				if (bullets[bullet]->isExploding && bullets[bullet]->grenadeIsDamaging) {
					boss->damage(bullets[bullet]->bulletDamage);
					bullets[bullet]->grenadeIsDamaging = false;
					setToExplode = true;
				}
			}

			if (setToExplode) {
				bullets[bullet]->grenadeIsDamaging = false;
			}
		}
	}
}

/*====================================================================================
								UPDATE DRILL
====================================================================================*/
void PlayerGameObject::updateDrill(std::vector<std::vector<int>>& map, Graph* gameworld) {
	float offset = map[0].size() / 4;
	if (canDrill) {
		if (glfwGetKey(Window::getWindow(), GLFW_KEY_E) == GLFW_PRESS) {
			canDrill = false;
			isDrilling = true;
			drillStartPosX = round((position.x + 0.5 * velocity.x / MAX_VELOCITY_X) / 2) + offset;
			drillStartPosY = -1 * round((position.y + 0.5 * velocity.y / MAX_VELOCITY_Y) / 2) + offset;
		}
	}
	if (isDrilling) {
		int drillPosX = round((position.x + 0.5 * velocity.x / MAX_VELOCITY_X) / 2) + offset;
		int drillPosY = -1 * round((position.y + 0.5 * velocity.y / MAX_VELOCITY_Y) / 2) + offset;
		float drillDistance = pow((drillPosX - drillStartPosX), 2) + pow((drillPosY - drillStartPosY), 2);
		if (drillDistance >= 2.5) {
			isDrilling = false;
			int i;
			int j;
			if (drillFacingAngle == 0.0f) { i = drillPosY + 1; j = drillPosX; }
			if (drillFacingAngle == 90.0f) { i = drillPosY; j = drillPosX - 1; }
			if (drillFacingAngle == 180.0f) { i = drillPosY - 1; j = drillPosX; }
			if (drillFacingAngle == 270.0f) { i = drillPosY; j = drillPosX + 1; }
			gameworld->getNodes().at(-1 * i + map[0].size() - 1)->at(j)->setType("empty");
			map[i][j] = EMPTY_TILE;
		}
		else {
			position.x += 0.02 * cos((drillFacingAngle - 90) * 0.0174533f);
			position.y += -0.02 * sin((drillFacingAngle - 90) * 0.0174533f);
		}
	}
}

// UPDATE
void PlayerGameObject::update(double deltaTime, std::vector<std::vector<int>>& map, Graph* gameworld, Boss* boss) {	

	getInput(deltaTime);
	updateFuel(deltaTime);
	updateBullets(deltaTime, map, gameworld, boss);
	updateDrill(map, gameworld);
	collisionDetection(map, gameworld);

	// need only start the timer ONCE
	if (powerUpTime == 0) {
		if (poweredUp) {
			powerUpTime = clock();
		}
	}

	double duration = (clock() - powerUpTime) / (double) CLOCKS_PER_SEC;
	if (duration >= 25) {
		powerUpTime = 0;
		poweredUp = false;
	}
	

	position.x += velocity.x * (double)deltaTime;
	position.y += velocity.y * (double)deltaTime;
	bodyTiltAngle = -3 * velocity.x;

}

/*====================================================================================
								RENDER PLAYER
====================================================================================*/
glm::mat4 PlayerGameObject::renderPlayer(Shader& shader) {
	/*===================BASE TRANSFORMATIONS===================*/
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, BODY_TEX);

	// Setup the transformation matrix for the shader
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), bodyTiltAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	if (poweredUp) {
		shader.setUniform3f("colorMod", glm::vec3(1.0f, 0.8f, 0.0f));
	}
	else {
		shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	}

	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	glm::mat4 parentTransform = transformationMatrix;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);

	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

	return parentTransform;

}

/*====================================================================================
								RENDER DRILL
====================================================================================*/
void PlayerGameObject::renderDrill(Shader& shader, glm::mat4 parentTransform) {
	/*===================DRILL TRANSFORMATIONS===================*/
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, DRILL_TEX);
	// Setup transformations for the player's drill
	glm::mat4 localTransform = glm::mat4();

	glm::mat4 scaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.8f, 0.8f, 0.0f));
	glm::mat4 orbitMatrix = glm::rotate(glm::mat4(), drillFacingAngle, glm::vec3(0, 0, -1));

	// Create an offset for the drill's orbit
	glm::mat4 offset = glm::translate(glm::mat4(), glm::vec3(0.0, 0.75, 0.0));

	// Set the transformation matrix in the shader
	localTransform = orbitMatrix * offset * scaleMatrix;
	glm::mat4 transformationMatrix = parentTransform * localTransform;
	// pass matrix to shader and draw:
	shader.setUniformMat4("transformationMatrix", transformationMatrix);

	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
}

/*====================================================================================
								RENDER MACHINE GUN
====================================================================================*/
void PlayerGameObject::renderMachineGun(Shader& shader, glm::mat4 parentTransform) {
	/*===================BARREL TRANSFORMATIONS===================*/
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, BARREL_TEX);
	// Setup transformations for player's barrel

	glm::mat4 scaleMatrix = glm::scale(glm::mat4(), glm::vec3(0.9f, 0.3f, 0.0f));
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(), 270.0f, glm::vec3(0, 0, 1));
	glm::mat4 orbitMatrix = glm::rotate(glm::mat4(), weaponAngle, glm::vec3(0, 0, 1)); //needed for rotating with mouse pos

	// Create an offset for the barrel's orbit
	glm::mat4 offset = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));
	glm::mat4 offsetundo = glm::translate(glm::mat4(), glm::vec3(0.0, -0.5, 0.0));
	glm::mat4 transMatrix = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));

	// TORSO:
	glm::mat4 localTransform = transMatrix * offsetundo * orbitMatrix * offset * rotationMatrix * scaleMatrix;
	// final transformation is parent * local
	glm::mat4 transformationMatrix = parentTransform * localTransform;

	// pass matrix to shader and draw:
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
}

/*====================================================================================
								RENDER GRENADE LAUNCHER
====================================================================================*/
void PlayerGameObject::renderGrenadeLauncher(Shader& shader, glm::mat4 parentTransform) {
	/*===================BARREL TRANSFORMATIONS===================*/
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, GRENADE_LAUNCHER_TEX);
	// Setup transformations for player's barrel

	glm::mat4 scaleMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, 0.4f, 0.0f));
	glm::mat4 orbitMatrix = glm::rotate(glm::mat4(), weaponAngle, glm::vec3(0, 0, 1)); //needed for rotating with mouse pos
	glm::mat4 rotMatrix = glm::rotate(glm::mat4(), 90.0f, glm::vec3(0, 0, 1));

	// Create an offset for the barrel's orbit
	glm::mat4 offset = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));
	glm::mat4 offsetundo = glm::translate(glm::mat4(), glm::vec3(0.0, -0.5, 0.0));
	glm::mat4 transMatrix = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));

	// TORSO:
	glm::mat4 localTransform = transMatrix * offsetundo * orbitMatrix * offset * rotMatrix * scaleMatrix;
	// final transformation is parent * local
	glm::mat4 transformationMatrix = parentTransform * localTransform;

	// pass matrix to shader and draw:
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
}

/*====================================================================================
								RENDER FLAME THROWER
====================================================================================*/
void PlayerGameObject::renderFlamethrower(Shader& shader, glm::mat4 parentTransform) {
	/*===================BARREL TRANSFORMATIONS===================*/
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, FLAME_THROWER_TEX);
	// Setup transformations for player's barrel

	glm::mat4 scaleMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, 0.5f, 0.0f));
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(), 270.0f, glm::vec3(0, 0, 1));
	glm::mat4 orbitMatrix = glm::rotate(glm::mat4(), weaponAngle, glm::vec3(0, 0, 1)); //needed for rotating with mouse pos

	// Create an offset for the barrel's orbit
	glm::mat4 offset = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));
	glm::mat4 offsetundo = glm::translate(glm::mat4(), glm::vec3(0.0, -0.5, 0.0));
	glm::mat4 transMatrix = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));

	// TORSO:
	glm::mat4 localTransform = transMatrix * offsetundo * orbitMatrix * offset * rotationMatrix * scaleMatrix;
	// final transformation is parent * local
	glm::mat4 transformationMatrix = parentTransform * localTransform;

	// pass matrix to shader and draw:
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
}

/*====================================================================================
								RENDER CRYOLATOR
====================================================================================*/
void PlayerGameObject::renderCryolator(Shader& shader, glm::mat4 parentTransform) {
	/*===================BARREL TRANSFORMATIONS===================*/
	// Bind the entities texture
	glBindTexture(GL_TEXTURE_2D, CRYOLATOR_TEX);
	// Setup transformations for player's barrel

	glm::mat4 scaleMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, 0.5f, 0.0f));
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(), 270.0f, glm::vec3(0, 0, 1));
	glm::mat4 orbitMatrix = glm::rotate(glm::mat4(), weaponAngle, glm::vec3(0, 0, 1)); //needed for rotating with mouse pos

	// Create an offset for the barrel's orbit
	glm::mat4 offset = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));
	glm::mat4 offsetundo = glm::translate(glm::mat4(), glm::vec3(0.0, -0.5, 0.0));
	glm::mat4 transMatrix = glm::translate(glm::mat4(), glm::vec3(0.0, 0.5, 0.0));

	// TORSO:
	glm::mat4 localTransform = transMatrix * offsetundo * orbitMatrix * offset * rotationMatrix * scaleMatrix;
	// final transformation is parent * local
	glm::mat4 transformationMatrix = parentTransform * localTransform;

	// pass matrix to shader and draw:
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	// Draw the entity
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
}

// RENDER
void PlayerGameObject::render(Shader& shader, Shader& grenadeShader, Shader& fireShader, Shader& iceShader, double deltaTime, glm::mat4 viewMatrix) {

	shader.enable();
	shader.SetAttributes_sprite();
	shader.setUniformMat4("viewMatrix", viewMatrix);

	glm::mat4 parentTransform = renderPlayer(shader);

	renderDrill(shader, parentTransform);

	if (selectedWeapon == MACHINE_GUN) {
		renderMachineGun(shader, parentTransform);
	}

	if (selectedWeapon == GRENADE_LAUNCHER) {
		renderGrenadeLauncher(shader, parentTransform);
	}

	if (selectedWeapon == FLAME_THROWER) {
		renderFlamethrower(shader, parentTransform);
		if (isFiring) {
			fireShader.enable();
			fireShader.SetAttributes_particle(2);
			fireShader.setUniformMat4("viewMatrix", viewMatrix);
			glm::vec3 flamePos = glm::vec3(position.x + cos((weaponAngle + bodyTiltAngle + 90) * 0.01745329251f) - 0.025f, position.y + sin((weaponAngle + bodyTiltAngle + 90) * 0.01745329251f) - 0.025f, 0.0);
			flame1->setPosition(flamePos);
			flame2->setPosition(flamePos);
			flameTimer1 = flame1->renderFireParticles(fireShader, flameTimer1, deltaTime, weaponAngle+bodyTiltAngle, 2000);
			flameTimer2 = flame2->renderFireParticles(fireShader, flameTimer2, deltaTime, weaponAngle+bodyTiltAngle, 2000);
		}
	}

	if (selectedWeapon == CRYOLATOR) {
		renderCryolator(shader, parentTransform);
		if (isFiring) {
			iceShader.enable();
			iceShader.SetAttributes_particle(3);
			iceShader.setUniformMat4("viewMatrix", viewMatrix);
			glm::vec3 icePos = glm::vec3(position.x + cos((weaponAngle + bodyTiltAngle + 90) * 0.01745329251f) - 0.025f, position.y + sin((weaponAngle + bodyTiltAngle + 90) * 0.01745329251f) - 0.025f, 0.0);
			ice1->setPosition(icePos);
			ice2->setPosition(icePos);
			iceTimer1 = ice1->renderIceParticles(iceShader, iceTimer1, deltaTime, weaponAngle + bodyTiltAngle, 2000);
			iceTimer2 = ice2->renderIceParticles(iceShader, iceTimer2, deltaTime, weaponAngle + bodyTiltAngle, 2000);
		}
	}

	// RENDER PLAYER'S BULLETS
	for (int i = 0; i < bullets.size(); i++) {
		bullets[i]->render(shader, grenadeShader, deltaTime, viewMatrix);
	}
}
