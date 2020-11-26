// Regular Libraries
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <math.h>
// GL Libraries
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Other Util Libraries
#include <SOIL/SOIL.h>
#include <chrono>
#include <thread>

// User-defined Libraries
#include "Shader.h"
#include "common.h"
#include "Window.h"
#include "PlayerGameObject.h"
#include "Boss.h"
#include "Graph.h"
#include "Node.h"
#include "FileUtils.h"

// Macro for printing exceptions
#define PrintException(exception_object)\
	std::cerr << exception_object.what() << std::endl

/*====================================================================================
								       GLOBALS
====================================================================================*/

// Globals that define the OpenGL window and viewport
const std::string window_title_g = "Planet Prober - COMP 2501";

extern int window_width_g  = 800;
extern int window_height_g = 600;
extern float cameraZoom    = 0.2f; //0.2
extern float aspectRatio   = (float)window_height_g / (float)window_width_g;

glm::vec3 background_colour = glm::vec3(1.0f);

// Global mouse position
extern int mouseX = 0;
extern int mouseY = 0;
extern bool mousePressed = false;

// Global texture info
GLuint playerTextures[9];
GLuint enemyTextures[6];
GLuint mapTextures[5];
GLuint uiTextures[10];
GLuint bossTextures[4];
GLuint numberTextures[10];
GLuint miscTextures[5];

// Global game info
std::string gameState = "titleScreen";
Graph* gameworld;
std::vector<std::vector<int>> map;
bool levelIsUnlocked[5] = { true, true, true, true, false };
bool levelComplete[5] = { false, false, false, false, false };

float shopCosts[3] = { 100, 100, 0 };
int healthUpgraded;
int fuelUpgraded;

bool gamePaused;
float pauseCooldown;

// Global object info
PlayerGameObject* player;
Boss* boss;
GameObject* wallTile;

/*====================================================================================
								   TEXTURE SETUP
====================================================================================*/

// Create the geometry for a square (with two triangles)
// Return the number of array elements that form the square
int CreateSquare(void) {

	GLfloat vertex[]  = {
		//  square (two triangles)
		//  Position     Color                  Texcoords
		-0.5f, 0.5f,	 1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // Top-left
		0.5f, 0.5f,		 0.0f, 1.0f, 0.0f,		1.0f, 0.0f, // Top-right
		0.5f, -0.5f,	 0.0f, 0.0f, 1.0f,		1.0f, 1.0f, // Bottom-right
		-0.5f, -0.5f,	 1.0f, 1.0f, 1.0f,		0.0f, 1.0f  // Bottom-left
	};

	// The face of the square is defined by four vertices and two triangles
	GLuint face[] = {
		0, 1, 2,
		2, 3, 0
	};

	GLuint vbo, ebo;

	// Create buffer for vertices
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// Create buffer for faces (index buffer)
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face), face, GL_STATIC_DRAW);

	// Return number of elements in array buffer (6 in this case)
	return sizeof(face) / sizeof(GLuint);
}

