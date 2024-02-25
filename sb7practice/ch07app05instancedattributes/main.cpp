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
		InitializeProgram();
		InitializeObject();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(program);

		glUniformMatrix4fv(0, 1, GL_FALSE, cameraProjectionMatrix * cameraViewMatrix);

		glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 4);
	}

	void shutdown()
	{
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}

public:
	void onResize(int w, int h)
	{
		sb7::application::onResize(w, h);

		// Update viewport
		glViewport(0, 0, info.windowWidth, info.windowHeight);

		// Update projection matrix: it is required viewport and projection to be consistent
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

private:
#pragma region Camera

	void InitializeCamera()
	{
		UpdateCameraViewMatrix(vmath::vec3(0.0f, 0.0f, 10.0f));
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		const vmath::vec3 kTarget = vmath::vec3(0.0f, 0.0f, 0.0f);
		const vmath::vec3 kUp = vmath::vec3(0.0f, 1.0f, 0.0f);
		cameraViewMatrix = vmath::lookat(position, kTarget, kUp);
	}

	void UpdateCameraProjectionMatrix(float width, float height)
	{
		float fov = 45.0f;
		float aspect = width / height;
		float n = 0.1f, f = 1000.0f;

		cameraProjectionMatrix = vmath::perspective(fov, aspect, n, f);
	}

#pragma endregion

#pragma region Material

	void InitializeProgram()
	{
		// Vertex shader
		const char* vertexShaderSource[] =
		{
			"#version 450 core																\n"
			"																				\n"
			"layout (location = 0) uniform mat4 vp_matrix;									\n"
			"																				\n"
			"layout (location = 0) in vec2 position;										\n"
			"layout (location = 1) in vec2 instance_position_offset;						\n"
			"layout (location = 2) in vec4 instance_color;									\n"
			"																				\n"
			"out vec4 vs_color;																\n"
			"																				\n"
			"void main(void)																\n"
			"{																				\n"
			"	vec2 instance_position = vec2(position + instance_position_offset);			\n"
			"	gl_Position = vp_matrix * vec4(instance_position, 0.0, 1.0);				\n"
			"	vs_color = instance_color;													\n"
			"}																				\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const char* fragmentShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"in vec4 vs_color;													\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vs_color;												\n"
			"}																	\n"
		};

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		// Program
		program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		// Free resources
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

#pragma endregion

#pragma region Object

	void InitializeObject()
	{
		// Create and bind VAO (Vertex Array Object)
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// Define (non-instanced and instanced) vertex attributes values
		const GLfloat vertex_positions[] = {
			-1.0f, -1.0f,
			 1.0f, -1.0f,
			 1.0f,  1.0f,
			-1.0f,  1.0f		
		};

		const GLfloat instance_position_offsets[] = {
			-2.0f, -2.0f,
			 2.0f, -2.0f,
			 2.0f,  2.0f,
			-2.0f,  2.0f
		};

		const GLubyte instance_colors[] = {
			0xFF, 0x00, 0x00, 0xFF,
			0x00, 0xFF, 0x00, 0xFF,
			0x00, 0x00, 0xFF, 0xFF,
			0xFF, 0xFF, 0x00, 0xFF
		};

		// Create VBO (Vertex Buffer Object), allocate memory and store data
		glCreateBuffers(1, &vbo);
		const unsigned int bufferSize = sizeof(vertex_positions) + sizeof(instance_position_offsets) + sizeof(instance_colors);
		glNamedBufferStorage(vbo, bufferSize, vertex_positions, GL_DYNAMIC_STORAGE_BIT);
		unsigned int offset = sizeof(vertex_positions);
		glNamedBufferSubData(vbo, offset, sizeof(instance_position_offsets), instance_position_offsets);
		offset += sizeof(instance_position_offsets);
		glNamedBufferSubData(vbo, offset, sizeof(instance_colors), instance_colors);

		// Setup and enable vertex attributes feeding in VAO

		// Vertex positions
		glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(GLfloat) * 2);
		glEnableVertexArrayAttrib(vao, 0);

		// Instance position offsets
		glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 1, 1);
		glVertexArrayVertexBuffer(vao, 1, vbo, sizeof(vertex_positions), sizeof(GLfloat) * 2);
		glVertexAttribDivisor(1, 1);  // Saved into active VAO
		//glVertexArrayBindingDivisor();
		//glVertexBindingDivisor();
		glEnableVertexArrayAttrib(vao, 1);

		// Instance colors
		glVertexArrayAttribFormat(vao, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0);
		glVertexArrayAttribBinding(vao, 2, 2);
		glVertexArrayVertexBuffer(vao, 2, vbo, sizeof(vertex_positions) + sizeof(instance_position_offsets), sizeof(GLubyte) * 4);
		glVertexAttribDivisor(2, 1);
		glEnableVertexArrayAttrib(vao, 2);
	}

#pragma endregion

private:
	// Camera
	vmath::mat4 cameraViewMatrix;
	vmath::mat4 cameraProjectionMatrix;

	// Material
	GLuint program;
	
	// Object
	GLuint vao;
	GLuint vbo;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);