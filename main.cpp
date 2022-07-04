
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
#include "camera.h"

// textures
unsigned int skyboxTextures[6];
unsigned int sunTexture, mercuryTexture, venusTexture, earthTexture, moonTexture, marsTexture, jupiterTexture, saturnTexture, uranusTexture, neptuneTexture;
unsigned int ringTexture;

// 3D models
Model cube, sphere, ring;

// Solar system
std::vector<Body> bodies;
const int inner = -1;
const int outer = -2;

// current state
bool paused = false;
int bodySelection = 0;
int sunScale = 30;
int bodyScale = 1000;
int moonOrbitScale = 80;
Camera camera;

void error_callback(int error, const char* description)
{
	std::cout << "GLFW error: " << description << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	camera.keyPress(key, action);

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

	camera.mouseClick(window, button, action);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// ignore mouse in ImGui window
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;

	camera.mouseMove(xpos, ypos);
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

				// gravitational force from the other body
				totalForce += bodies[i].calcForce(bodies[j]);
			}

			// update velocity and position of this body
			bodies[i].update(totalForce, timeStep);
		}
	}
}

double bodyRadius(int i)
{
	Body& body = bodies[i];
	return body.radius * (i == 0 ? sunScale : bodyScale);
}

dvec3 bodyPosition(int i)
{
	Body& body = bodies[i];
	dvec3 position = body.position;

	// for moons, scale their orbit to improve visibilty
	if (body.texture == moonTexture)
	{
		// host planet is the previous body
		Body& planet = bodies[i - 1];
		position = planet.position + (position - planet.position) * moonOrbitScale;
	}

	return position;
}

void setCamera()
{
	Body& sun = bodies[0];

	if (bodySelection == inner)
	{
		// focus on inner planets
		double cameraDistance = 5e11;
		dvec3 cameraDirection = normalize(dvec3(0, 1, 1));
		double cameraSpeed = 1e11;
		camera.set(sun.position, cameraDirection, cameraDistance, cameraSpeed);
	}
	else if (bodySelection == outer)
	{
		// focus on outer planets
		double cameraDistance = 1e13;
		dvec3 cameraDirection = normalize(dvec3(0, 1, 1));
		double cameraSpeed = 1e12;
		camera.set(sun.position, cameraDirection, cameraDistance, cameraSpeed);
	}
	else
	{
		// camera distance is roughly proportional to selected body radius
		Body& body = bodies[bodySelection];
		dvec3 position = bodyPosition(bodySelection);
		double cameraDistance = bodySelection == 0 ? 8e10 : 4e7 * sqrt(body.radius);

		// camera direction is back and above
		dvec3 cameraDirection = normalize(dvec3(0, 0, 1));
		if (bodySelection != 0)
			cameraDirection = normalize(position - sun.position);
		cameraDirection.rotate(0.8, dvec3(0, 1, 0));
		cameraDirection.y = 0.5;

		// focus on the selected body
		double cameraSpeed = 1e11;
		camera.set(position, cameraDirection, cameraDistance, cameraSpeed);
	}
}

void addMoon()
{
	// new moon name
	Body& planet = bodies[bodySelection];
	std::string name = planet.name + " moon";

	// create new moon
	double radius = planet.radius * 0.3;
	double mass = planet.mass * 0.001;
	Body newMoon(name, 0, 0, radius, mass, 0.5, 1e-5, false, moonTexture);

	// Sun-planet direction
	Body& sun = bodies[0];
	dvec3 direction = normalize(planet.position - sun.position);

	// new moon position
	double distance = planet.radius * 40;
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
	ImGui::Begin("Solar system", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	// create button for pausing/resuming
	if (ImGui::Button(paused ? "resume" : "pause"))
		paused = !paused;

	// create scale sliders
	ImGui::SliderInt("Sun scale", &sunScale, 1, 50);
	ImGui::SliderInt("Body scale", &bodyScale, 1, 1000);
	ImGui::SliderInt("Moon orbit scale", &moonOrbitScale, 1, 100);

	// create checkboxes and buttons for each body
	for (int i = 0; i < bodies.size(); i++)
	{
		Body& body = bodies[i];

		// create buttons for inner planets
		if (body.name == "Mercury" && ImGui::Button("Inner planets"))
		{
			bodySelection = inner;
			setCamera();
		}

		// create buttons for outer planets
		if (body.name == "Jupiter" && ImGui::Button("Outer planets"))
		{
			bodySelection = outer;
			setCamera();
		}

		// create checkbox for body visibility
		ImGui::Checkbox(("##" + body.name).c_str(), &body.visible);
		ImGui::SameLine();

		// create button for body focusing
		if (ImGui::Button(body.name.c_str()))
		{
			bodySelection = i;
			setCamera();
		}
	}

	if (bodySelection >= 0)
	{
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
	}

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
	setCamera();

	while (!glfwWindowShouldClose(window))
	{
		// get time step since the previous frame
		static double time = glfwGetTime();
		double newTime = glfwGetTime();
		double timeStep = newTime - time;
		time = newTime;

		// update velocities and positions of all bodies
		if (!paused)
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

		// set view matrix
		camera.update(timeStep);
		mat4x4 viewMatrix;
		camera.GetViewMatrix(viewMatrix);

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
			Body& body = bodies[i];
			if (!body.visible)
				continue;

			// draw a sphere
			glUniform1i(glGetUniformLocation(program, "useLighting"), body.name != "Sun");
			sphere.draw(program, bodyPosition(i), bodyRadius(i), body.tilt, body.rotAngle, body.texture);
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
				ring.draw(program, bodyPosition(i), bodyRadius(i), bodies[i].tilt, 0, ringTexture);
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
