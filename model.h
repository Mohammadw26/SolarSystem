
class Model
{
public:
	void load(const char* filename, GLuint program)
	{
		// load model file
		const aiScene* scene = aiImportFile(filename, aiProcessPreset_TargetRealtime_MaxQuality);
		if (!scene)
		{
			// try parent folder
			std::string parent = filename;
			parent = "../../" + parent;
			scene = aiImportFile(parent.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
		}
		if (!scene)
		{
			// print error message
			std::cout << "Error loading model: " << filename << std::endl;
			return;
		}

		// vertex info with positions and texture coordinates
		struct Vertex
		{
			float x, y, z;
			float u, v;

			Vertex(float x_, float y_, float z_, float u_, float v_) : x(x_), y(y_), z(z_), u(u_), v(v_)
			{
			}
		};
		std::vector<Vertex> vertices;

		// for each mesh
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[i];

			// for each face
			for (unsigned int t = 0; t < mesh->mNumFaces; t++)
			{
				aiFace* face = &mesh->mFaces[t];

				// for each index
				for (i = 0; i < face->mNumIndices; i++)
				{
					// get position and texture coordinate
					int index = face->mIndices[i];
					aiVector3D pos = mesh->mVertices[index];
					aiVector3D uv = mesh->mTextureCoords[0][index];

					// add vertex to array
					Vertex vertex(pos.x, pos.y, pos.z, uv.x, uv.y);
					vertices.push_back(vertex);
				}
			}
		}

		// create vertex array object
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// create vertex buffer object
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		// enable vertex attributes
		GLint vPos_location = glGetAttribLocation(program, "vPos");
		GLint vUV_location = glGetAttribLocation(program, "vUV");
		glEnableVertexAttribArray(vPos_location);
		glVertexAttribPointer(vPos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(vUV_location);
		glVertexAttribPointer(vUV_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));

		// store number of vertices
		numVertices = (int)vertices.size();
	}

	void draw(GLuint program, dvec3 position, double radius, double tilt, double rotAngle, unsigned int texture)
	{
		// set position, rotation, and size in model matrix
		mat4x4 modelMatrix;
		mat4x4_identity(modelMatrix);
		mat4x4_translate_in_place(modelMatrix, (float)position.x, (float)position.y, (float)position.z);
		mat4x4_rotate_Z(modelMatrix, modelMatrix, (float)tilt);
		mat4x4_rotate_Y(modelMatrix, modelMatrix, (float)rotAngle);
		mat4x4_scale_aniso(modelMatrix, modelMatrix, (float)radius * 2, (float)radius * 2, (float)radius * 2);
		glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (const GLfloat*)modelMatrix);

		// draw textured 3D model
		glBindVertexArray(vao);
		glEnable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
	}

	void drawSkybox(GLuint program, unsigned int textures[6])
	{
		// set size in model matrix
		mat4x4 modelMatrix;
		mat4x4_identity(modelMatrix);
		mat4x4_scale_aniso(modelMatrix, modelMatrix, 1e10, 1e10, 1e10);
		glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (const GLfloat*)modelMatrix);

		// draw skybox sides using different textures without depth testing
		glBindVertexArray(vao);
		glDisable(GL_DEPTH_TEST);
		for (int i = 0; i < 6; i++)
		{
			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glDrawArrays(GL_TRIANGLES, 6 * i, 6);
		}
	}

private:
	GLuint vao;      // vertex array object
	int numVertices; // number of vertices
};
