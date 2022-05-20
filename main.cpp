
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <vector>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "linmath.h"
#include "stb_image.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "texture.h"
#include "shaders.h"
#include "object.h"

// textures
unsigned int skyboxTextures[6];
unsigned int sunTexture, mercuryTexture, venusTexture, earthTexture, moonTexture, marsTexture, jupiterTexture, saturnTexture, uranusTexture, neptuneTexture;

// objects
Object cube, sphere;

void error_callback(int error, const char* description)
{
	std::cout << "GLFW error: " << description << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// exit on Escape
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
	// init GLFW
	glfwSetErrorCallback(error_callback);
	if(!glfwInit())
		return 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// create window
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Solar system", NULL, NULL);
	if(!window)
	{
		glfwTerminate();
		return 0;
	}

	// set up GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1);

	// create shaders
	GLuint program = createShaders();

	// load skybox textures
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	skyboxTextures[0] = loadTexture("textures/skybox_bk.jpg");
	skyboxTextures[1] = loadTexture("textures/skybox_lf.jpg");
	skyboxTextures[2] = loadTexture("textures/skybox_rt.jpg");
	skyboxTextures[3] = loadTexture("textures/skybox_ft.jpg");
	skyboxTextures[4] = loadTexture("textures/skybox_up.jpg");
	skyboxTextures[5] = loadTexture("textures/skybox_dn.jpg");

	// load planet textures
	sunTexture     = loadTexture("textures/sun.jpg");
	mercuryTexture = loadTexture("textures/mercury.jpg");
	venusTexture   = loadTexture("textures/venus.jpg");
	earthTexture   = loadTexture("textures/earth.jpg");
	moonTexture    = loadTexture("textures/moon.jpg");
	marsTexture    = loadTexture("textures/mars.jpg");
	jupiterTexture = loadTexture("textures/jupiter.jpg");
	saturnTexture  = loadTexture("textures/saturn.jpg");
	uranusTexture  = loadTexture("textures/uranus.jpg");
	neptuneTexture = loadTexture("textures/neptune.jpg");

	// load models
	cube.load("models/cube.obj", program);
	sphere.load("models/sphere.obj", program);

	while(!glfwWindowShouldClose(window))
	{
		// clear window
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set projection matrix
		mat4x4 projMatrix;
		float ratio = float(width) / height;
		mat4x4_perspective(projMatrix, 1.0f, ratio, 0.01f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_FALSE, (const GLfloat*)projMatrix);

		// set camera position
		vec3 cameraPos = {0, 0, 6};

		// set view matrix
		mat4x4 viewMatrix;
		mat4x4_identity(viewMatrix);
		mat4x4_translate_in_place(viewMatrix, -cameraPos[0], -cameraPos[1], -cameraPos[2]);
		mat4x4_rotate_X(viewMatrix, viewMatrix, (float)glfwGetTime() * 0.3f);
		mat4x4_rotate_Y(viewMatrix, viewMatrix, (float)glfwGetTime() * 0.3f);

		// for skybox, reset position column in view matrix
		mat4x4 skyboxViewMatrix;
		mat4x4_dup(skyboxViewMatrix, viewMatrix);
		for(int i = 0; i < 3; i++)
			skyboxViewMatrix[3][i] = 0;

		// draw skybox
		glUniformMatrix4fv(glGetUniformLocation(program, "viewMatrix"), 1, GL_FALSE, (const GLfloat*)skyboxViewMatrix);
		cube.drawSkybox(program, skyboxTextures);

		// draw planets
		glUniformMatrix4fv(glGetUniformLocation(program, "viewMatrix"), 1, GL_FALSE, (const GLfloat*)viewMatrix);
		sphere.draw(program, -4, 0, 0, sunTexture);
		sphere.draw(program, -3, 0, 0, mercuryTexture);
		sphere.draw(program, -2, 0, 0, venusTexture);
		sphere.draw(program, -1, 0, 0, earthTexture);
		sphere.draw(program,  0, 0, 0, moonTexture);
		sphere.draw(program,  1, 0, 0, marsTexture);
		sphere.draw(program,  2, 0, 0, jupiterTexture);
		sphere.draw(program,  3, 0, 0, saturnTexture);
		sphere.draw(program,  4, 0, 0, uranusTexture);
		sphere.draw(program,  5, 0, 0, neptuneTexture);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
