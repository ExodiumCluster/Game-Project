#pragma once
#include "GameObject.h"

#define MACHINE_GUN			"machineGun"
#define GRENADE_LAUNCHER	"grenadeLauncher"
#define FLAME_THROWER		"flameThrower"
#define CRYOLATOR			"cryolator"

// Inherits from GameObject
class ProjectileObject : public GameObject {
public:
	ProjectileObject(glm::vec3& entityPos, GLuint entityTexture, GLint entityNumElements, GLfloat direction, std::string type);

	// Update function for moving the player object around
	virtual void update(double deltaTime);
	virtual void render(Shader& shader, Shader& psShader, double deltaTime, glm::mat4 viewMatrix);

	void bounceGrenade() { bounceDir *= -1; bounceSpeed *= 0.8; elapsedTime = 0.0f; }

	GLfloat		elapsedTime;
	GLfloat		decayTimer;
	GLfloat		dirAngle;
	std::string	bulletType;
	GLfloat		bulletDamage;

	GLint		bounceDir;
	GLfloat		bounceSpeed;
	GameObject* explosion;
	bool		isExploding;
	GLfloat		explosionTimer;
	bool		grenadeIsDamaging;
};
