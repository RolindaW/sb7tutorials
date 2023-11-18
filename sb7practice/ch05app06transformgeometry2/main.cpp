// Include the "sb7.h" header file
#include "sb7.h"

// Other includes
#include "vmath.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		update_view_matrix();
		update_projection_matrix();

		program = compile_shaders();

		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		vbo = create_vbo(vao);

		// Enable culling (by default back-facing triangles are culled).
		glEnable(GL_CULL_FACE);
	}

	void render(double currentTime)
	{
		// Clear the framebuffer with dark green
		static const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// Use the program object we created earlier for rendering
		glUseProgram(program);

		// Update projection matrix uniform value - same for all game objects in the scene
		glUniformMatrix4fv(1, 1, GL_FALSE, m_proj);  // proj_matrix

		const int num_cubes = 25;
		for (int i = 0; i < num_cubes; i++)
		{
			// Simulate object motion
			simulate_model_world_matrix(currentTime, i);

			// Update model-view matrix uniform value - unique per each game object
			vmath::mat4 m_model_view = m_view * m_model_world;
			glUniformMatrix4fv(0, 1, GL_FALSE, m_model_view);  // mv_matrix

			// Render all polygons in wireframe mode
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			// Draw 6 faces of 2 triangles of 3 vertices each = 36 vertices
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	void shutdown()
	{
		glDeleteProgram(program);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

public:
	void onResize(int w, int h)
	{
		sb7::application::onResize(w, h);

		// Update viewport
		int pos_x, pos_y;
		glfwGetWindowPos(window, &pos_x, &pos_y);
		glViewport(pos_x, pos_y, info.windowWidth, info.windowHeight);

		update_projection_matrix();
	}

private:
	// Rendering program creation
	GLuint compile_shaders(void)
	{
		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint program;

		// Source code for vertex shader
		static const GLchar* vertex_shader_source[] =
		{
			"#version 450 core														\n"
			"																		\n"
			"layout (location = 0) uniform mat4 mv_matrix;							\n"
			"layout (location = 1) uniform mat4 proj_matrix;						\n"
			"																		\n"
			"layout (location = 0) in vec3 position;								\n"
			"layout (location = 1) in vec3 color;									\n"
			"																		\n"
			"out vec4 vs_color;														\n"
			"																		\n"
			"void main(void)														\n"
			"{																		\n"
			"	// Transform vertex position into clip space						\n"
			"	gl_Position = proj_matrix * mv_matrix * vec4(position, 1.0);		\n"
			"																		\n"
			"	// Calculate vertex color based on original position				\n"
			"	vs_color = vec4(position, 1.0) + vec4(0.5, 0.5, 0.5, 0.0);			\n"
			"	//vs_color = vec4(color, 1.0);										\n"
			"}																		\n"
		};

		// Source code for fragment shader
		static const GLchar* fragment_shader_source[] =
		{
			"#version 450 core							\n"
			"											\n"
			"in vec4 vs_color;							\n"
			"											\n"
			"out vec4 color;							\n"
			"											\n"
			"void main(void)							\n"
			"{											\n"
			"	color = vs_color;						\n"
			"}											\n"
		};

		// Create and compile vertex shader
		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// Create and compile fragment shader
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// Create program, attach shaders to it, and link it
		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		// Delete the shaders as the program has them now
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}

	GLuint create_vbo(GLuint vao)
	{
		const float data[] = {
			// Face 1: near - red
			-0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
			 0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
			-0.5,  0.5,  0.5, 1.0, 0.0, 0.0,

			-0.5,  0.5,  0.5, 1.0, 0.0, 0.0,
			 0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
			 0.5,  0.5,  0.5, 1.0, 0.0, 0.0,

			// Face 2: right - green
			 0.5, -0.5,  0.5, 0.0, 1.0, 0.0,
			 0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
			 0.5,  0.5,  0.5, 0.0, 1.0, 0.0,

			 0.5,  0.5,  0.5, 0.0, 1.0, 0.0,
			 0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
			 0.5,  0.5, -0.5, 0.0, 1.0, 0.0,

			// Face 3: left - blue
			-0.5, -0.5, -0.5, 0.0, 0.0, 1.0,
			-0.5, -0.5,  0.5, 0.0, 0.0, 1.0,
			-0.5,  0.5, -0.5, 0.0, 0.0, 1.0,

			-0.5,  0.5, -0.5, 0.0, 0.0, 1.0,
			-0.5, -0.5,  0.5, 0.0, 0.0, 1.0,
			-0.5,  0.5,  0.5, 0.0, 0.0, 1.0,

			// Face 4: up - yellow
			-0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
			 0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
			-0.5,  0.5, -0.5, 1.0, 1.0, 0.0,

			-0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
			 0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
			 0.5,  0.5, -0.5, 1.0, 1.0, 0.0,

			// Face 5: down - pink
			-0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
			 0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
			-0.5, -0.5,  0.5, 1.0, 0.0, 1.0,

			-0.5, -0.5,  0.5, 1.0, 0.0, 1.0,
			 0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
			 0.5, -0.5,  0.5, 1.0, 0.0, 1.0,

			// Face 6: far - light blue
			 0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
			-0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
			 0.5,  0.5, -0.5, 0.0, 1.0, 1.0,

			 0.5,  0.5, -0.5, 0.0, 1.0, 1.0,
			-0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
			-0.5,  0.5, -0.5, 0.0, 1.0, 1.0
		};

		// Create buffer object
		GLuint vbo;
		glCreateBuffers(1, &vbo);

		// Allocate memory and copy data
		glNamedBufferStorage(vbo, sizeof(data), data, NULL);

		// Bind as a vertex buffer object
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// Set up vertex attribute "position" (location = 0) and enable automatic load
		int vertex_size = sizeof(float) * 3 + sizeof(float) * 3;

		// Set up vertex attribute "position" (location = 0) and enable automatic load
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, vertex_size);
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(vao, 0);

		// Set up vertex attribute "color" (location = 0) and enable automatic load
		glVertexArrayAttribBinding(vao, 1, 1);
		glVertexArrayVertexBuffer(vao, 1, vbo, 0, vertex_size);
		glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
		glEnableVertexArrayAttrib(vao, 1);

		return vbo;
	}

	// Projection matrix can change on window (viewport) resize. Call both during application initialization and on window resize.
	void update_projection_matrix()
	{
		// Perspective projection
		float aspect_ratio = (float)info.windowWidth / (float)info.windowHeight;
		vmath::mat4 mat = vmath::perspective(50.0f, aspect_ratio, 0.1f, 1000.0f);

		// Store new value
		m_proj = mat;
	}

	// Camera pose (position and orientation) does not change in this application. It is enough calling this function once - during application initialization.
	void update_view_matrix()
	{
		// Fixed data of the camera (default orientation; rotation not applied)
		vmath::vec3 camera_position(1.0f, 1.0f, 12.0f);

		// Calculate world-view transformation matrix.

		// One approach is to calculate it as inverse of the model-world transformation matrix of the camera.
		// First, calculate the model-view transformation matrix of the camera.
		// Next, calculate the inverse of this matrix to get the world-view transformation matrix.
		// Note: In this case is very simple: the inverse of the translation (unary negation).
		vmath::mat4 mat = vmath::translate(-camera_position);

		// Another approach is to calculate the LooAt matrix with the origin of the world reference frame as target
		//mat = vmath::lookat(camera_position, vmath::vec3(0.0, 0.0, 0.0), vmath::vec3(0.0, 1.0, 0.0));

		// Store new value
		m_view = mat;
	}

	void simulate_model_world_matrix(double currentTime, int cube_index)
	{
		float f = (float)cube_index + (float)currentTime * 0.3f;

		vmath::mat4 mat =
			vmath::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f) *
			vmath::rotate((float)currentTime * 21.0f, 1.0f, 0.0f, 0.0f) *
			vmath::translate(sinf(2.1f * f) * 2.0f,
							 cosf(1.7f * f) * 2.0f,
							 sinf(1.3f * f) * cosf(1.5f * f) * 2.0f);

		//mat = vmath::mat4::identity();  // Uncomment to make the cube static.

		m_model_world = mat;
	}

private:
	GLuint program;
	GLuint vao;
	GLuint vbo;

	vmath::mat4 m_model_world;
	vmath::mat4 m_view;
	vmath::mat4 m_proj;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);