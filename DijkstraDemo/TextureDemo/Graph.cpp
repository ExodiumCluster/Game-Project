#include "Graph.h"

#include <iostream>	//used for debug purposes
#include <GL/glew.h> // window management library
#include <GL/glfw3.h>

using namespace std;

extern int window_width_g;
extern int window_height_g;
extern float cameraZoom;
extern float aspectRatio;


//class used to compare 2 nodeTuple structs

class compareNode
{
public:
	int operator() (const QNode& n1, const QNode& n2)
	{
		return n1.cost > n2.cost;
	}
};

//main constructor
//takes the width, height of graph, as well as a gameobject used to render each node.
Graph::Graph(GameObject nodeSprite, std::vector<std::vector<int>> map) : nodeObj(nodeSprite) {
	//initializes the 2d nodes array and nodeMap
	Node::resetNodeCount();

	std::vector<std::vector<Node*>*> nodes;
	nodeMap = std::map<int, Node*>();
	//data for setting node positions on screen.
	float movementX = 2.0;
	float movementY = -2.0f;
	int nodeWidth = map.size();
	int nodeHeight = map[0].size();
	
	currentLevel = 0;

	//sometimes missing nodes in center because of the rounding to nearest whole, this shouldn't be a problem in practice because you'll define your own graphs
	// you could round these vals to fix it but then its not centered anymore, but you'll probably just change both of these to 0 for your project anyway.
	start_x = -0.5f*nodeWidth; //movementX * -0.5 - 0.5f;
	start_y = 0.5f*nodeHeight; //movementY * -0.5 + 0.5f;
	//fills the 2d nodes array with nodes.
	for (int i = 0; i < nodeHeight; i++) {
		std::vector<Node*>* nodeRow = new std::vector<Node*>();

		for (int j = 0; j < nodeWidth; j++) {
			//creates each node, starting at (start_x, start_y), topLeft, going down, right
			Node* newNode = new Node(start_x + j* movementX, start_y + i* movementY);
			nodeRow->push_back(newNode);

		}
		nodes.push_back(nodeRow);
	}
	//connects node to each other to form a 4-connected graph with random edge weights
	for (int i = 0; i < nodes.size(); i++) {
		for (int j = 0; j < nodes.at(i)->size(); j++) {
			if (map[i][j] == WALL_TILE) {
				nodes.at(i)->at(j)->setType("wall");
			}
			if (map[i][j] == EMPTY_TILE) {
				nodes.at(i)->at(j)->setType("empty");
			}
			if (map[i][j] == BRK_TILE) {
				nodes.at(i)->at(j)->setType("breakable");
			}
			if (map[i][j] == TURRET_ENEMY_TILE) {
				nodes.at(i)->at(j)->setType("empty");
				glm::vec3 pos = glm::vec3(nodes.at(i)->at(j)->getX(), nodes.at(i)->at(j)->getY(),0.0f);
				enemies.push_back(new EnemyGameObject(pos, TURRET_ENEMY_TEX, TURRET_BARREL_TEX, 6, TURRET_ENEMY));
			}
			if (map[i][j] == MOVING_ENEMY_TILE) {
				nodes.at(i)->at(j)->setType("empty");
				glm::vec3 pos = glm::vec3(nodes.at(i)->at(j)->getX(), nodes.at(i)->at(j)->getY(), 0.0f);
				enemies.push_back(new EnemyGameObject(pos, MOVING_ENEMY_TEX, MOVING_BARREL_TEX, 6, MOVING_ENEMY));
			}
			if (map[i][j] == INSECT_ENEMY_TILE) {
				nodes.at(i)->at(j)->setType("empty");
				glm::vec3 pos = glm::vec3(nodes.at(i)->at(j)->getX(), nodes.at(i)->at(j)->getY(), 0.0f);
				enemies.push_back(new EnemyGameObject(pos, INSECT_ENEMY_TEX_BODY, TURRET_BARREL_TEX, 6, INSECT_ENEMY));
			}
			if (map[i][j] == POWERUP_TILE) {
				nodes.at(i)->at(j)->setType("empty");
				glm::vec3 pos = glm::vec3(nodes.at(i)->at(j)->getX(), nodes.at(i)->at(j)->getY(), 0.0f);
				powerups.push_back(new GameObject(pos, POWERUP_TEX, 6));
			}
			if (map[i][j] == FUEL_STATION_TILE) {
				nodes.at(i)->at(j)->setType("empty");
				glm::vec3 pos = glm::vec3(nodes.at(i)->at(j)->getX(), nodes.at(i)->at(j)->getY(), 0.0f);
				fuelstations.push_back(new GameObject(pos, FUEL_STATION_TEX, 6));
			}
			if (map[i][j] == PLANET_CORE_TILE) {
				nodes.at(i)->at(j)->setType("empty");
				glm::vec3 pos = glm::vec3(nodes.at(i)->at(j)->getX(), nodes.at(i)->at(j)->getY(), 0.0f);
				enemies.push_back(new EnemyGameObject(pos, 45, TURRET_BARREL_TEX, 6, PLANET_CORE));
			}

			//if there exists a node to the right of the current node, link them together
			int weight = 0;
			if (map[i][j] == WALL_TILE || map[i][j] == BRK_TILE) {
				weight = 9999;
			} 
			else {
				weight = 10;
			}
			if (j + 1 < nodes.at(i)->size()) {
				if (map[i][j] == EMPTY_TILE && (map[i][j + 1] == WALL_TILE || map[i][j + 1] == BRK_TILE)) {
					weight = 9999;
				}
				Node *n1 = nodes.at(i)->at(j);			//reference to current node in graph.
				Node *n2 = nodes.at(i)->at(j + 1);		//reference to node to the left of the current node.
				n1->addNode(*n2, weight);				//links both nodes together
			}

			//if there exists a node to the right of the current node, link them together
			if (j - 1 > 0) {
				if (map[i][j] == EMPTY_TILE && (map[i][j - 1] == WALL_TILE || map[i][j - 1] == BRK_TILE)) {
					weight = 9999;
				}
				Node* n1 = nodes.at(i)->at(j);			//reference to current node in graph.
				Node* n2 = nodes.at(i)->at(j - 1);		//reference to node to the left of the current node.
				n1->addNode(*n2, weight);				//links both nodes together
			}

			//if there exists a node below the current node, link them together
			if (i + 1 < nodes.size()) {
				if (map[i][j] == EMPTY_TILE && (map[i + 1][j] == WALL_TILE || map[i + 1][j] == BRK_TILE)) {
					weight = 9999;
				}
				Node *n1 = nodes.at(i)->at(j);			//reference to current node in graph.
				Node *n2 = nodes.at(i + 1)->at(j);		//node below the current node.
				n1->addNode(*n2, weight);				//links both nodes together
			}

			//if there exists a node below the current node, link them together
			if (i - 1 > 0) {
				if (map[i][j] == EMPTY_TILE && (map[i - 1][j] == WALL_TILE || map[i - 1][j] == BRK_TILE)) {
					weight = 9999;
				}
				Node* n1 = nodes.at(i)->at(j);			//reference to current node in graph.
				Node* n2 = nodes.at(i - 1)->at(j);		//node below the current node.
				n1->addNode(*n2, weight);				//links both nodes together
			}
		}
	}
	//adds all nodes to map with nodeId as key and a pointer to the node
	for (int i = 0; i < nodes.size(); i++) {
		for (int j = 0; j < nodes.at(i)->size(); j++) {
			nodeMap.insert(std::pair<int, Node*>(nodes.at(i)->at(j)->getId(), nodes.at(i)->at(j)));
		}
	}

	std::vector<Node*> tempVec;
	for (std::map<int, Node*>::iterator it = nodeMap.begin(); it != nodeMap.end(); ++it) {
		tempVec.push_back(it->second);
	}


	formatNodes(tempVec);
}


