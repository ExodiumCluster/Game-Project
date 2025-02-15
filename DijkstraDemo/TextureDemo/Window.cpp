#include "Window.h"
#include <stdexcept>
GLFWwindow* Window::window;
extern int window_width_g;
extern int window_height_g;
extern float aspectRatio;

Window::Window(const unsigned int windowWidth, const unsigned int windowHeight, const std::string &windowTitle) {
	// Initialize the window management library (GLFW)
	if (!glfwInit()) {
		throw(std::runtime_error(std::string("Could not initialize the GLFW library")));
	}

	// Create a window and its OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // Required or else the calculation to get cursor pos to screenspace will be incorrect
	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), NULL, NULL);
	if (!window) {
		glfwTerminate();
		throw(std::runtime_error(std::string("Could not create window")));
	}

	/* Make the window's OpenGL context the current one */
	glfwMakeContextCurrent(window);

	// Initialize the GLEW library to access OpenGL extensions
	// Need to do it after initializing an OpenGL context
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		throw(std::runtime_error(std::string("Could not initialize the GLEW library: ") + std::string((const char *)glewGetErrorString(err))));
	}

	// Set event callbacks
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetFramebufferSizeCallback(window, ResizeCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

// Clears the window by setting it to a colour
void Window::clear(const glm::vec3 &colour) {
	glDepthMask(GL_TRUE);
	glClearColor(colour.r, colour.g, colour.b, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Callback for when a key is pressed
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
}

// Callback for when the window is resized
static void ResizeCallback(GLFWwindow* window, int width, int height) {

	// Set OpenGL viewport based on framebuffer width and height
	glViewport(0, 0, width, height);

}

// Callback for when the mouse moves
static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	mouseX = xpos;
	mouseY = ypos;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mousePressed = true;
	}
}