
// vertex shader
const char* vertex_source =
"#version 110\n"
"uniform mat4 projMatrix, viewMatrix, modelMatrix;\n"
"attribute vec3 vPos;\n"
"attribute vec3 vNormal;\n"
"attribute vec2 vUV;\n"
"varying vec3 Position;\n"
"varying vec3 Normal;\n"
"varying vec2 UV;\n"
"void main()\n"
"{\n"
"	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vPos, 1.0);\n"
"	Position = (modelMatrix * vec4(vPos, 1.0)).xyz;\n"
"	Normal = (modelMatrix * vec4(vNormal, 0.0)).xyz;\n"
"	UV = vUV;\n"
"}\n";

// fragment shader
const char* fragment_source =
"#version 110\n"
"uniform sampler2D tex;\n"
"uniform bool useLighting;\n"
"uniform vec3 sunPosition;\n"
"varying vec3 Position;\n"
"varying vec3 Normal;\n"
"varying vec2 UV;\n"
"void main()\n"
"{\n"
"	gl_FragColor = texture2D(tex, UV);\n"
"	if(useLighting)\n"
"	{\n"
"		vec3 normal = normalize(Normal);\n"
"		vec3 sunDirection = normalize(sunPosition - Position);\n"
"		float ambient = 0.3;\n"
"		float diffuse = max(dot(sunDirection, normal), 0.0);\n"
"		float light = ambient + diffuse;\n"
"		gl_FragColor.rgb *= light;\n"
"	}\n"
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
	if (result != GL_TRUE)
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
