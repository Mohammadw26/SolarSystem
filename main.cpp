
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
#include "model.h"
#include "body.h"

// textures
unsigned int skyboxTextures[6];
unsigned int sunTexture, mercuryTexture, venusTexture, earthTexture, moonTexture, marsTexture, jupiterTexture, saturnTexture, uranusTexture, neptuneTexture;

// 3D models
Model cube, sphere;

// Solar system
std::vector<Body> bodies;

void error_callback(int error, const char* description)
{
	std::cout << "GLFW error: " << description << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// exit on Escape
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void createBodies()
{
	// set parameters        x   y   z radius   mass
	bodies.push_back(Body(0, 0, 0, 40, 10000, sunTexture));
	bodies.push_back(Body(200, 0, 0, 6, 20, mercuryTexture));
	bodies.push_back(Body(300, 0, 0, 9, 80, venusTexture));
	bodies.push_back(Body(500, 0, 0, 10, 100, earthTexture));
	bodies.push_back(Body(500, 0, 20, 3, 1, moonTexture));
	bodies.push_back(Body(800, 0, 0, 8, 60, marsTexture));
	bodies.push_back(Body(1100, 0, 0, 18, 200, jupiterTexture));
	bodies.push_back(Body(1500, 0, 0, 17, 160, saturnTexture));
	bodies.push_back(Body(2100, 0, 0, 14, 120, uranusTexture));
	bodies.push_back(Body(2700, 0, 0, 14, 120, neptuneTexture));

	// set initial velocities for planets
	for (int i = 1; i < bodies.size(); i++)
	{
		// gravitational force from the Sun
		Body& sun = bodies[0];
		vec3 force;
		bodies[i].calcForce(sun, force);

		// velocity required for circular motion around the Sun
		bodies[i].velocity[2] = -sqrtf(vec3_len(force) * bodies[i].position[0] / bodies[i].mass);
	}

	// set initial velocity for the Moon
	{
		// gravitational force from the Earth
		Body& earth = bodies[3];
		Body& moon = bodies[4];
		vec3 force;
		moon.calcForce(earth, force);

		// velocity required for circular motion around the Earth
		moon.velocity[0] = sqrtf(vec3_len(force) * moon.position[2] / moon.mass);
	}
}

void updateBodies(float timeStep)
{
	const int iterations = 100;

	// use multiple iterations per frame for precise simulation
	for (int k = 0; k < iterations; k++)
	{
		// update each body
		for (int i = 0; i < bodies.size(); i++)
		{
			vec3 totalForce = { 0, 0, 0 };

			// collect forces from all bodies
			for (int j = 0; j < bodies.size(); j++)
			{
				// skip itself
				if (j == i)
					continue;

				// gravitational force from other body
				vec3 force;
				bodies[i].calcForce(bodies[j], force);
				vec3_add(totalForce, totalForce, force);
			}

			// update velocity and position of this body
			bodies[i].update(totalForce, timeStep / iterations);
		}
	}
}

int main(void)
{
	// init GLFW
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// create window
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Solar system", NULL, NULL);
	if (!window)
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
	glUniform1i(glGetUniformLocation(program, "tex"), 0);

	// load skybox textures
	skyboxTextures[0] = loadTexture("textures/skybox_bk.jpg");
	skyboxTextures[1] = loadTexture("textures/skybox_lf.jpg");
	skyboxTextures[2] = loadTexture("textures/skybox_rt.jpg");
	skyboxTextures[3] = loadTexture("textures/skybox_ft.jpg");
	skyboxTextures[4] = loadTexture("textures/skybox_up.jpg");
	skyboxTextures[5] = loadTexture("textures/skybox_dn.jpg");

	// load planet textures
	sunTexture = loadTexture("textures/sun.jpg");
	mercuryTexture = loadTexture("textures/mercury.jpg");
	venusTexture = loadTexture("textures/venus.jpg");
	earthTexture = loadTexture("textures/earth.jpg");
	moonTexture = loadTexture("textures/moon.jpg");
	marsTexture = loadTexture("textures/mars.jpg");
	jupiterTexture = loadTexture("textures/jupiter.jpg");
	saturnTexture = loadTexture("textures/saturn.jpg");
	uranusTexture = loadTexture("textures/uranus.jpg");
	neptuneTexture = loadTexture("textures/neptune.jpg");

	// load models
	cube.load("models/cube.obj", program);
	sphere.load("models/sphere.obj", program);

	// create Solar system bodies
	createBodies();

	while (!glfwWindowShouldClose(window))
	{
		// get time step since the previous frame
		static float time = (float)glfwGetTime();
		float newTime = (float)glfwGetTime();
		float timeStep = newTime - time;
		time = newTime;

		// update velocities and positions of all bodies
		updateBodies(timeStep);

		// clear window
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set projection matrix
		mat4x4 projMatrix;
		float ratio = float(width) / height;
		mat4x4_perspective(projMatrix, 1.0f, ratio, 0.1f, 10000.0f);
		glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_FALSE, (const GLfloat*)projMatrix);

		// set camera position
		vec3 cameraPos = { 0, 0, 600 };

		// set view matrix
		mat4x4 viewMatrix;
		mat4x4_identity(viewMatrix);
		mat4x4_translate_in_place(viewMatrix, -cameraPos[0], -cameraPos[1], -cameraPos[2]);
		mat4x4_rotate_X(viewMatrix, viewMatrix, 0.5f);

		// for skybox, reset position column in view matrix
		mat4x4 skyboxViewMatrix;
		mat4x4_dup(skyboxViewMatrix, viewMatrix);
		for (int i = 0; i < 3; i++)
			skyboxViewMatrix[3][i] = 0;

		// draw skybox
		glUniformMatrix4fv(glGetUniformLocation(program, "viewMatrix"), 1, GL_FALSE, (const GLfloat*)skyboxViewMatrix);
		cube.drawSkybox(program, skyboxTextures);

		// draw bodies
		glUniformMatrix4fv(glGetUniformLocation(program, "viewMatrix"), 1, GL_FALSE, (const GLfloat*)viewMatrix);
		for (int i = 0; i < bodies.size(); i++)
			sphere.draw(program, bodies[i].position, bodies[i].radius, bodies[i].texture);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
