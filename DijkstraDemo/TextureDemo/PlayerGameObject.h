#pragma once

#include "GameObject.h"
#include "Graph.h"
#include "Boss.h"
#include "time.h"

#define MAX_VELOCITY_X 6
#define MAX_VELOCITY_Y 6

#define BODY_TEX			 1
#define BULLET_TEX			 2
#define DRILL_TEX			 3
#define BARREL_TEX			 4
#define GRENADE_LAUNCHER_TEX 5
#define GRENADE_TEX			 6
#define FIRE_TEX			 7
#define ICE_TEX				 7
#define FLAME_THROWER_TEX	 8
#define CRYOLATOR_TEX		 9

#define MACHINE_GUN			"machineGun"
#define GRENADE_LAUNCHER	"grenadeLauncher"
#define FLAME_THROWER		"flameThrower"
#define CRYOLATOR			"cryolator"


// Inherits from GameObject
class PlayerGameObject : public GameObject {
public:
	PlayerGameObject(glm::vec3& entityPos, GLuint entityTexture, GLint entityNumElements);

	void		 getInput(double deltaTime);
	void		 updateFuel(double deltaTime);
	void		 collisionDetection(std::vector<std::vector<int>> map, Graph* gameworld);
	void		 updateBullets(double deltaTime, std::vector<std::vector<int>> map, Graph* gameworld, Boss* boss);
	void	     updateDrill(std::vector<std::vector<int>>& map, Graph* gameworld);
	virtual void update(double deltaTime, std::vector<std::vector<int>>& map, Graph* gameworld, Boss* boss);

	// Render Methods
	glm::mat4	 renderPlayer(Shader& shader);
	void		 renderDrill(Shader& shader, glm::mat4 parentTransform);
	void		 renderMachineGun(Shader& shader, glm::mat4 parentTransform);
	void		 renderGrenadeLauncher(Shader& shader, glm::mat4 parentTransform);
	void		 renderFlamethrower(Shader& shader, glm::mat4 parentTransform);
	void		 renderCryolator(Shader& shader, glm::mat4 parentTransform);
	virtual void render(Shader& shader, Shader& grenadeShader, Shader& fireShader, Shader& iceShader, double deltaTime, glm::mat4 viewMatrix);

	// Setters
	inline void setDrillFacingAngle(GLfloat angle) { drillFacingAngle = angle; }

	// PLAYER ATTRIBUTES
	GLfloat fuel;				// Is decremented over time
	GLfloat maxFuel;
	GLfloat health;				// Goes down when hit by enemies
	GLfloat maxHealth;
	GLfloat bodyTiltAngle;		// When moving left or right, body will tilt (lean) into that direction slightly
	GLuint  scrap;				// Player's currency from killing enemies
	double powerUpTime;			// Used to calculate the duration of the power up

	std::vector<std::string> availableWeapons;
	std::string	selectedWeapon;
	GLuint selectedWeaponIndex;
	GLfloat weaponSwapCooldown;

	GLfloat weaponAngle;		// Angle will be dependent on which type of weapon is current selected
	bool	isFiring;
	GLuint	firingCooldown;
	std::vector<ProjectileObject*> bullets;
	GameObject* flame1;
	GameObject* flame2;
	GLfloat flameTimer1;
	GLfloat flameTimer2;

	GameObject* ice1;
	GameObject* ice2;
	GLfloat iceTimer1;
	GLfloat iceTimer2;

	GLfloat drillFacingAngle;	// Will be 0, 90, 180, or 270 depending on which of the four directions to drill
	bool    isDrilling;			// Used to interrupt other actions while drilling happens
	bool	canDrill;
	int		drillStartPosX;
	int		drillStartPosY;

	GLfloat   speed;
	glm::vec3 acceleration;
};