void setthisTexture(GLuint w, char *fname){
	glBindTexture(GL_TEXTURE_2D, w);

	int width, height;
	unsigned char* image = SOIL_load_image(fname, &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	// Texture Wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Texture Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void setallTextures(void) {
	glGenTextures(9, playerTextures);
	glGenTextures(6, enemyTextures);
	glGenTextures(5, mapTextures);
	glGenTextures(10, uiTextures);
	glGenTextures(4, bossTextures);
	glGenTextures(10, numberTextures);
	glGenTextures(5, miscTextures);

	setthisTexture(playerTextures[0], "player.png");		//1
	setthisTexture(playerTextures[1], "bullet.png");		//2
	setthisTexture(playerTextures[2], "drill.png");			//3
	setthisTexture(playerTextures[3], "machinegun.png");	//4
	setthisTexture(playerTextures[4], "grenadelaunch.png");	//5
	setthisTexture(playerTextures[5], "grenade.png");		//6
	setthisTexture(playerTextures[6], "fire.png");			//7
	setthisTexture(playerTextures[7], "flamethrower.png");	//8
	setthisTexture(playerTextures[8], "cryolator.png");		//9

	setthisTexture(enemyTextures[0], "turretEnemy.png");	//10
	setthisTexture(enemyTextures[1], "movingEnemy.png");	//11
	setthisTexture(enemyTextures[2], "movingBarrel.png");	//12
	setthisTexture(enemyTextures[3], "turretBarrel.png");	//13
	setthisTexture(enemyTextures[4], "insectBody.png");		//14
	setthisTexture(enemyTextures[5], "insectHead.png");		//15

	setthisTexture(mapTextures[0], "emptyTile.png");		//16
	setthisTexture(mapTextures[1], "wallTile.png");			//17
	setthisTexture(mapTextures[2], "digTile.png");			//18
	setthisTexture(mapTextures[3], "powerup.png");			//19
	setthisTexture(mapTextures[4], "fuelStation.png");		//20

	setthisTexture(uiTextures[0], "menuscreen.png");		//21
	setthisTexture(uiTextures[1], "menuscreen-topbar.png"); //22
	setthisTexture(uiTextures[2], "lock.png");				//23
	setthisTexture(uiTextures[3], "titlescreen.png");		//24
	setthisTexture(uiTextures[4], "fuelIcon.png");			//25
	setthisTexture(uiTextures[5], "healthIcon.png");		//26
	setthisTexture(uiTextures[6], "statusBar.png");			//27
	setthisTexture(uiTextures[7], "shopscreen.png");		//28
	setthisTexture(uiTextures[8], "scrapIcon.png");			//29
	setthisTexture(uiTextures[9], "pauseScreen.png");		//30
															
	setthisTexture(bossTextures[0], "player.png");			//31
	setthisTexture(bossTextures[1], "healthbar.png");		//32
	setthisTexture(bossTextures[2], "indicator.png");		//33
	setthisTexture(bossTextures[3], "none.png");			//34

	setthisTexture(numberTextures[0], "zero.png");			//35
	setthisTexture(numberTextures[1], "one.png");			//36
	setthisTexture(numberTextures[2], "two.png");			//37
	setthisTexture(numberTextures[3], "three.png");			//38
	setthisTexture(numberTextures[4], "four.png");			//39
	setthisTexture(numberTextures[5], "five.png");			//40
	setthisTexture(numberTextures[6], "six.png");			//41
	setthisTexture(numberTextures[7], "seven.png");			//42
	setthisTexture(numberTextures[8], "eight.png");			//43
	setthisTexture(numberTextures[9], "nine.png");			//44
	
	setthisTexture(miscTextures[0], "planetCore.png");		//45

	glBindTexture(GL_TEXTURE_2D, playerTextures[0]);
}

/*====================================================================================
						    SETUP - Helper Function
====================================================================================*/
void setup() {

	// Set up z-buffer for rendering
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Enable Alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int numElements = 6;
	player = new PlayerGameObject(glm::vec3(0.0f, 0.0f, 0.0f), playerTextures[0], numElements);
	boss = new Boss(glm::vec3(14.0f, -46.0f, 0.0f), numElements);
}

/*====================================================================================
						  IMAGE RENDER - Helper Function
====================================================================================*/
void renderImage(Shader& shader, GLuint texture, glm::vec3 position, glm::vec3 scale) {

	shader.enable();
	shader.SetAttributes_sprite();

	// Render the you background image
	glBindTexture(GL_TEXTURE_2D, texture);

	// Setup the transformation matrix for the shader
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(), scale);
	glm::mat4 transMatrix = glm::translate(glm::mat4(), position);

	// Set the transformation matrix in the shader
	glm::mat4 transformationMatrix = transMatrix * scaleMatrix;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);

	// Draw the image
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

/*====================================================================================
						     LOAD LEVEL - Helper Function
====================================================================================*/
void loadLevel(int levelNum) {
	// Load the csv file based on the selected level
	std::string fname = "level" + std::to_string(levelNum) + ".csv";
	map = FileUtils::LoadCSVFile(fname.c_str());

	// Create wall tile object
	wallTile = new GameObject(glm::vec3(0.0f), mapTextures[0], 6);

	// Create an array of nodes based on this array of ints
	gameworld = new Graph(*wallTile, map);
}