Graph::Graph(std::vector<Node*>& nodes, GameObject nodeSprite) : nodeObj(nodeSprite) {
	nodeMap = std::map<int, Node*>();
	nodeVec = std::vector<std::vector<Node*>*>();

	formatNodes(nodes);

	setStart(0);
	setEnd(nodeMap.size()-1);
}

//structures the nodes for later access
void Graph::formatNodes(std::vector<Node*>& nodes) {
	//adds all nodes to map with nodeId as key and a pointer to the node
	for (int i = 0; i < nodes.size(); i++) {
		nodes.at(i)->setX(round(nodes.at(i)->getX()));
		nodes.at(i)->setY(round(nodes.at(i)->getY()));
		nodeMap.insert(std::pair<int, Node*>(nodes.at(i)->getId(), nodes.at(i)));
	}

	std::sort(nodes.begin(), nodes.end(), Node::sortByY);
	minX = nodes.front()->getX();
	minY = nodes.front()->getY();

	float lastY = minY;

	Node* lastNode = NULL;
	for (std::vector<Node*>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
		if ((*it)->getX() < minX) {
			minX = (*it)->getX();
		}
		if ((*it)->getY() < minY) {
			minY = (*it)->getY();
		}
		std::vector<Node*>* tempVec = new std::vector<Node*>;
		if (lastNode != NULL) {
			tempVec->push_back(lastNode);
			lastY = lastNode->getY();
		}
		else {
			lastY = (*it)->getY();
		}
		while (it != nodes.end()) {
			if (lastY == (*it)->getY()) {
				tempVec->push_back(*it);
				lastY = (*it)->getY();
			}
			else {
				lastNode = *it;
				if (std::next(it, 1) == nodes.end()) {
					std::sort(tempVec->begin(), tempVec->end(), Node::sortByX);
					nodeVec.push_back(tempVec);
					tempVec = new std::vector<Node*>;
					tempVec->push_back(lastNode);
				}
				break;
			}
			++it;
		}

		std::sort(tempVec->begin(), tempVec->end(), Node::sortByX);

		//error checking
		Node* n = NULL;
		for(Node* ele : *tempVec) {
			if (n != NULL && n->getX() == ele->getX()) {
				std:: cout << "ERROR: NODES CANNOT SHARE COORDS: ID " << ele->getId() << " & " << n->getId();
				throw "ya done goof'd. Check console to see the problematic nodes ^.";
			}
			n = ele;
		}
		nodeVec.push_back(tempVec);
		if (it == nodes.end()) {
			break;
		}
	}
}

