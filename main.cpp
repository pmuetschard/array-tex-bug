#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

static int mode = 1;

static void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	} else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		mode = ((mode + 2) % 3) - 1;
	}
}

static GLuint loadShader(GLenum type, const GLchar* src) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	GLint status; glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLint len; glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];
		glGetShaderInfoLog(shader, len, NULL, log);
		fprintf(stderr, "Shader compile failed:\n%s\n", log);
		delete[] log;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static GLuint loadProgram(const GLchar* vSrc, const GLchar* fSrc) {
	int vShader = loadShader(GL_VERTEX_SHADER, vSrc);
	if (vShader == 0) {
		return 0;
	}
	int fShader = loadShader(GL_FRAGMENT_SHADER, fSrc);
	if (fShader == 0) {
		glDeleteShader(vShader);
		return 0;
	}
	int program = glCreateProgram();
	glAttachShader(program, vShader);
	glAttachShader(program, fShader);
	glBindAttribLocation(program, 1, "aPos");
	glBindAttribLocation(program, 2, "aNorm");
	glLinkProgram(program);
	GLint status; glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		GLint len; glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];
		glGetProgramInfoLog(program, len, NULL, log);
		fprintf(stderr, "Program link failed:\n%s\n", log);
		delete[] log;

		glDeleteProgram(program);
		glDeleteShader(vShader);
		glDeleteShader(fShader);
		return 0;
	}
	return program;
}

static void checkGlError(const char* msg) {
	GLint error = glGetError();
	if (error != 0) {
		fprintf(stderr, "GL error: %x - %s\n", error, msg);
	}
}

static GLuint va, fb, textures[2], shaders[2], buffers[2];
static GLuint uTexture, uLayer, uRot;
static const float fsTri[] = {-1, -1, 3, -1, -1, 3};
static const float cubeData[] = {
	-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
	 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
	 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
	 1.0f,  1.0f, -1.0f,  0.0f, 0.0f,  -1.0f,  1.0f, -1.0f, -1.0f,  0.0f, 0.0f,  -1.0f,
	-1.0f,  1.0f, -1.0f,  0.0f, 0.0f,  -1.0f,  1.0f, -1.0f, -1.0f,  0.0f, 0.0f,  -1.0f,
	-1.0f, -1.0f, -1.0f,  0.0f, 0.0f,  -1.0f, -1.0f,  1.0f, -1.0f,  0.0f, 0.0f,  -1.0f,
	-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
	-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
	 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
	 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
	 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
	-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f
};

int main(int argc, char** argv) {
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(800, 400, "Testink", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return 2;
	}
	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

	printf("GL Version:  %s\n", glGetString(GL_VERSION));
	printf("GL Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("GL Renderer: %s\n", glGetString(GL_RENDERER));
	checkGlError("init");

	glGenVertexArrays(1, &va);
	glBindVertexArray(va);
	checkGlError("vertex array");

	glGenFramebuffers(1, &fb);
	checkGlError("framebuffer");

	glGenTextures(2, textures);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textures[0]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 512, 512, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	checkGlError("color texture");

	glBindTexture(GL_TEXTURE_2D_ARRAY, textures[1]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT16, 512, 512, 2, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	checkGlError("depth texture");

	shaders[0] = loadProgram("#version 150 core\n"
		"in vec3 aPos;\n"
		"in vec3 aNorm;\n"
		"uniform float uRot;\n"
		"out vec3 vNorm;\n"
		"void main() {\n"
		"	mat4 m = mat4(cos(uRot), 0, -sin(uRot), 0,  0, 1, 0, 0,  sin(uRot), 0, cos(uRot), 0,  0, 0, -3, 1);\n"
		"	mat4 p = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, -1, 0, 0, -1, 0);\n"
		"	gl_Position = p * m * vec4(aPos, 1);\n"
		"	vNorm = mat3(m) * aNorm;"
		"}\n", "#version 150 core\n"
		"in vec3 vNorm;"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	float diffuse = max(dot(normalize(vNorm), vec3(0.707, 0.707, 0)), 0.0);"
		"	fragColor = vec4(0.2 + 0.8 * diffuse, 0, 0, 1);\n"
		"}\n");
	shaders[1] = loadProgram("#version 150 core\n"
		"in vec2 aPos;\n"
		"out vec2 vUV;\n"
		"void main() {\n"
		"	vUV = (aPos + 1.0) / 2.0;\n"
		"	gl_Position = vec4(aPos, 0, 1);\n"
		"}\n", "#version 150 core\n"
		"uniform sampler2DArray uTexture;\n"
		"uniform float uLayer;\n"
		"in vec2 vUV;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	fragColor = texture(uTexture, vec3(vUV, uLayer));\n"
		"}\n");
	uRot = glGetUniformLocation(shaders[0], "uRot");
	uTexture = glGetUniformLocation(shaders[1], "uTexture");
	uLayer = glGetUniformLocation(shaders[1], "uLayer");
	checkGlError("shaders");

	glGenBuffers(2, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 6 * 6 * 4, cubeData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, 2 * 3 * 4, fsTri, GL_STATIC_DRAW);
	checkGlError("buffer");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE0);
	glClearColor(0, 0, 0, 1);
	checkGlError("setup");

	float rot = 0;
	while (!glfwWindowShouldClose(window)) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[0], 0, 0);
		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textures[1], 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[0], 0, 1);
		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textures[1], 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, 512, 512);
		glUseProgram(shaders[0]);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * 4, NULL);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * 4, (void*)(3 * 4));

		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[0], 0, 0);
		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textures[1], 0, 0);
		glUniform1f(uRot, rot + mode * 0.05);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 3);
		
		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[0], 0, 1);
		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textures[1], 0, 1);
		glUniform1f(uRot, rot - mode * 0.05);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 3);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaders[1]);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textures[0]);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * 4, NULL);
		glUniform1i(uTexture, 0);

		glViewport(0, 0, width / 2, height);
		glUniform1f(uLayer, 0);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glViewport(width / 2, 0, width / 2, height);
		glUniform1f(uLayer, 1);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisableVertexAttribArray(1);

		checkGlError("frame");
		glfwSwapBuffers(window);
		glfwPollEvents();

		rot += 0.01;
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}