/*====================================================================================
							  RENDER NUMBERS - Helper Function
====================================================================================*/
void renderNumbers(GLuint number, glm::vec3 startPos, Shader& shader, glm::vec3 scale, float spacing) {
	int digits[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 ,-1 };
	int i = 0;
	while (number > 0) {
		digits[i] = number % 10;
		number /= 10;
		i++;
	}
	glm::vec3 digitPos = startPos;
	for (int i = 9; i >= 0; i--) {
		if (digits[i] != -1) {
			renderImage(shader, numberTextures[digits[i]], digitPos, scale);
			digitPos.x += spacing;
		}
	}
}

/*====================================================================================
							 RENDER UI - Helper Function
====================================================================================*/
void renderUI(Shader& shader) {
	glm::vec3 pos = player->getPosition();

	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	// RENDER HEALTH ICON
	renderImage(shader, uiTextures[4], glm::vec3((pos.x - 3.0f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(1.0f));
	// RENDER FUEL ICON
	renderImage(shader, uiTextures[5], glm::vec3((pos.x - 6.0f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(1.0f));
	// RENDER SCRAP ICON
	renderImage(shader, uiTextures[8], glm::vec3((pos.x) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(1.0f));

	// RENDER HEALTH BAR
	if (player->health > 0) {
		float playerHealthPercent = player->health / player->maxHealth;
		shader.setUniform3f("colorMod", glm::vec3(-1.0f, 1.0f, -1.0f));
		renderImage(shader, uiTextures[6], glm::vec3((pos.x - 4.5f - (1 - playerHealthPercent)) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(2.0f * playerHealthPercent, 0.8f, 0.0f));
	}
	
	// RENDER FUEL BAR
	float playerFuelPercent = player->fuel / player->maxFuel;
	shader.setUniform3f("colorMod", glm::vec3(1.0f, 0.5f, -1.0f));
	renderImage(shader, uiTextures[6], glm::vec3((pos.x - 1.5f - (1 - playerFuelPercent)) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(2.0f * playerFuelPercent, 0.8f, 0.0f));

	// RENDER SCRAP VALUE
	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	renderNumbers(player->scrap, glm::vec3((pos.x + 0.8f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), shader, glm::vec3(0.5f, 0.8f, 1.0f), 0.5f);

	// RENDER AVAILABLE WEAPONS
	for (int i = 0; i < player->availableWeapons.size(); i++) {
		if (player->availableWeapons[i] == "machineGun") {
			renderImage(shader, playerTextures[3], glm::vec3((pos.x + 3.2f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(0.7f, 0.2f, 0.0f));
			if (i == player->selectedWeaponIndex) {
				renderImage(shader, uiTextures[6], glm::vec3((pos.x + 3.2f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(0.75f, 0.3f, 0.0f));
			}
		}
		else if (player->availableWeapons[i] == "grenadeLauncher") {
			renderImage(shader, playerTextures[4], glm::vec3((pos.x + 4.2f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(0.8f, 0.3f, 0.0f));
			if (i == player->selectedWeaponIndex) {
				renderImage(shader, uiTextures[6], glm::vec3((pos.x + 4.2f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(0.85f, 0.35f, 0.0f));
			}
		}
		else if (player->availableWeapons[i] == "flameThrower") {
			renderImage(shader, playerTextures[7], glm::vec3((pos.x + 5.2f ) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(0.8f, 0.3f, 0.0f));
			if (i == player->selectedWeaponIndex) {
				renderImage(shader, uiTextures[6], glm::vec3((pos.x + 5.2f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(0.85f, 0.35f, 0.0f));
			}
		}
		else if (player->availableWeapons[i] == "cryolator") {
			renderImage(shader, playerTextures[8], glm::vec3((pos.x + 6.2f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(0.8f, 0.3f, 0.0f));
			if (i == player->selectedWeaponIndex) {
				renderImage(shader, uiTextures[6], glm::vec3((pos.x + 6.2f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(0.85f, 0.35f, 0.0f));
			}
		}
	}
	// Render back panel to the available weapons
	shader.setUniform3f("colorMod", glm::vec3(0.0f));
	renderImage(shader, uiTextures[6], glm::vec3((pos.x + 5.0f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(4.45f, 0.75f, 0.0f));
	shader.setUniform3f("colorMod", glm::vec3(1.0f));
	renderImage(shader, uiTextures[6], glm::vec3((pos.x + 5.0f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(4.5f, 0.8f, 0.0f));


	// RENDER BACK PANEL
	shader.setUniform3f("colorMod", glm::vec3(0.0f, 0.0f, 0.0f));
	renderImage(shader, uiTextures[6], glm::vec3(pos.x * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(window_width_g * cameraZoom / 12, 1.0f, 0.0f));
	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
}

/*====================================================================================
							  LEVEL SELECTION - Game State
====================================================================================*/
void levelSelect(Shader& shader) {
	glm::vec3 pos = player->getPosition();
	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));

	int numLevels = 6;
	// Define five locations for clickable buttons (five levels)
	//                      EARTH        MARS         JUPITER       NEPTUNE       SATURN		SHOP
	int mButtonPos[6][3] = { {95,185,85},{380,175,80},{690,205,120},{280,425,100},{625,495,85},{760,40,40} };
	
	//Check for Level 5 unlock condition
	if (levelComplete[0] && levelComplete[1] && levelComplete[2] && levelComplete[3] && !levelComplete[4]) {
		levelIsUnlocked[4] = true;
	}
	//Level select mouse detection
	bool highlight = false;
	for (int level = 0; level < numLevels; level++) {
		if (pow((mouseX - mButtonPos[level][0]), 2) + pow((mouseY - mButtonPos[level][1]), 2) <= pow(mButtonPos[level][2], 2)) {
			if (mousePressed) {
				int currentLevel = level + 1;
				if (currentLevel == 1 && levelIsUnlocked[level]) {
					gameState = "levelOne";
				}
				else if (currentLevel == 2 && levelIsUnlocked[level]) {
				gameState = "levelTwo";
				}
				else if (currentLevel == 3 && levelIsUnlocked[level]) {
				gameState = "levelThree";
				}
				else if (currentLevel == 4 && levelIsUnlocked[level]) {
				gameState = "levelFour";
				}
				else if (currentLevel == 5 && levelIsUnlocked[level]) {
				gameState = "levelFive";
				}
				if (currentLevel == 6) {
					gameState = "shopScreen";
				}
				if (gameState != "levelSelect" && gameState != "shopScreen") {
					loadLevel(currentLevel);
					player->health = player->maxHealth;
					player->fuel = player->maxFuel;
					player->setPosition(glm::vec3(12.0f, 16.0f, 0.0f));
					boss = new Boss(glm::vec3(14.0f, -46.0f, 0.0f), 6);
					break;
				}
			}
			shader.setUniform1f("drawLevelSelectHighlight", mButtonPos[level][2]);
			shader.setUniform2f("offset", glm::vec2(mButtonPos[level][0] / 800.0f, mButtonPos[level][1] / 600.0f));
			highlight = true;
		}
		if (levelIsUnlocked[level] == false && level != 5) {
			// Render lock symbol
			float lock_x = 6.6f*(((mButtonPos[level][0]) / (float)(window_width_g / 2.0f)) - 1.0f);
			float lock_y = 5.0f*(1.0f - ((mButtonPos[level][1]) / (float)(window_height_g / 2.0f)));
			renderImage(shader, uiTextures[2], glm::vec3(lock_x, lock_y, 0.0f), glm::vec3(3.5f,3.0f,0.0f));
		}
	}
	if (!highlight) {
		shader.setUniform1i("drawLevelSelectHighlight", 0.0f);
		shader.setUniform2f("offset", glm::vec2(-800, -600));
	}

	// RENDER SCRAP ICON
	renderImage(shader, uiTextures[8], glm::vec3((pos.x) * cameraZoom * 5, (pos.y + 4.4f) * cameraZoom * 5, pos.z), glm::vec3(1.0f));
	// RENDER SCRAP VALUE
	renderNumbers(player->scrap, glm::vec3((pos.x + 0.8f) * cameraZoom * 5, (pos.y + 4.4f) * cameraZoom * 5, pos.z), shader, glm::vec3(0.5f, 0.8f, 1.0f), 0.5f);

	// Render Level Selection Screen
	renderImage(shader, uiTextures[0], glm::vec3(0.0f), glm::vec3(window_width_g * cameraZoom / 12, window_height_g * cameraZoom / 12, 0.0f));
	// Render menu top bar
	renderImage(shader, uiTextures[1], glm::vec3(0.0f), glm::vec3(window_width_g * cameraZoom / 12, window_height_g * cameraZoom / 12, 0.0f));


}

/*====================================================================================
								 TITLE SCREEN - Game State
====================================================================================*/
void titleScreen(Shader& shader) {
	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	renderImage(shader, uiTextures[3], glm::vec3(0.0f), glm::vec3(window_width_g * cameraZoom / 12, window_height_g * cameraZoom / 12, 0.0f));

	// Define a clickable area for the PLAY button
	int mButtonPos[2] = { 333, 300 };

	// Mouse pos and click detection
	if (mButtonPos[0] < mouseX && mouseX < mButtonPos[0] + 127 && mButtonPos[1] < mouseY && mouseY < mButtonPos[1] + 47) {
		if (mousePressed) {
			gameState = "levelSelect";
		}
	}
}

/*====================================================================================
									  SHOP
====================================================================================*/
void shop(Shader& shader) {
	glm::vec3 pos = player->getPosition();

	// Define a clickable area for the buttons
	//							HP		FUEL	  BACK BUTTON
	int mButtonPos[3][2] = { {496,222},{496,311},{600,8} }; //corner coords for clickables

	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	// Mouse pos and click detection HEALTH
	//increment base health and take away appropriate amt of currency
	if (player->scrap >= shopCosts[0]) {
		if (mButtonPos[0][0] < mouseX && mouseX < mButtonPos[0][0] + 28 && mButtonPos[0][1] < mouseY && mouseY < mButtonPos[0][1] + 28) {
			if (mousePressed && healthUpgraded < 4) {
				player->scrap -= shopCosts[0];
				player->maxHealth += 50;
				shopCosts[0] *= 2.5;
				healthUpgraded++;
			}
		}
	}
	else {
		shader.setUniform3f("colorMod", glm::vec3(1.0f, 0.0f, 0.0f));
	}
	// RENDER HEALTH COST
	renderNumbers(shopCosts[0], glm::vec3((pos.x + 0.5f) * cameraZoom * 5, (pos.y + 1.1f) * cameraZoom * 5, pos.z), shader, glm::vec3(0.2f, 0.4f, 1.0f), 0.2f);

	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	// Mouse pos and click detection FUEL
	//increment fuel and take away appropriate amt of currency
	if (player->scrap >= shopCosts[1]) {
		if (mButtonPos[1][0] < mouseX && mouseX < mButtonPos[1][0] + 28 && mButtonPos[1][1] < mouseY && mouseY < mButtonPos[1][1] + 28) {
			if (mousePressed && fuelUpgraded < 4) {
				player->scrap -= shopCosts[1];
				player->maxFuel += 50;
				shopCosts[1] *= 2.1;
				fuelUpgraded++;
			}
		}
	}
	else {
		shader.setUniform3f("colorMod", glm::vec3(1.0f, 0.0f, 0.0f));
	}
	// RENDER FUEL COST
	renderNumbers(shopCosts[1], glm::vec3((pos.x + 0.5f) * cameraZoom * 5, (pos.y - 0.4f) * cameraZoom * 5, pos.z), shader, glm::vec3(0.2f, 0.4f, 1.0f), 0.2f);


	if (mButtonPos[2][0] < mouseX && mouseX < mButtonPos[2][0] + 190 && mButtonPos[2][1] < mouseY && mouseY < mButtonPos[2][1] + 72) {
		if (mousePressed) {
			gameState = "levelSelect";
		}
	}
	
	shader.setUniform3f("colorMod", glm::vec3(0.0f, 1.0f, 0.0f));

	for (int i = 0; i < healthUpgraded; i++) {
		renderImage(shader, uiTextures[6], glm::vec3((pos.x - 0.69f + (i + 0) * 0.65f) * cameraZoom * 5, (pos.y + 0.41f) * cameraZoom * 5, pos.z), glm::vec3(0.57, 0.37f, 0.0f));
	}

	for (int j = 0; j < fuelUpgraded; j++) {
		renderImage(shader, uiTextures[6], glm::vec3((pos.x - 0.69f + (j + 0) * 0.65f) * cameraZoom * 5, (pos.y - 0.95f) * cameraZoom * 5, pos.z), glm::vec3(0.57, 0.37f, 0.0f));
	}

	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	// RENDER SCRAP ICON
	renderImage(shader, uiTextures[8], glm::vec3((pos.x - 6.0f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), glm::vec3(1.0f));
	// RENDER SCRAP VALUE
	renderNumbers(player->scrap, glm::vec3((pos.x - 5.2f) * cameraZoom * 5, (pos.y + 4.5f) * cameraZoom * 5, pos.z), shader, glm::vec3(0.5f, 0.8f, 1.0f), 0.5f);

	//render shop "GUI"
	renderImage(shader, uiTextures[7], glm::vec3(0.0f), glm::vec3(window_width_g * cameraZoom / 12 + 5, window_height_g * cameraZoom / 12, 0.0f));
}


/*====================================================================================
									  PAUSE MENU
====================================================================================*/
void pauseMenu(Shader& shader) {
	glm::vec3 pos = player->getPosition();

	// Define a clickable area for the buttons
	//						  RETURN    MAINMENU  QUIT
	int mButtonPos[3][2] = { {365,250},{340,300},{380,355} }; //corner coords for clickables

	// Mouse hover over RETURN button
	if (mButtonPos[0][0] < mouseX && mouseX < mButtonPos[0][0] + 95 && mButtonPos[0][1] < mouseY && mouseY < mButtonPos[0][1] + 25) {
		if (mousePressed) { //Click button
			gamePaused = false;
		}
	}

	// Mouse hover over MAIN MENU button
	if (mButtonPos[1][0] < mouseX && mouseX < mButtonPos[1][0] + 140 && mButtonPos[1][1] < mouseY && mouseY < mButtonPos[1][1] + 22) {
		if (mousePressed) { //Click button
			gamePaused = false;
			gameState = "levelSelect";
			player->setPosition(glm::vec3(0.0f));
		}
	}

	// Mouse hover over QUIT button
	if (mButtonPos[2][0] < mouseX && mouseX < mButtonPos[2][0] + 60 && mButtonPos[2][1] < mouseY && mouseY < mButtonPos[2][1] + 25) {
		if (mousePressed) { //Click button
			exit(0);
		}
	}

	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	// Render pause menu GUI
	renderImage(shader, uiTextures[9], glm::vec3((pos.x) * cameraZoom * 5, (pos.y - 1.0f) * cameraZoom * 5, pos.z), glm::vec3(window_width_g * cameraZoom / 12 + 5, window_height_g * cameraZoom / 12, 0.0f));
}

void deathScreen(Shader& shader) {
	glm::vec3 pos = player->getPosition();
	setthisTexture(uiTextures[9], "deathScreen.png");

	// Define a clickable area for the buttons
	//						  MAINMENU
	int mButtonPos[1][2] = { {340,300} }; //corner coords for clickables

	// Mouse hover over MAIN MENU button
	if (mButtonPos[0][0] < mouseX && mouseX < mButtonPos[0][0] + 140 && mButtonPos[0][1] < mouseY && mouseY < mButtonPos[0][1] + 22) {
		if (mousePressed) { //Click button
			gamePaused = false;
			gameState = "levelSelect";
			setthisTexture(uiTextures[9], "pauseScreen.png");
			player->scrap = round(player->scrap / 2);
			player->setPosition(glm::vec3(0.0f));
		}
	}

	shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	// Render pause menu GUI
	renderImage(shader, uiTextures[9], glm::vec3((pos.x) * cameraZoom * 5, (pos.y - 1.0f) * cameraZoom * 5, pos.z), glm::vec3(window_width_g * cameraZoom / 12 + 5, window_height_g * cameraZoom / 12, 0.0f));

}

/*====================================================================================
								   LEVEL - Game State
====================================================================================*/
void level(Shader& shader, Shader& grenadeShader, Shader& fireShader, Shader& iceShader, double deltaTime, int level) {

	glm::vec3 pos = player->getPosition();
	glm::vec3 cameraPos = glm::vec3(-pos.x * cameraZoom * 5, -pos.y * cameraZoom * 5, pos.z);
	glm::mat4 cameraTranslatePos = glm::translate(glm::mat4(1.0f), cameraPos);
	glm::mat4 windowScale = glm::scale(glm::mat4(1.0f), glm::vec3(aspectRatio, 1.0f, 1.0f));
	glm::mat4 cameraScale = glm::scale(glm::mat4(1.0f), glm::vec3(cameraZoom, cameraZoom, cameraZoom));
	glm::mat4 viewMatrix = windowScale * cameraScale * cameraTranslatePos;

	std::vector<EnemyGameObject*>* enemies = gameworld->getEnemies();
	std::vector<GameObject*>* powerups = gameworld->getPowerUps();
	std::vector<GameObject*>* fuelstations = gameworld->getFuelStations();

	if (glfwGetKey(Window::getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS && pauseCooldown <= 0) {
		gamePaused = !gamePaused;
		pauseCooldown = 10.0f * deltaTime;
	}
	else {
		pauseCooldown -= deltaTime;
	}
	if (player->health <= 0) {
		gamePaused = true;
	}

	// Render UI
	renderUI(shader);
	if (!gamePaused) {
		// UPDATE
		// Update Player
		player->update(deltaTime, map, gameworld, boss);
		// Update Enemies
		for (int i = 0; i < enemies->size(); i++) {
			int playerDmg = 0;
			(*enemies)[i]->update(deltaTime, map, player->getPosition(), playerDmg);
			if (!player->poweredUp) {
				player->health -= playerDmg;
			}

			if ((*enemies)[i]->enemyType == 8 && (*enemies)[i]->getHealth() <= 0) {
				levelComplete[level - 1] = true;
				levelIsUnlocked[level - 1] = false;
				if (level == 1) {
					player->availableWeapons.push_back("grenadeLauncher");
					std::cout << "grenadeLauncher!" << std::endl;
				}
				else if (level == 2) {
					player->availableWeapons.push_back("flameThrower");
					std::cout << "flameThrower!" << std::endl;
				}
				else if (level == 4) {
					player->availableWeapons.push_back("cryolator");
					std::cout << "cryolator!" << std::endl;
				}
				gameState = "levelSelect";
				player->setPosition(glm::vec3(0.0f));
			}
		}
		// Update Powerups
		for (int i = 0; i < powerups->size(); i++) {
			(*powerups)[i]->checkPowerUps(deltaTime, player);
		}
		// Update fuel stations
		for (int i = 0; i < fuelstations->size(); i++) {
			int fuelValue = (*fuelstations)[i]->checkFuelStations(deltaTime, player);
			if (rand()%3 == 0 && player->fuel < player->maxFuel) {
				player->fuel += fuelValue;
				player->scrap -= fuelValue;
			}
			(*fuelstations)[i]->render(shader, glm::vec3(2.0f));
		}

		// Update Boss
		int playerDmg = 0;
		boss->update(deltaTime, map, gameworld, player->getPosition(), playerDmg);
		if (!player->poweredUp) {
			player->health -= playerDmg;
		}
		// Update Map
		gameworld->update(deltaTime, boss->getPosition(), player->getPosition());

	}
	else {
		if (player->health <= 0) {
			deathScreen(shader);
		}
		else {
			pauseMenu(shader);
		}
	}


	// RENDER
	// Render Player
	player->render(shader, grenadeShader, fireShader, iceShader, deltaTime, viewMatrix);
	// Render Enemies
	for (int i = 0; i < enemies->size(); i++) {
		(*enemies)[i]->render(shader, grenadeShader, fireShader, deltaTime, viewMatrix);
	}
	// Render Powerups
	for (int i = 0; i < powerups->size(); i++) {
		if ((*powerups)[i]->exists) {
			(*powerups)[i]->render(shader, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if ((*powerups)[i]->obtained) {
			powerups->erase(powerups->begin() + i);
		}
	}
	// Render Boss
	boss->render(shader, grenadeShader, deltaTime, player->getPosition(), viewMatrix);
	// Render Map
	gameworld->render(shader, player->getPosition(), map);
}

/*====================================================================================
								        MAIN
====================================================================================*/
int main(void) {
	try {

		// Setup window
		Window window(window_width_g, window_height_g, window_title_g);
		// Set up shaders
		Shader shader("shader.vert", "shader.frag", true, 0);
		Shader grenadeShader("grenade_shader.vert", "grenade_shader.frag", false, 1);
		Shader fireShader("fire_shader.vert", "fire_shader.frag", false, 2);
		Shader iceShader("ice_shader.vert", "ice_shader.frag", false, 3);

		// Setup objects
		setup();

		// Set up the textures
		setallTextures();

		// Run the main loop
		double lastTime = glfwGetTime();
		while (!glfwWindowShouldClose(window.getWindow())) {

			// Clear background
			window.clear(background_colour);

			// Calculate delta time
			double currentTime = glfwGetTime();
			double deltaTime = currentTime - lastTime;
			lastTime = currentTime;

			// Setup camera to focus on the player
			glm::vec3 pos = player->getPosition();

			glm::mat4 cameraTranslatePos = glm::translate(glm::mat4(1.0f), glm::vec3(-pos.x * cameraZoom * 5, -pos.y * cameraZoom * 5, pos.z));
			glm::mat4 windowScale = glm::scale(glm::mat4(1.0f), glm::vec3(aspectRatio, 1.0f, 1.0f));
			glm::mat4 cameraScale = glm::scale(glm::mat4(1.0f), glm::vec3(cameraZoom, cameraZoom, cameraZoom));
			glm::mat4 viewMatrix = windowScale * cameraScale * cameraTranslatePos;
			shader.setUniformMat4("viewMatrix", viewMatrix);

			//======FINITE STATE MACHINE======//
			if (gameState == "titleScreen") {
				titleScreen(shader);
			}
			else if (gameState == "levelSelect") {
				levelSelect(shader);
				background_colour = glm::vec3(1.0f);
			}
			else if (gameState == "levelOne") {
				level(shader, grenadeShader, fireShader, iceShader, deltaTime, 1);
				gameworld->setLevel(1);
				background_colour = glm::vec3(-0.15f, -0.01f, 0.57f) + glm::vec3(0.8f);
			}
			else if (gameState == "levelTwo") {
				level(shader, grenadeShader, fireShader, iceShader, deltaTime, 2);
				gameworld->setLevel(2);
				background_colour = glm::vec3(0.22f, -0.27f, -0.58f) + glm::vec3(0.8f);
			}
			else if (gameState == "levelThree") {
				level(shader, grenadeShader, fireShader, iceShader, deltaTime, 3);
				gameworld->setLevel(3);
				background_colour = glm::vec3(0.09f, 0.25f, 0.30f) + glm::vec3(0.8f);
			}
			else if (gameState == "levelFour") {
				level(shader, grenadeShader, fireShader, iceShader, deltaTime, 4);
				gameworld->setLevel(4);
				background_colour = glm::vec3(0.0f);
			}
			else if (gameState == "levelFive") {
				level(shader, grenadeShader, fireShader, iceShader, deltaTime, 5);
				gameworld->setLevel(5);
				background_colour = glm::vec3(0.48f, 0.48f, -0.25f) + glm::vec3(0.8f);
			}
			else if (gameState == "shopScreen") {
				shop(shader);
			}


			mousePressed = false;
			// Update other events like input handling
			glfwPollEvents();
			// Push buffer drawn in the background onto the display
			glfwSwapBuffers(window.getWindow());
		}
	}
	catch (std::exception &e){
		// print exception and sleep so error can be read
		PrintException(e);
		std::this_thread::sleep_for(std::chrono::milliseconds(100000));
	}

	return 0;
}
