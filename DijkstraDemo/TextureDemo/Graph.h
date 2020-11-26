#pragma once

#include "Node.h"
#include "Shader.h"
#include "GameObject.h"
#include "EnemyGameObject.h"
#include "Window.h"
#include "common.h"

#include <queue>
#include <map>
#include <algorithm>

#define WALL_TILE			0
#define EMPTY_TILE			1
#define BRK_TILE			2
#define TURRET_ENEMY_TILE	3
#define MOVING_ENEMY_TILE	4
#define POWERUP_TILE		5
#define FUEL_STATION_TILE   6
#define INSECT_ENEMY_TILE   7
#define PLANET_CORE_TILE    8

#define EMPTY_TILE_TEX		16
#define WALL_TILE_TEX		17
#define BRK_TILE_TEX		18
#define POWERUP_TEX			19
#define FUEL_STATION_TEX	20

//struct used to rank various nodes in the priority_queue
struct QNode{
	Node *node;
	int cost;
};

class Graph {
public:
	//constructor.
	Graph(GameObject nodeSprite, std::vector<std::vector<int>> map);
	Graph(std::vector<Node*>&, GameObject nodeSprite);

	void formatNodes(std::vector<Node*>& nodes);

	//Loops through array and prints out associated data for each node.
	void printData();

	//gets mouse input, updates start and end position using that information
	void update(double deltaTime, glm::vec3 startPos, glm::vec3 endPos);

	//returns the id of a node from an x/y coordinate
	int selectNode(double x, double y);

	//returns a reference to the node with the supplied id.
	Node& getNode(int id);

	std::vector<std::vector<Node*>*>& getNodes() { return nodeVec; }

	//renders all the nodes in the graph
	void render(Shader &shader, glm::vec3 playerPos, std::vector<std::vector<int>> map);

	//creates and marks a path from start to end
	std::vector<Node*> pathfind(glm::vec3 startPos);

	//setters
	inline void setStart(int nodeId) { startNodeId = nodeId; }
	inline void setEnd(int nodeId) { endNodeId = nodeId; }
	inline void setLevel(int level) { currentLevel = level; }

	//getters
	inline int getStartId() { return startNodeId; }
	inline int getEndId() { return endNodeId; }
	inline std::vector<EnemyGameObject*>* getEnemies() { return &enemies; }
	inline std::vector<GameObject*>* getPowerUps() { return &powerups; }
	inline std::vector<GameObject*>* getFuelStations() { return &fuelstations; }


private:
	//node sprite used to draw each node.
	GameObject nodeObj;

	GLuint currentLevel;

	//id of start and endnode
	int startNodeId, endNodeId;
	float start_x;
	float start_y;
	//2d vector containing all the nodes in the graph
	//std::vector<std::vector<Node>> nodes;
	std::vector<std::vector<Node*>*> nodeVec;

	std::vector<Node*> pathNodes;

	int nodeWid;
	int size;
	//map used for easy access to nodes via id
	std::map<int, Node*> nodeMap;

	float minX;
	float minY;

	//array of enemies that exist in the level
	std::vector<EnemyGameObject*> enemies;
	//array of powerups that exist in the level
	std::vector<GameObject*> powerups;
	//array of fuel stations that exist in the level
	std::vector<GameObject*> fuelstations;

	int binarySearchY(std::vector<std::vector<Node*>*> vec, int l, int r, int dist);
	int binarySearchX(std::vector<Node*>* vec, int l, int r, int dist);
};
