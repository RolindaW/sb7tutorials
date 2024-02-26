// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializeProgram();
		InitializeCamera();
		InitializeObject();

		glEnable(GL_CULL_FACE);
	}

	void render(double currentTime)
	{
		// Clear the framebuffer with dark green
		static const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(program);

		SimulateObject(currentTime);

		// Update the uniforms of the Model-View and Projection matrices
		glUniformMatrix4fv(0, 1, GL_FALSE, cameraViewMatrix * modelWorldMatrix);
		glUniformMatrix4fv(1, 1, GL_FALSE, cameraProjectionMatrix);

		// Draw indexed cube
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
	}

	void shutdown()
	{
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &vebo);
	}

public:
	void onResize(int w, int h)
	{
		sb7::application::onResize(w, h);

		// Update viewport
		int pos_x, pos_y;
		glfwGetWindowPos(window, &pos_x, &pos_y);
		glViewport(pos_x, pos_y, info.windowWidth, info.windowHeight);

		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

private:

	void InitializeProgram()
	{
		// Vertex shader
		const char* vertexShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 mv_matrix;						\n"
			"layout (location = 1) uniform mat4 proj_matrix;					\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Output vertex position in clip space							\n"
			"	gl_Position = proj_matrix * mv_matrix * vec4(position, 1.0);	\n"
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
			"	color = vec4(0.0, 0.8, 1.0, 1.0);								\n"
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

#pragma region Camera

	void InitializeCamera()
	{
		// View matrix
		vmath::vec3 cameraPosition(0.0f, 0.0f, 5.0f);
		UpdateCameraViewMatrix(cameraPosition);

		// Projection matrix
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		// Calculate view matrix as the inverse of camera model-world matrix - assume only translation
		cameraViewMatrix = vmath::translate(-position);
	}

	void UpdateCameraProjectionMatrix(float viewportWidth, float viewportHeight)
	{
		float fov = 45.0f;
		float aspect = viewportWidth / viewportHeight;
		float n = 0.1f, f = 1000.0f;

		cameraProjectionMatrix = vmath::perspective(fov, aspect, n, f);
	}

#pragma endregion

#pragma region Object - Cube

	void InitializeObject()
	{
		// Vertex array object
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// Vertex buffer object
		const float positions[] = {
			-0.5f, -0.5f,  0.5f,  // front
			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f, -0.5f, -0.5f,  // back
			 0.5f, -0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,
		};

		// Create buffer object to store values of position vertex attribute, allocate required memory and bind to array buffer target
		glCreateBuffers(1, &vbo);
		glNamedBufferStorage(vbo, sizeof(positions), positions, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// Define position vertex attribute data format, index mapping, buffer object binding and enable it
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(GL_FLOAT) * 3);
		glEnableVertexArrayAttrib(vao, 0);

		// Vertex element (indices) buffer object
		const unsigned char indices[] = {
			0, 1, 2,  // front
			0, 2, 3,
			1, 5, 6,  // right
			1, 6, 2,
			4, 0, 3,  // left
			4, 3, 7,
			3, 2, 6,  // top
			3, 6, 7,
			4, 5, 1,  // bottom
			4, 1, 0,
			5, 4, 7,  // back
			5, 7, 6
		};

		// Create buffer object to store values of indices, allocate required memory and bind to array element buffer target
		glCreateBuffers(1, &vebo);
		glNamedBufferStorage(vebo, sizeof(indices), indices, NULL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vebo);

		// Reset Model-World matrix
		modelWorldMatrix = vmath::mat4::identity();
	}

	void SimulateObject(double currentTime)
	{
		float f = (float)currentTime * (float)M_PI * 0.1f;

		vmath::mat4 mat =
			vmath::translate(sinf(2.1f * f) * 0.5f,
				cosf(1.7f * f) * 0.5f,
				sinf(1.3f * f) * cosf(1.5f * f) * 2.0f) *
			vmath::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f) *
			vmath::rotate((float)currentTime * 81.0f, 1.0f, 0.0f, 0.0f);

		//mat = vmath::mat4::identity();  // Uncomment to make the cube static.

		modelWorldMatrix = mat;
	}

#pragma endregion

private:
	GLuint program;

	GLuint vao;  // Store vertex fetch stage settings
	GLuint vbo;  // Store per-vertex data
	GLuint vebo;  // Store vertex indices
	vmath::mat4 modelWorldMatrix;

	vmath::mat4 cameraViewMatrix;
	vmath::mat4 cameraProjectionMatrix;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);