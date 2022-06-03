
unsigned int loadTexture(const char* filename)
{
	// load image
	int width, height;
	stbi_set_flip_vertically_on_load(1);
	unsigned char* data = stbi_load(filename, &width, &height, NULL, 4);
	if (!data)
	{
		// try parent folder
		std::string parent = filename;
		parent = "../../" + parent;
		data = stbi_load(parent.c_str(), &width, &height, NULL, 4);
	}
	if (!data)
	{
		// print error message
		std::cout << "Error loading texture: " << filename << std::endl;
		return 0;
	}

	// create texture
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	stbi_image_free(data);
	return textureID;
}