//these two searches are used to find the node the mouse is hovering on
int Graph::binarySearchY(std::vector<std::vector<Node*>*> vec, int l, int r, int dist){
	while (l <= r) {
		int m = l + (r - l) / 2;
		if (m < 0 || m >= vec.size()) {
			return -1;
		}
		// Check if x is present at mid
		float yVal = vec.at(m)->front()->getY();
		if (yVal - minY == dist) {
			return m;
		}
		// If x greater, ignore left half
		if (yVal - minY < dist) {
			l = m + 1;
		}
		// If x is smaller, ignore right half
		else {
			r = m - 1;
		}
	}
	// if we reach here, then element was
	// not present
	return -1;
}

int Graph::binarySearchX(std::vector<Node*>* vec, int l, int r, int dist){
	while (l <= r) {
		int m = l + (r - l) / 2;
		if (m < 0 || m >= vec->size()) {
			return -1;
		}
		// Check if x is present at mid
		float xVal = vec->at(m)->getX();
		if (xVal - minX == dist) {
			return m;
		}
		// If x greater, ignore left half
		if (xVal - minX < dist) {
			l = m + 1;
		}
		// If x is smaller, ignore right half
		else {
			r = m - 1;
		}
	}
	// if we reach here, then element was
	// not present
	return -1;
}



//Loops through array and prints out associated data for each node.
void Graph::printData() {
	for (int i = 0; i < nodeVec.size(); i++) {
		for (int j = 0; j < nodeVec.at(i)->size(); j++) {
			cout << i << ", " << j << " - " << nodeVec.at(i)->at(j)->getId() << " - " << &nodeVec.at(i)->at(j);
			cout << " - " << nodeVec.at(i)->at(j)->getEdges().size() << endl;
		}
	}
}

