static GLuint program;

void OpenGLCreateProgram() {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLchar* vertexSource[1] = { LoadText("vertex.glsl") };
	glShaderSource(vertexShader, 1, vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		Log("fail to compile vertex shader");
		Log(infoLog);
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar* fragmentSource[1] = { LoadText("fragment.glsl") };
	glShaderSource(fragmentShader, 1, fragmentSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		Log("fail to compile fragment shader");
		Log(infoLog);
	}

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		Log("fail to link program");
		Log(infoLog);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

GLuint OpenGLGenerateTextureFromImage(Image image, GLint filter) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

GLuint OpenGLGenerateTextureFromRGBA(uint32 rgba) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &rgba);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

void OpenGLSetVisibleArea() {
	glViewport(0, 0, windowDim.width, windowDim.height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGLClearScreen() {
	glClearColor(0.5f, 0.65f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLRender(GLuint texture, Rectanglef crop, Position2f pos, pixels pwidth, pixels pheight, int32 flip){
	float32 iwwidth = 1.0f / windowDim.width;
	float32 iwheight = 1.0f / windowDim.height;

	float32 radius = pwidth * iwwidth;
	float32 height = 2.0f * pheight * iwheight;
	float32 bottom = (2.0f * pos.y * iwheight) - 1;
	float32 center = (2.0f * pos.x * iwwidth) - 1;

	float32 bottomTex = crop.y1;
	float32 topTex = crop.y0;
	float32 rightTex = flip == 0 ? crop.x1 : crop.x0;
	float32 leftTex = flip == 0 ? crop.x0 : crop.x1;

	float32 vertices[] = {
		// pos                             // tex
		center + radius, bottom,           rightTex, bottomTex,   // bottom-right
		center - radius, bottom + height,  leftTex, topTex,       // top-left
		center - radius, bottom,           leftTex, bottomTex,    // bottom-left

		center - radius, bottom + height,  leftTex, topTex,       // top-left
		center + radius, bottom + height,  rightTex, topTex,      // top-right
		center + radius, bottom,           rightTex, bottomTex    // bottom-right
	};

	GLuint buffersHandle;
	glGenBuffers(1, &buffersHandle);
	glBindBuffer(GL_ARRAY_BUFFER, buffersHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint verticesHandle;
	glGenVertexArrays(1, &verticesHandle);
	glBindVertexArray(verticesHandle);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float32), NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(program, "image"), 0);

	glBindVertexArray(verticesHandle);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &verticesHandle);
	glDeleteBuffers(1, &buffersHandle);
}