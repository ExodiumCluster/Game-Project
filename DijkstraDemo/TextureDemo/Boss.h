#pragma once

#include "GameObject.h"
#include "Graph.h"

#define MAX_VELOCITY_X 6
#define MAX_VELOCITY_Y 6
#define BODY_TEX	   31
#define HPBAR_TEX	   32
#define INDICATOR_TEX  33

// Inherits from GameObject
class Boss : public GameObject {
public:
	Boss(glm::vec3& entityPos, GLint entityNumElements);

	virtual void update(double deltaTime, std::vector<std::vector<int>>& map, Graph* gameworld, glm::vec3 playerPos, int& playerDmg);

	glm::mat4 renderBoss(Shader& shader);
	void renderHPBar(Shader& shader, glm::mat4 parentTransform);
	void renderIndicator(Shader& shader, glm::vec3 playerPos);
	virtual void render(Shader& shader, Shader& psShader, double deltaTime, glm::vec3 playerPos, glm::mat4 viewMatrix);

	void damage(GLfloat dmg) { health -= dmg; }

	// Getters
	inline GLfloat getHealth() { return health; }

	// BOSS ATTRIBUTES
	GLfloat health;				// Goes down when shot by player
	
	GLfloat machineGunAngle;	// Angle will be dependent on mouse (This may not be needed)
	bool	isFiring;
	GLfloat	firingCooldown;
	std::vector<ProjectileObject*> bullets;

	GLfloat searchCooldown;		// Controls how often the boss will poll the graph class for finding the path between it and the player

	GLfloat   speed;
	glm::vec3 acceleration;

	std::vector<Node*> pathNodes;
	GLuint			   pathNodeIndex;
};