//gets mouse input, updates start and end position using that information
void Graph::update(double deltaTime, glm::vec3 startPos, glm::vec3 endPos) {
	int bossX = round(startPos.x / 2) + 8;
	int bossY = -1 * round(startPos.y / 2) + 8;

	setStart(bossX + 32 * bossY);

	int playerX = round(endPos.x / 2) + 8;
	int playerY = -1 * round(endPos.y / 2) + 8;

	setEnd(playerX + 32 * playerY);

}

//returns the id of the node at the mouse coordinates
int Graph::selectNode(double x, double y) {

	glfwGetWindowSize(Window::getWindow(), &window_width_g, &window_height_g);
	//if the mouse is outside the window, return -1
	if (x < 0 || x > window_width_g || y < 0 || y > window_height_g) {
		return -1;
	}
	else {
		float cursor_x_pos = ((x + 0.5f) / (float)(window_width_g / 2.0f)) - 1.0f;
		float cursor_y_pos = 1.0f - ((y + 0.5f) / (float)(window_height_g / 2.0f));

		cursor_x_pos /= (cameraZoom*aspectRatio); //only x is scaled by using the aspect ratio atm.
		cursor_y_pos /= cameraZoom;	//transforms cursor position based on screen scale. used to be const 0.2

		int x = round(cursor_x_pos - minX);
		int y = round(cursor_y_pos - minY);

		int yindex = binarySearchY(nodeVec, 0, nodeVec.size()-1, y);
		if (yindex < 0 || yindex >= nodeVec.size()) {
			return -1;
		}
		int xindex = binarySearchX(nodeVec.at(yindex), 0, nodeVec.at(yindex)->size()-1, x);
		if (xindex == -1) {
			return -1;
		}
		int tryId = nodeVec.at(yindex)->at(xindex)->getId();
		return tryId;
	}
}

