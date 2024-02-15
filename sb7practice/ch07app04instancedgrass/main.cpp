// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{	
		InitializeCamera();
		InitializeGroundProgram();
		InitializeGround();
		InitializeGrassProgram();
		InitializeGrass();
	}

	void render(double currentTime)
	{
		// Clar color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// Draw - Ground
		glUseProgram(groundProgram);
		glBindVertexArray(groundVao);
		glUniformMatrix4fv(0, 1, GL_FALSE, cameraProjectionMatrix * cameraViewMatrix * groundModelWorldMatrix);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// Draw - Grass
		glUseProgram(grassProgram);
		glBindVertexArray(grassVao);
		glUniformMatrix4fv(0, 1, GL_FALSE, cameraProjectionMatrix * cameraViewMatrix * grassModelWorldMatrix);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 7, 1);
	}

	void shutdown()
	{
		RemoveGround();
		RemoveGrass();
	}

private:

#pragma region Camera

	void InitializeCamera()
	{
		cameraPosition = vmath::vec3(0.0f, 10.0f, 25.0f);
		cameraTarget = vmath::vec3(0.0f, 0.0f, 0.0f);
		cameraUp = vmath::vec3(0.0f, 1.0f, 0.0f);

		// Camera view matrix
		cameraViewMatrix = vmath::lookat(cameraPosition, cameraTarget, cameraUp);

		// Camera projetion matrix
		float fov = 45.0f;
		float aspect = (float)info.windowWidth / (float)info.windowHeight;
		float n = 0.1f, f = 1000.0f;

		cameraProjectionMatrix = vmath::perspective(fov, aspect, n, f);
	}

#pragma endregion

#pragma region Object - Ground

	void InitializeGroundProgram()
	{
		// Vertex shader
		const char* vertexShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 mvp_matrix;						\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	gl_Position = mvp_matrix * vec4(position, 1.0);					\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const char* fragmentShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(0.73, 0.48, 0.34, 1.0);							\n"
			"}																	\n"
		};

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		// Program
		groundProgram = glCreateProgram();
		glAttachShader(groundProgram, vertexShader);
		glAttachShader(groundProgram, fragmentShader);
		glLinkProgram(groundProgram);

		// Free resources
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void InitializeGround()
	{
		// Vertex array object - VAO
		glCreateVertexArrays(1, &groundVao);

		// Vertex buffer object - VBO

		// Vertex attribute values - Position (triangle stripe)
		const GLfloat positions[] = {
			-20.f, 0.0f, 20.f,
			20.f, 0.0f, 20.f,
			-20.f, 0.0f, -20.f,
			20.f, 0.0f, -20.f
		};

		// Create buffer, allocate memory and store data
		glCreateBuffers(1, &groundVbo);
		glNamedBufferStorage(groundVbo, sizeof(positions), positions, NULL);

		// Setup vertex attribute - Position
		glVertexArrayAttribFormat(groundVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(groundVao, 0, 0);
		glVertexArrayVertexBuffer(groundVao, 0, groundVbo, 0, sizeof(GLfloat) * 3);
		glEnableVertexArrayAttrib(groundVao, 0);

		groundModelWorldMatrix = vmath::mat4::identity();
	}

	void RemoveGround()
	{
		glDeleteProgram(groundProgram);
		glDeleteVertexArrays(1, &groundVao);
		glDeleteBuffers(1, &groundVbo);
	}

#pragma endregion

#pragma region Object - Grass

	void InitializeGrassProgram()
	{
		// Vertex shader
		const char* vertexShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 mvp_matrix;						\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	gl_Position = mvp_matrix * vec4(position, 1.0);					\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const char* fragmentShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(0.71, 0.9, 0.11, 1.0);								\n"
			"}																	\n"
		};

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		// Program
		grassProgram = glCreateProgram();
		glAttachShader(grassProgram, vertexShader);
		glAttachShader(grassProgram, fragmentShader);
		glLinkProgram(grassProgram);

		// Free resources
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void InitializeGrass()
	{
		// Vertex array object - VAO
		glCreateVertexArrays(1, &grassVao);

		// Vertex buffer object - VBO

		// Vertex attribute values - Position (triangle stripe)
		const GLfloat positions[] = {
			-0.12f, 0.0f,
			0.13f, 0.0f,
			-0.06f, 0.33f,
			0.19f, 0.33f,
			0.0f, 0.66f,
			0.20f, 0.66f,
			0.10f, 1.0f
		};

		// Create buffer, allocate memory and store data
		glCreateBuffers(1, &grassVbo);
		glNamedBufferStorage(grassVbo, sizeof(positions), positions, NULL);

		// Setup vertex attribute - Position
		glVertexArrayAttribFormat(grassVao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(grassVao, 0, 0);
		glVertexArrayVertexBuffer(grassVao, 0, grassVbo, 0, sizeof(GLfloat) * 2);
		glEnableVertexArrayAttrib(grassVao, 0);

		grassModelWorldMatrix = vmath::mat4::identity();
	}

	void RemoveGrass()
	{
		glDeleteProgram(grassProgram);
		glDeleteVertexArrays(1, &grassVao);
		glDeleteBuffers(1, &grassVbo);
	}

#pragma endregion	

private:
	// Ground
	GLuint groundProgram;
	GLuint groundVao;
	GLuint groundVbo;
	vmath::mat4 groundModelWorldMatrix;

	// Grass
	GLuint grassProgram;
	GLuint grassVao;
	GLuint grassVbo;
	vmath::mat4 grassModelWorldMatrix;

	// Camera
	vmath::vec3 cameraTarget;
	vmath::vec3 cameraPosition;
	vmath::vec3 cameraUp;
	vmath::mat4 cameraViewMatrix;
	vmath::mat4 cameraProjectionMatrix;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);