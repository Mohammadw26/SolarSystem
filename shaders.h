
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
"	gl_FragColor = texture2D(tex, UV);\n"
"}\n";

GLuint createShader(GLenum type, const char* source)
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

GLuint createShaders()
{
	// create vertex and fragment shaders from sources
	GLuint vertex_shader = createShader(GL_VERTEX_SHADER, vertex_source);
	GLuint fragment_shader = createShader(GL_FRAGMENT_SHADER, fragment_source);

	// create shader program
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	// enable shaders
	glUseProgram(program);
	return program;
}