//renders all the nodes in the graph
void Graph::render(Shader &shader, glm::vec3 playerPos, std::vector<std::vector<int>> map) {
	shader.enable();
	shader.SetAttributes_sprite();

	//goes through each node and renders it, using the provided gameObject

	float offset = map[0].size() / 4;
	int playerX = round(playerPos.x/2) + offset;
	int playerY = round(playerPos.y/2) - 2*offset;

	int leftEdge = playerX - 4;
	if (leftEdge < 0) { leftEdge = 0; }
	int rightEdge = playerX + 4;
	if (rightEdge > map[0].size() - 1) { rightEdge = map[0].size() - 1; }
	int topEdge = playerY - 4;
	if (topEdge < 0) { topEdge = 0; }
	int botEdge = playerY + 4;
	if (botEdge > map[0].size() - 1) { botEdge = map[0].size() - 1; }

	

	for (int j = topEdge; j <= botEdge; j++) {

		if (nodeVec.size() == 0) {	//if there are no nodes, do nothing.
			return;
		}

		//get the location of the color uniform
		GLint color_loc = glGetUniformLocation(shader.getShaderID(), "colorMod");

		for (int i = leftEdge; i <= rightEdge; i++) {
			//gets the current node to draw
			Node currentNode = *(nodeVec.at(j)->at(i));

			//set the node 'pen' to the position of the current node.
			nodeObj.setPosition(glm::vec3(currentNode.getX(), currentNode.getY(), 0.0f));

			//change the color uniform depending on if the node is the start or end node.
			if (currentNode.getType() == "wall" || currentNode.getType() == "breakable") {
				if (currentNode.getType() == "wall") { nodeObj.setTexture(WALL_TILE_TEX); }
				if (currentNode.getType() == "breakable") { nodeObj.setTexture(BRK_TILE_TEX); }
				Node* above = NULL;
				Node* below = NULL;
				Node* left  = NULL;
				Node* right = NULL;
				if (j > 0)
					above = (nodeVec.at(j-1)->at(i));
				if (j < map.size() - 1)
					below = (nodeVec.at(j+1)->at(i));
				if (i > 0)
					left = (nodeVec.at(j)->at(i-1));
				if (i < map[0].size() - 1)
					right = (nodeVec.at(j)->at(i+1));

				shader.setUniform3f("colorMod", glm::vec3(0.0f));

				if (above != NULL && above->getType() == "empty") {
					glBindTexture(GL_TEXTURE_2D, nodeObj.getTexture());
					glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), nodeObj.getPosition() + glm::vec3(0.0f, -1.0f, 0.0f));
					glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
					glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 0.1f, 1.0f));
					glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
					shader.setUniformMat4("transformationMatrix", transformationMatrix);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				} if (below != NULL && below->getType() == "empty") {
					glBindTexture(GL_TEXTURE_2D, nodeObj.getTexture());
					glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), nodeObj.getPosition() + glm::vec3(0.0f, 1.0f, 0.0f));
					glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
					glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 0.05f, 1.0f));
					glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
					shader.setUniformMat4("transformationMatrix", transformationMatrix);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				} if (left != NULL && left->getType() == "empty") {
					glBindTexture(GL_TEXTURE_2D, nodeObj.getTexture());
					glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), nodeObj.getPosition() + glm::vec3(-1.0f, 0.0f, 0.0f));
					glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
					glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 0.1f, 1.0f));
					glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
					shader.setUniformMat4("transformationMatrix", transformationMatrix);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				} if (right != NULL && right->getType() == "empty") {
					glBindTexture(GL_TEXTURE_2D, nodeObj.getTexture());
					glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), nodeObj.getPosition() + glm::vec3(1.0f, 0.0f, 0.0f));
					glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
					glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 0.05f, 1.0f));
					glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
					shader.setUniformMat4("transformationMatrix", transformationMatrix);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				}
			}
			if (currentNode.getType() == "empty") {
				nodeObj.setTexture(EMPTY_TILE_TEX);
			}

			//set the colorMod depending on which level is being played
			if (currentLevel == 1) {
				shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
			} else if (currentLevel == 2) {
				shader.setUniform3f("colorMod", glm::vec3(0.22f, -0.27f, -0.58f) + glm::vec3(0.8f));
			} else if (currentLevel == 3) {
				shader.setUniform3f("colorMod", glm::vec3(0.09f, 0.25f, 0.30f) + glm::vec3(0.8f));
			} else if (currentLevel == 4) {
				shader.setUniform3f("colorMod", glm::vec3(-0.15f, -0.01f, 0.57f) + glm::vec3(0.8f));
			} else if (currentLevel == 5) {
				shader.setUniform3f("colorMod", glm::vec3(0.48f, 0.48f, -0.25f) + glm::vec3(0.8f));
			}

			// Bind the entities texture
			glBindTexture(GL_TEXTURE_2D, nodeObj.getTexture());

			// Setup the transformation matrix for the shader
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), nodeObj.getPosition());
			glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));

			// Set the transformation matrix in the shader
			glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
			//transformationMatrix = rotationMatrix * translationMatrix  * scaleMatrix;
			shader.setUniformMat4("transformationMatrix", transformationMatrix);

			// Draw the entity
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
	}

	//set the colorMod depending on which level is being played
	if (currentLevel == 1) {
		shader.setUniform3f("colorMod", glm::vec3(1.0f, 1.0f, 1.0f));
	}
	else if (currentLevel == 2) {
		shader.setUniform3f("colorMod", glm::vec3(0.22f, -0.27f, -0.58f) + glm::vec3(0.8f));
	}
	else if (currentLevel == 3) {
		shader.setUniform3f("colorMod", glm::vec3(0.09f, 0.25f, 0.30f) + glm::vec3(0.8f));
	}
	else if (currentLevel == 4) {
		shader.setUniform3f("colorMod", glm::vec3(-0.15f, -0.01f, 0.57f) + glm::vec3(0.8f));
	}
	else if (currentLevel == 5) {
		shader.setUniform3f("colorMod", glm::vec3(0.48f, 0.48f, -0.25f) + glm::vec3(0.8f));
	}

	// Render an underground back plate
	float mapCenterX = nodeVec.at(16)->at(16)->getX();
	float mapCenterY = nodeVec.at(16)->at(16)->getY();
	nodeObj.setPosition(glm::vec3(mapCenterX, mapCenterY, 0.0f));
	nodeObj.setTexture(WALL_TILE_TEX);
	glBindTexture(GL_TEXTURE_2D, nodeObj.getTexture());
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), nodeObj.getPosition() + glm::vec3(0.0f, -6.0f, 0.0f));
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(72.0f, 74.0f, 1.0f));
	glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	shader.setUniformMat4("transformationMatrix", transformationMatrix);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

