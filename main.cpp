
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "linmath.h"
#include "stb_image.h"

// positions and texture coordinates for square (2 triangles)
const struct
{
	float x, y, z;
	float u, v;
} square_vertices[6] =
{
	// X   Y  Z     U  V
	{ -1, -1, 0,    0, 0 },
	{ +1, -1, 0,    1, 0 },
	{ +1, +1, 0,    1, 1 },
	{ -1, -1, 0,    0, 0 },
	{ +1, +1, 0,    1, 1 },
	{ -1, +1, 0,    0, 1 },
};

// vertex shader
const char* vertex_source =
"#version 110\n"
"uniform mat4 projMatrix, viewMatrix, modelMatrix;\n"
"attribute vec3 vPos;\n"
"attribute vec2 vUV;\n"
"varying vec2 UV;\n"
"void main()\n"
"{\n"
"	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vPos, 1.0);\n"
"	UV = vUV;\n"
"}\n";

// fragment shader
const char* fragment_source =
"#version 110\n"
"uniform sampler2D tex;\n"
"varying vec2 UV;\n"
"void main()\n"
"{\n"
"	gl_FragColor = texture2D(tex, UV) * 2.0;\n"
"}\n";

const float pi = 3.1415927f;
GLuint program = 0;
unsigned int textures[6];

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

GLuint create_shader(GLenum type, const GLchar* source)
{
	// compile shader
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	// check for complilation errors
	int result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if(result != GL_TRUE)
	{
		// error message length
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

		// print error message
		char* buffer = new char[length];
		glGetShaderInfoLog(shader, length, 0, buffer);
		std::cout << "vertex shader compile error: " << std::endl << buffer << std::endl;
		delete[] buffer;
	}

	return shader;
}

unsigned int load_texture(const char* filename)
{
	// load image
	int width, height;
	stbi_set_flip_vertically_on_load(1);
	unsigned char* data = stbi_load(filename, &width, &height, NULL, 3);
	if(!data)
	{
		// try parent folder
		std::string parent = filename;
		parent = "../../" + parent;
		data = stbi_load(parent.c_str(), &width, &height, NULL, 3);
	}
	if(!data)
	{
		// print error message
		std::cout << "Error loading texture: " << filename << std::endl;
		return 0;
	}

	// create texture
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	// set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	stbi_image_free(data);
	return textureID;
}

void draw_square()
{
	// draw 2 triangles
	glDrawArrays(GL_TRIANGLES, 0, sizeof(square_vertices) / sizeof(square_vertices[0]));
}

void draw_skybox()
{
	mat4x4 modelMatrix;

	// back side
	mat4x4_identity(modelMatrix);
	mat4x4_translate_in_place(modelMatrix, 0, 0, -1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (const GLfloat*)modelMatrix);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	draw_square();

	// left side
	mat4x4_identity(modelMatrix);
	mat4x4_rotate_Y(modelMatrix, modelMatrix, pi * 0.5f);
	mat4x4_translate_in_place(modelMatrix, 0, 0, -1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (const GLfloat*)modelMatrix);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	draw_square();

	// right side
	mat4x4_identity(modelMatrix);
	mat4x4_rotate_Y(modelMatrix, modelMatrix, pi * -0.5f);
	mat4x4_translate_in_place(modelMatrix, 0, 0, -1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (const GLfloat*)modelMatrix);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	draw_square();

	// front side
	mat4x4_identity(modelMatrix);
	mat4x4_rotate_Y(modelMatrix, modelMatrix, pi);
	mat4x4_translate_in_place(modelMatrix, 0, 0, -1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (const GLfloat*)modelMatrix);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	draw_square();

	// up side
	mat4x4_identity(modelMatrix);
	mat4x4_rotate_X(modelMatrix, modelMatrix, pi * 0.5f);
	mat4x4_rotate_Z(modelMatrix, modelMatrix, pi * 0.5f);
	mat4x4_translate_in_place(modelMatrix, 0, 0, -1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (const GLfloat*)modelMatrix);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	draw_square();

	// down side
	mat4x4_identity(modelMatrix);
	mat4x4_rotate_X(modelMatrix, modelMatrix, pi * -0.5f);
	mat4x4_rotate_Z(modelMatrix, modelMatrix, pi * -0.5f);
	mat4x4_translate_in_place(modelMatrix, 0, 0, -1);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (const GLfloat*)modelMatrix);
	glBindTexture(GL_TEXTURE_2D, textures[5]);
	draw_square();
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

	// create vertex buffer for square
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);

	// create shaders
	GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_source);
	GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_source);
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glUseProgram(program);

	// enable vertex attributes
	GLint vPos_location = glGetAttribLocation(program, "vPos");
	GLint vUV_location = glGetAttribLocation(program, "vUV");
	glEnableVertexAttribArray(vPos_location);
	glVertexAttribPointer(vPos_location, 3, GL_FLOAT, GL_FALSE, sizeof(square_vertices[0]), (void*)0);
	glEnableVertexAttribArray(vUV_location);
	glVertexAttribPointer(vUV_location, 2, GL_FLOAT, GL_FALSE, sizeof(square_vertices[0]), (void*)(sizeof(float) * 3));

	// load skybox textures
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	textures[0] = load_texture("textures/corona_bk.jpg");
	textures[1] = load_texture("textures/corona_lf.jpg");
	textures[2] = load_texture("textures/corona_rt.jpg");
	textures[3] = load_texture("textures/corona_ft.jpg");
	textures[4] = load_texture("textures/corona_up.jpg");
	textures[5] = load_texture("textures/corona_dn.jpg");

	while(!glfwWindowShouldClose(window))
	{
		// clear window
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// set projection matrix
		mat4x4 projMatrix;
		float ratio = float(width) / height;
		mat4x4_perspective(projMatrix, 1.0f, ratio, 0.01f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_FALSE, (const GLfloat*)projMatrix);

		// set view matrix
		mat4x4 viewMatrix;
		mat4x4_identity(viewMatrix);
		mat4x4_rotate_X(viewMatrix, viewMatrix, (float)glfwGetTime() * 0.3f);
		mat4x4_rotate_Y(viewMatrix, viewMatrix, (float)glfwGetTime() * 0.3f);
		glUniformMatrix4fv(glGetUniformLocation(program, "viewMatrix"), 1, GL_FALSE, (const GLfloat*)viewMatrix);

		// draw scene
		draw_skybox();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
