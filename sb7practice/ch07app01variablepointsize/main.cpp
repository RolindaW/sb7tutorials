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
	
		// Enable programable point size (glPointSize no longer has any effect)
		glEnable(GL_PROGRAM_POINT_SIZE);
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat backgroundColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, backgroundColor);

		glUseProgram(program);

		// Update the uniforms of the Model-View and Projection matrices
		glUniformMatrix4fv(0, 1, GL_FALSE, cameraViewMatrix * modelWorldMatrix);
		glUniformMatrix4fv(1, 1, GL_FALSE, cameraProjectionMatrix);

		glDrawArrays(GL_POINTS, 0, 13);
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
		int pos_x, pos_y;
		glfwGetWindowPos(window, &pos_x, &pos_y);
		glViewport(pos_x, pos_y, info.windowWidth, info.windowHeight);

		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void onKey(int key, int action)
	{
		sb7::application::onKey(key, action);

		// Check keyboard arrows to...
		// - move camera foward/backward
		// - rotate object
		switch (key)
		{
		case GLFW_KEY_UP:
			if (action)
			{
				// Forward
				MoveCamera(-0.5f);
			}
			break;
		case GLFW_KEY_DOWN:
			if (action)
			{
				// Backward
				MoveCamera(0.5f);
			}
			break;
		case GLFW_KEY_LEFT:
			if (action)
			{
				RotateObject(-1);
			}
			break;
		case GLFW_KEY_RIGHT:
			if (action)
			{
				RotateObject(1);
			}
			break;
		default:
			break;
		}
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
			"const float min_distance = 0.0;									\n"
			"const float max_distance = 20.0;									\n"
			"																	\n"
			"const float max_point_size = 64.0;									\n"
			"const float min_point_size = 1.0;									\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Get vertex position in view space to perform following calcs	\n"
			"	vec4 view_position = mv_matrix * vec4(position, 1.0);			\n"
			"																	\n"
			"	// Calculate distance between camera and point					\n"
			"	// Position vector length is enough as is in view space - camera is the origin (0, 0, 0), not need to substract positions \n"
			"	float distance = length(view_position.xyz);						\n"
			"	// Use normalized distance value to interpolate between point sizes	\n"
			"	float n_distance = (clamp(distance, min_distance, max_distance) - min_distance)	/ (max_distance - min_distance); \n"
			"	// Closest points look biggest; otherwise, smallest				\n"
			"	gl_PointSize = mix(max_point_size, min_point_size, n_distance);	\n"
			"																	\n"
			"	// Output vertex position in clip space							\n"
			"	gl_Position = proj_matrix * view_position;						\n"
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
		cameraPosition = vmath::vec3(0.0f, 0.0f, 20.0f);
		UpdateCameraViewMatrix(cameraPosition);

		// Projection matrix
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void MoveCamera(float z)
	{
		cameraPosition += vmath::vec3(0.0f, 0.0f, z);
		UpdateCameraViewMatrix(cameraPosition);
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

#pragma region Object - Cloud of points

	void InitializeObject()
	{
		// Vertex array object
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// Vertex buffer object
		const float positions[] = {
			0.0f, 0.0f, 0.0f,
			7.0f, 2.0f, 3.0f,
			5.0f, 5.0f, 6.0f,
			2.0f, -4.0f, 3.0f,
			4.0f, 4.0f, -7.0f,
			1.0f, -6.0f, -1.0f,
			4.0f, -3.0f, -2.0f,
			-7.0f, 5.0f, -3.0f,
			-8.0f, -1.0f, -5.0f,
			-3.0f, 2.0f, -5.0f,
			-1.0f, 0.0f, 2.0f,
			-7.0f, 3.0f, 4.0f,
			-5.0f, -2.0f, 2.0f
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

		// Reset Model-World matrix
		modelWorldMatrix = vmath::mat4::identity();
	}

	void RotateObject(int ccw)
	{
		modelWorldMatrix = vmath::rotate(0.0f, ccw * kObjectRotationYStep, 0.0f) * modelWorldMatrix;
	}

#pragma endregion

private:
	GLuint program;

	// Point cloud object
	GLuint vao;
	GLuint vbo;
	vmath::mat4 modelWorldMatrix;
	const float kObjectRotationYStep = 5.0f;

	// Camera
	vmath::vec3 cameraPosition;
	vmath::mat4 cameraViewMatrix;
	vmath::mat4 cameraProjectionMatrix;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);