//returns a reference to the node with the supplied id.
Node& Graph::getNode(int id) {

	//because we also store the graph as a map with the id as the key, we can easily reference each node.
	return *nodeMap.at(id);
}

//using zombie-key based approach to Djikstra's algorithm, returns a vec with nodes on path from startNode to endNode inclusive
std::vector<Node*> Graph::pathfind(glm::vec3 startPos) {

	//priority queue used in pathfinding.
	//it is created using the NodeTuple struct with a min compare function called compareNode
	priority_queue < QNode, vector<QNode>, compareNode> pq;
	//sets the costs of all nodes to infinity. reset all nodes to be off-path
	pathNodes.clear();
	for (int i = 0; i < nodeVec.size(); i++) {
		for (int j = 0; j < nodeVec.at(i)->size(); j++) {
			nodeVec.at(i)->at(j)->setCost(INT_MAX);
			nodeVec.at(i)->at(j)->setOnPath(false);
		}
	}
	int bossX = round(startPos.x/2) + 8;
	int bossY = -1 * round(startPos.y/2) + 8;

	//The startnode is added to the pq with cost 0
	QNode startNode = {&getNode(bossX + 32*bossY), 0};
	pq.push(startNode);

	if (startNodeId == endNodeId) {
		return pathNodes;
	}

	//now that the pq is setup, we can start the algorithm
	//keep in mind that the as the QNode struct has a pointer to the corresponding node
	//some function calls will use pointer syntax (->)

	while (!pq.empty()) {
		//get the current lowest-cost node in pq
		QNode lowest = pq.top();

		//if the current node is the end node, done!
		if (lowest.node->getId() == endNodeId) {
			break;
		}

		//OPEN NODE
		vector<Edge> neighbours = lowest.node->getEdges();
		for (int i = 0; i < neighbours.size(); i++) {

			//compute cost to get to neighbouring node
			//cost = the cost to get the corrent node + cost to traverse the edge

			Node *n = &getNode(lowest.node->getOtherNode(neighbours.at(i)).getId());
			//Node *n = &(lowest.node->getOtherNode(neighbours.at(i)));

			int nodeCost = lowest.cost + neighbours.at(i).cost;

			//if current node cost is higher than calculated, update node, and add QNode to queue
			if (n->getCost() > nodeCost) {
				n->setCost(nodeCost);
				n->setPrev(lowest.node);

				QNode updatedNode = {n, nodeCost };
				pq.push(updatedNode);
			}
		}

		pq.pop();	//REMOVE NODE FROM QUEUE
	}

	//queue is done, go in reverse from END to START to determine path
	Node* currentNode = getNode(endNodeId).getPrev();

	//while the current node isn't null, or the end, mark the current node as on the path
	pathNodes.push_back(&getNode(endNodeId));
	while (currentNode != NULL && currentNode->getId() != startNodeId) {
		pathNodes.push_back(currentNode);
		currentNode->setOnPath(true);
		currentNode = currentNode->getPrev();
	}
	pathNodes.push_back(currentNode);
	std::reverse(pathNodes.begin(), pathNodes.end());

	return pathNodes;

}
