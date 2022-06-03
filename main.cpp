
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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
unsigned int ringTexture;

// 3D models
Model cube, sphere, ring;

// Solar system
std::vector<Body> bodies;

// current state
int bodySelection = 0;
int enlargement = 30;
bool dragging = false;
double cursorX = 0, cursorY = 0;
double cameraAngleX = -0.8, cameraAngleY = -0.2;

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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// ignore mouse in ImGui window
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;

	// enable dragging on left button click
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		dragging = true;
		glfwGetCursorPos(window, &cursorX, &cursorY);
	}

	// disable dragging on left button release
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		dragging = false;
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// ignore mouse in ImGui window
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;

	// cursor dragging
	if (dragging)
	{
		// update camera angles
		cameraAngleX -= (xpos - cursorX) * 0.01;
		cameraAngleY -= (ypos - cursorY) * 0.01;
		cursorX = xpos;
		cursorY = ypos;

		// constrain vertical camera angle
		if (cameraAngleY < -1)
			cameraAngleY = -1;
		if (cameraAngleY > 1)
			cameraAngleY = 1;
	}
}

void createBodies()
{
	// set parameters                distance(m)    speed(m/s) radius(m) mass(kg)    tilt(rad) rotSpeed(rad/s) moon option
	bodies.push_back(Body("Sun", 0, 0, 6.957e8, 1.9885e30, 0.13, 2.90308e-6, false, sunTexture));
	bodies.push_back(Body("Mercury", 5.790905e10, 47360, 2.4397e6, 3.3011e23, 0.00, 1.24002e-6, true, mercuryTexture));
	bodies.push_back(Body("Venus", 1.08208e11, 35020, 6.0518e6, 4.8675e24, 3.10, 2.99240e-7, true, venusTexture));
	bodies.push_back(Body("Earth", 1.49598023e11, 29780, 6.371e6, 5.97237e24, 0.41, 7.29212e-5, false, earthTexture));
	bodies.push_back(Body("Moon", 1.49982422e11, 30802, 1.7374e6, 7.342e22, 0.03, 2.66170e-6, false, moonTexture));
	bodies.push_back(Body("Mars", 2.27939366e11, 24070, 3.3895e6, 6.4171e23, 0.44, 7.08822e-5, true, marsTexture));
	bodies.push_back(Body("Jupiter", 7.78479e11, 13070, 6.9911e7, 1.8982e27, 0.05, 1.75852e-4, true, jupiterTexture));
	bodies.push_back(Body("Saturn", 1.43353e12, 9680, 5.8232e7, 5.6834e26, 0.47, 1.65269e-4, true, saturnTexture));
	bodies.push_back(Body("Uranus", 2.870972e12, 6800, 2.5362e7, 8.681e25, 1.71, 1.01238e-4, true, uranusTexture));
	bodies.push_back(Body("Neptune", 4.5e12, 5430, 2.4622e7, 1.02413e26, 0.49, 1.08330e-4, true, neptuneTexture));
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

void addMoon()
{
	// new moon name
	Body& planet = bodies[bodySelection];
	std::string name = planet.name + " moon";

	// create new moon
	double radius = planet.radius * 0.4;
	double mass = planet.mass * 0.001;
	Body newMoon(name, 0, 0, radius, mass, 0.5, 1e-5, false, moonTexture);

	// Sun-planet direction
	Body& sun = bodies[0];
	dvec3 direction = normalize(planet.position - sun.position);

	// new moon position
	double distance = planet.radius * 100;
	newMoon.position = planet.position + direction * distance;

	// new moon velocity required for circular motion around planet
	double force = newMoon.calcForce(planet).length();
	double velocity = sqrt(force * distance / newMoon.mass);
	newMoon.velocity = planet.velocity + normalize(planet.velocity) * velocity;

	// add new moon
	planet.moonOption = false;
	bodies.insert(bodies.begin() + bodySelection + 1, newMoon);
}

void drawGui()
{
	// start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// create ImGui window
	ImGui::Begin("Bodies", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	// create enlargement slider
	ImGui::SliderInt("enlargement", &enlargement, 1, 30);

	// create body checkboxes and buttons
	for (int i = 0; i < bodies.size(); i++)
	{
		// create checkbox for body visibility
		ImGui::Checkbox(("##" + bodies[i].name).c_str(), &bodies[i].visible);
		ImGui::SameLine();

		// create button for body focusing
		if (ImGui::Button(bodies[i].name.c_str()))
			bodySelection = i;
	}

	// create properties label
	ImGui::Separator();
	ImGui::Text("%s properties:", bodies[bodySelection].name.c_str());

	// create mass and spin input fields
	ImGui::InputDouble("mass (kg)", &bodies[bodySelection].mass, 0.0, 0.0, "%e", ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::InputDouble("spin (rad/s)", &bodies[bodySelection].rotSpeed, 0.0, 0.0, "%e", ImGuiInputTextFlags_EnterReturnsTrue);

	// create button for adding moon
	if (bodies[bodySelection].moonOption)
		if (ImGui::Button("add moon"))
			addMoon();

	// draw ImGui window
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main(void)
{
	// init GLFW
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1);

	// set up ImGui
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 110");

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

	// load body textures
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
	ringTexture = loadTexture("textures/ring.png");

	// load models
	cube.load("models/cube.obj", program);
	sphere.load("models/sphere.obj", program);
	ring.load("models/ring.obj", program);

	// create Solar system bodies
	createBodies();

	// select Earth by default
	bodySelection = 3;

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

		// camera rotation using cursor dragging
		dvec3 cameraDirection;
		cameraDirection.x = cos(cameraAngleY) * sin(cameraAngleX);
		cameraDirection.y = -sin(cameraAngleY);
		cameraDirection.z = cos(cameraAngleY) * cos(cameraAngleX);

		// camera distance is roughly proportional to selected body radius
		double cameraDistance = 1.2e6 * sqrt(bodies[bodySelection].radius);
		dvec3 cameraPosition = bodies[bodySelection].position + cameraDirection * cameraDistance;

		// set view matrix with camera focused on selected body
		mat4x4 viewMatrix;
		lookAt(viewMatrix, cameraPosition, bodies[bodySelection].position, dvec3(0, 1, 0));

		// for skybox, reset position column in view matrix
		mat4x4 skyboxViewMatrix;
		mat4x4_dup(skyboxViewMatrix, viewMatrix);
		for (int i = 0; i < 3; i++)
			skyboxViewMatrix[3][i] = 0;

		// draw skybox without lighting
		glUniform1i(glGetUniformLocation(program, "useLighting"), 0);
		glUniformMatrix4fv(glGetUniformLocation(program, "viewMatrix"), 1, GL_FALSE, (const GLfloat*)skyboxViewMatrix);
		cube.drawSkybox(program, skyboxTextures);

		// set sun position for shaders
		Body& sun = bodies[0];
		glUniform3f(glGetUniformLocation(program, "sunPosition"), (float)sun.position.x, (float)sun.position.y, (float)sun.position.z);

		// draw bodies
		glUniformMatrix4fv(glGetUniformLocation(program, "viewMatrix"), 1, GL_FALSE, (const GLfloat*)viewMatrix);
		for (int i = 0; i < bodies.size(); i++)
		{
			// ignore hidden bodies
			if (!bodies[i].visible)
				continue;

			// draw a sphere with enlarged radius to improve visibilty
			glUniform1i(glGetUniformLocation(program, "useLighting"), bodies[i].name != "Sun");
			double radius = bodies[i].radius * enlargement;
			sphere.draw(program, bodies[i].position, radius, bodies[i].tilt, bodies[i].rotAngle, bodies[i].texture);
		}

		// draw a ring for Saturn
		for (int i = 0; i < bodies.size(); i++)
		{
			if (bodies[i].visible && bodies[i].name == "Saturn")
			{
				// use transparent blending
				glUniform1i(glGetUniformLocation(program, "useLighting"), 0);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				double radius = bodies[i].radius * enlargement;
				ring.draw(program, bodies[i].position, radius, bodies[i].tilt, 0, ringTexture);
				glDisable(GL_BLEND);
			}
		}

		drawGui();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
