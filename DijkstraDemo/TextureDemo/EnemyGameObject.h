#pragma once

#include "GameObject.h"
#include "ProjectileObject.h"


#define TURRET_ENEMY			3
#define MOVING_ENEMY			4
#define INSECT_ENEMY			7
#define PLANET_CORE				8

#define BULLET_TEX				2
#define TURRET_ENEMY_TEX		10
#define MOVING_ENEMY_TEX		11
#define MOVING_BARREL_TEX		12
#define TURRET_BARREL_TEX		13
#define INSECT_ENEMY_TEX_BODY	14
#define INSECT_ENEMY_TEX_HEAD	15
#define HP_BAR_TEX				32

#define MACHINE_GUN				"machineGun"
#define GRENADE_LAUNCHER		"grenadeLauncher"
#define FLAME_THROWER			"flameThrower"
#define CRYOLATOR				"cryolator"
#define FIRE_TEX			    7

// Inherits from GameObject
class EnemyGameObject : public GameObject {
public:
	EnemyGameObject(glm::vec3& entityPos, GLuint entityTexture, GLuint barrelTex, GLint entityNumElements, GLuint type);

	virtual void update(double deltaTime, std::vector<std::vector<int>> map, glm::vec3 playerPos, int& playerDmg);

	glm::mat4 renderBaseTurret(Shader& shader);
	glm::mat4 renderBaseMoving(Shader& shader);
	glm::mat4 renderInsectEnemy(Shader& shader);
	glm::mat4 renderPlanetCore(Shader& shader);
	void renderWeapon(Shader& shader, glm::mat4 parentTransform);
	void renderHPBar(Shader& shader, glm::mat4 parentTransform);
	virtual void render(Shader& shader, Shader& psShader, Shader& fireShader, double deltaTime, glm::mat4 viewMatrix);

	void damage(GLfloat dmg) { health -= dmg; }

	// Getters
	inline GLfloat getHealth() { return health; }

	// ENEMY ATTRIBUTES
	GLuint		enemyType;
	GLint		mountAngle;
	GLfloat		aimingAngle;
	GLint		flipFactor;
	GLuint		barrelTexture;
	GLfloat		firingCooldown;
	GLfloat		bulletDamage;
	GLfloat		health;
	GLuint      scrapValue;
	bool		canShoot;

	bool		isOnFire;
	GLfloat		burnTimer;
	bool		isFrozen;
	GLfloat		frozenTimer;

	GLfloat		fireTimer;
	GameObject* burnSpot;

	GLuint		numSegments; //This is used only for the insect enemy

	GLfloat		elapsedTime;
	bool		latchedOn;

	std::vector<ProjectileObject*> bullets;
};
