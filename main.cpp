
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
#include "dvec3.h"
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
	// set parameters     distance(m)    speed(m/s) radius(m) mass(kg)    tilt(rad) rotSpeed(rad/s) 
	bodies.push_back(Body(0, 0, 6.957e8, 1.9885e30, 0.13, 2.90308e-6, sunTexture));
	bodies.push_back(Body(5.790905e10, 47360, 2.4397e6, 3.3011e23, 0.00, 1.24002e-6, mercuryTexture));
	bodies.push_back(Body(1.08208e11, 35020, 6.0518e6, 4.8675e24, 3.10, 2.99240e-7, venusTexture));
	bodies.push_back(Body(1.49598023e11, 29780, 6.371e6, 5.97237e24, 0.41, 7.29212e-5, earthTexture));
	bodies.push_back(Body(1.49982422e11, 30802, 1.7374e6, 7.342e22, 0.03, 2.66170e-6, moonTexture));
	bodies.push_back(Body(2.27939366e11, 24070, 3.3895e6, 6.4171e23, 0.44, 7.08822e-5, marsTexture));
	bodies.push_back(Body(7.78479e11, 13070, 6.9911e7, 1.8982e27, 0.05, 1.75852e-4, jupiterTexture));
	bodies.push_back(Body(1.43353e12, 9680, 5.8232e7, 5.6834e26, 0.47, 1.65269e-4, saturnTexture));
	bodies.push_back(Body(2.870972e12, 6800, 2.5362e7, 8.681e25, 1.71, 1.01238e-4, uranusTexture));
	bodies.push_back(Body(4.5e12, 5430, 2.4622e7, 1.02413e26, 0.49, 1.08330e-4, neptuneTexture));
}

void updateBodies(double timeStep)
{
	// speed up time for noticeable animation
	timeStep *= 1e5;

	// use multiple iterations per frame for precise simulation
	const int iterations = 100;
	timeStep /= iterations;

	for (int k = 0; k < iterations; k++)
	{
		// update each body
		for (int i = 0; i < bodies.size(); i++)
		{
			dvec3 totalForce;

			// collect forces from all bodies
			for (int j = 0; j < bodies.size(); j++)
			{
				// skip itself
				if (j == i)
					continue;

				// gravitational force from other body
				totalForce += bodies[i].calcForce(bodies[j]);
			}

			// update velocity and position of this body
			bodies[i].update(totalForce, timeStep);
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
		static double time = glfwGetTime();
		double newTime = glfwGetTime();
		double timeStep = newTime - time;
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
		float ratio = (float)width / height;
		mat4x4_perspective(projMatrix, 1.0f, ratio, 1e8f, 1e13f);
		glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_FALSE, (const GLfloat*)projMatrix);

		// set view matrix with camera focused on the Earth
		Body& earth = bodies[3];
		dvec3 cameraPosition = earth.position + dvec3(0, 3e8, 1e9);
		mat4x4 viewMatrix;
		lookAt(viewMatrix, cameraPosition, earth.position, dvec3(0, 1, 0));

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
		{
			// draw a sphere with exaggerated radius to improve visibilty
			double radius = bodies[i].radius * 20;
			sphere.draw(program, bodies[i].position, radius, bodies[i].tilt, bodies[i].rotAngle, bodies[i].texture);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
