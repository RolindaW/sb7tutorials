// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"

enum BufferType
{
	kPositionA,
	kPositionB,
	kVelocityA,
	kVelocityB,
	kConnection
};

enum
{
	kPointsX = 50,
	kPointsY = 50,
	kPointsTotal = (kPointsX * kPointsY)
};

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	my_application() : reset_simulation_(false), run_simulation_(true), iterations_per_frame_(16), iteration_index_(0)
	{
	}

public:
	void startup()
	{
		InitializeData();
		InitializeArrays();
		InitializeUpdateProgram();
		InitializeRenderProgram();
		InitializeCamera();
	}

	void render(double currentTime)
	{
		// Check buffer values - Warning! Read map flag required
		/*
		vmath::vec4* dataPositionA = (vmath::vec4*)glMapNamedBufferRange(vbo_[kPositionA],
			0, kPointsTotal * sizeof(vmath::vec4),
			GL_MAP_READ_BIT);

		glUnmapNamedBuffer(vbo_[kPositionA]);

		vmath::ivec4* dataConnection = (vmath::ivec4*)glMapNamedBufferRange(vbo_[kConnection],
			0, kPointsTotal * sizeof(vmath::ivec4),
			GL_MAP_READ_BIT);

		glUnmapNamedBuffer(vbo_[kConnection]);
		*/

		// Reset simulation
		if (reset_simulation_)
		{
			ResetBuffers();
			iteration_index_ = 0;
			reset_simulation_ = false;
		}

		// Simulate physics
		if (run_simulation_)
		{
			glUseProgram(update_program_);

			glEnable(GL_RASTERIZER_DISCARD);

			for (int i = 0; i < iterations_per_frame_; i++)
			{
				// Current iteration input data (read from):
				// - vao (vertex attributes: position, velocity and connection)
				// - buffer texture (to access connected neighbors position)
				glBindVertexArray(vao_[iteration_index_ & 1]);
				glBindTextureUnit(0, tbo_[iteration_index_ & 1]);

				// Swap buffers
				iteration_index_++;

				// Current interation output data (write into; it would be following iteration input data):
				// - position
				// - velocity
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo_[kPositionA + (iteration_index_ & 1)]);
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, vbo_[kVelocityA + (iteration_index_ & 1)]);

				// Process vertices within the transform feedback
				glBeginTransformFeedback(GL_POINTS);
				glDrawArrays(GL_POINTS, 0, kPointsTotal);
				glEndTransformFeedback();
			}

			glDisable(GL_RASTERIZER_DISCARD);
		}

		// Render simulation

		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(render_program_);

		// Render using input data stored into the buffers that have been written last
		glBindVertexArray(vao_[iteration_index_ & 1]);

		glUniformMatrix4fv(0, 1, GL_FALSE, camera_projection_matrix_ * camera_view_matrix_ * model_world_matrix_);

		// Draw particles
		glPointSize(4.0f);
		glDrawArrays(GL_POINTS, 0, kPointsTotal);
	}

	void shutdown()
	{
		delete[] initial_positions_;
		delete[] initial_velocities_;
		delete[] connection_vectors_;

		glDeleteTextures(2, tbo_);
		glDeleteBuffers(5, vbo_);
		glDeleteVertexArrays(2, vao_);

		glDeleteProgram(update_program_);
		glDeleteProgram(render_program_);
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

	void onKey(int key, int action)
	{
		sb7::application::onKey(key, action);

		switch (key)
		{
		case GLFW_KEY_R:
			if (action)
			{
				reset_simulation_ = true;
			}
			break;
		case GLFW_KEY_P:
			if (action)
			{
				run_simulation_ = !run_simulation_;
			}
			break;
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

#pragma region Object

	/*
	* Warning! Sample code provided within the book does not match picture and text describing the example.
	*
	* Picture and text:
	* - Position layout (by index): +x right; +y down
	* - Neighbors ordering: up, right, down, left
	*
	* 0 1 2 3 4
	* 5 6 7 8 9
	* ...
	*
	* Vertex 1 neighbors: -1, 2, 6, 0
	*
	* Sample code:
	* - Position layout (by index): +x right: +y up
	* - Neighbors ordering: left, down, right, up
	*
	* 5 6 7 8 9
	* 0 1 2 3 4
	*
	* Vertex 1 neighbors: 0, -1, 2, 6
	*/
	void InitializeData()
	{
		initial_positions_ = new vmath::vec4[kPointsTotal];
		initial_velocities_ = new vmath::vec3[kPointsTotal];
		connection_vectors_ = new vmath::ivec4[kPointsTotal];

		int n = 0;
		for (int j = 0; j < kPointsY; j++)  // Iterate over rows [0, kPointsY)
		{
			float fj = (float)j / (float)kPointsY;  // Current row weight [0, 1)
			for (int i = 0; i < kPointsX; i++)  // Iterate over columns [0, kPointsX)
			{
				float fi = (float)i / (float)kPointsX;  // Current column weight [0, 1)

				// Warning! Generated distribution does not lie on the XY plane, but it describes a wavy shape (because of depth coordinate - Z)
				initial_positions_[n] = vmath::vec4((fi - 0.5f) * (float)kPointsX,  // X coordinate centered at the origin [-kPointsX/2, kPointsX/2)
												   (fj - 0.5f) * (float)kPointsY,  // Y coordinate centered at the origin [-kPointsY/2, kPointsY/2)
												   0.6f * sinf(fi) * cosf(fj),  // Z coordinate based on f(fi, fj) sinusoidal distribution
												   0.02f);  // Same weight value (0.02 [Kg] = 20 [gr]) for all particles
				initial_velocities_[n] = vmath::vec3(0.0f);  // All particles start at rest (0 [m/s])
				connection_vectors_[n] = vmath::ivec4(-1);  // Null connection vertor (i.e. fixed position) by default

				unsigned int kFixedPoints = 2;  // Number of desired equidistant gaps in a same row
				if ((j != (kPointsY - 1)) || (i % (kPointsX / kFixedPoints) != 0))  // Fix the position of specific particles in the last row
				{
					if (i != 0)
						connection_vectors_[n][0] = n - 1;
					if (j != 0)
						connection_vectors_[n][1] = n - kPointsX;
					if (i != (kPointsX - 1))
						connection_vectors_[n][2] = n + 1;
					if (j != (kPointsY - 1))
						connection_vectors_[n][3] = n + kPointsX;
				}

				n++;
			}
		}
	}

	void InitializeArrays()
	{
		// VAOs and VBOs
		glCreateVertexArrays(2, vao_);
		glCreateBuffers(5, vbo_);

		for (int i = 0; i < 2; i++)
		{
			// Positions (and mass)
			glNamedBufferStorage(vbo_[kPositionA + i], kPointsTotal * sizeof(vmath::vec4), initial_positions_, GL_DYNAMIC_STORAGE_BIT /*| GL_MAP_READ_BIT*/ );
			glVertexArrayAttribFormat(vao_[i], 0, 4, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(vao_[i], 0, 0);
			glVertexArrayVertexBuffer(vao_[i], 0, vbo_[kPositionA + i], 0, sizeof(vmath::vec4));
			glEnableVertexArrayAttrib(vao_[i], 0);

			// Velocities
			glNamedBufferStorage(vbo_[kVelocityA + i], kPointsTotal * sizeof(vmath::vec3), initial_velocities_, GL_DYNAMIC_STORAGE_BIT);
			glVertexArrayAttribFormat(vao_[i], 1, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(vao_[i], 1, 1);
			glVertexArrayVertexBuffer(vao_[i], 1, vbo_[kVelocityA + i], 0, sizeof(vmath::vec3));
			glEnableVertexArrayAttrib(vao_[i], 1);

			// Connection vectors
			// Warning! (no worries) Function "glNamedBufferStorage" throw error on second pass because it is trying to reallocate memory for an immmutable buffer (flag GL_BUFFER_IMMUTABLE_STORAGE)
			glNamedBufferStorage(vbo_[kConnection], kPointsTotal * sizeof(vmath::ivec4), connection_vectors_, GL_DYNAMIC_STORAGE_BIT /*| GL_MAP_READ_BIT*/ );
			glVertexArrayAttribIFormat(vao_[i], 2, 4, GL_UNSIGNED_INT, 0);
			glVertexArrayAttribBinding(vao_[i], 2, 2);
			glVertexArrayVertexBuffer(vao_[i], 2, vbo_[kConnection], 0, sizeof(vmath::ivec4));
			glEnableVertexArrayAttrib(vao_[i], 2);
		}

		// Buffer textures
		glCreateTextures(GL_TEXTURE_BUFFER, 2, tbo_);
		glTextureBuffer(tbo_[0], GL_RGBA32F, vbo_[kPositionA]);
		glTextureBuffer(tbo_[1], GL_RGBA32F, vbo_[kPositionB]);

		// Object model-world matrix
		model_world_matrix_ = vmath::mat4::identity();
	}

	void ResetBuffers()
	{
		for (int i = 0; i < 2; i++)
		{
			// Positions (and mass)
			glNamedBufferSubData(vbo_[kPositionA + i], 0, kPointsTotal * sizeof(vmath::vec4), initial_positions_);

			// Velocities
			glNamedBufferSubData(vbo_[kVelocityA + i], 0, kPointsTotal * sizeof(vmath::vec3), initial_velocities_);

			// Connection vectors - not required because keep unchanged
			//glNamedBufferSubData(vbo_[kConnection], 0, kPointsTotal * sizeof(vmath::ivec4), connection_vectors_);
		}
	}

	void RotateObject(int ccw)
	{
		model_world_matrix_ = vmath::rotate(0.0f, ccw * kObjectRotationYStep, 0.0f) * model_world_matrix_;
	}

#pragma endregion

#pragma region Programs

	void InitializeUpdateProgram()
	{
		// Vertex shader
		const char* vertex_shader_source[] =
		{
			"#version 450 core															\n"
			"																			\n"
			"// Buffer texture to access connected neighbors position					\n"
			"layout (binding = 0) uniform samplerBuffer tex_position_mass;				\n"
			"																			\n"
			"// Vertex attributes														\n"
			"layout (location = 0) in vec4 position_mass;								\n"
			"layout (location = 1) in vec3 velocity;									\n"
			"layout (location = 2) in ivec4 connection;									\n"
			"																			\n"
			"// Varyings																\n"
			"layout (location = 0) out vec4 xfb_position_mass;							\n"
			"layout (location = 1) out vec3 xfb_velocity;								\n"
			"																			\n"
			"// Constants																\n"
			"uniform float t = 0.01;  // Timestep (application can update this) [s]		\n"
			"const vec3 gravity = vec3(0.0, -9.8, 0.0); // Gravity constant [m/s2]		\n"
			"uniform float k = 7.1;  // Global spring constant [N/m] N = [Kg * m/s2]	\n"
			"uniform float c = 2.8;  // Global damping constant [Kg/s]					\n"
			"uniform float rest_length = 0.88;  // Spring resting length [m]			\n"
			"																			\n"
			"void main(void)															\n"
			"{																			\n"
			"	vec3 p = position_mass.xyz;  // our position [m]						\n"
			"	float m = position_mass.w;  // mass of our vertex [Kg]					\n"
			"	vec3 u = velocity;  // initial velocity [m/s]							\n"
			"	vec3 F = gravity * m - c * u;  // force on the mass	[N]					\n"
			"	bool fixed_node = true;  // Becomes false when force is applied			\n"
			"																			\n"
			"	for (int i = 0; i < 4; i++)												\n"
			"	{																		\n"
			"		if (connection[i] != -1)											\n"
			"		{																	\n"
			"			// position of the other vertex									\n"
			"			vec3 q = texelFetch(tex_position_mass, connection[i]).xyz;		\n"
			"			vec3 d = q - p;													\n"
			"			float x = length(d);  // distance [m]							\n"
			"			F += -k * (rest_length - x) * normalize(d);						\n"
			"			fixed_node = false;												\n"
			"		}																	\n"
			"	}																		\n"
			"																			\n"
			"	// If this is a fixed node, reset force to zero							\n"
			"	if (fixed_node)															\n"
			"	{																		\n"
			"		F = vec3(0.0);														\n"
			"	}																		\n"
			"																			\n"
			"	// Accelleration due to force											\n"
			"	vec3 a = F / m;															\n"
			"																			\n"
			"	// Displacement															\n"
			"	vec3 s = u * t + 0.5 * a * t * t;										\n"
			"																			\n"
			"	// Final velocity														\n"
			"	vec3 v = u + a * t;														\n"
			"																			\n"
			"	// Constrain the absolute value of the displacement per step			\n"
			"	s = clamp(s, vec3(-25.0), vec3(25.0));									\n"
			"																			\n"
			"	// Write outputs														\n"
			"	xfb_position_mass = vec4(p + s, m);										\n"
			"	xfb_velocity = v;														\n"
			"}																			\n"
		};

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// Program
		update_program_ = glCreateProgram();
		glAttachShader(update_program_, vertex_shader);

		// Varyings state specification
		const char* xfb_varyings[] =
		{
			"xfb_position_mass",
			"xfb_velocity"
		};

		glTransformFeedbackVaryings(update_program_, 2, xfb_varyings, GL_SEPARATE_ATTRIBS);

		glLinkProgram(update_program_);

		// Free resources
		glDeleteShader(vertex_shader);
	}

	void InitializeRenderProgram()
	{
		// Vertex shader
		const char* vertex_shader_source[] =
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

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// Fragment shader
		const char* fragment_shader_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) out vec4 color;								\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(1.0);												\n"
			"}																	\n"
		};

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// Program
		render_program_ = glCreateProgram();
		glAttachShader(render_program_, vertex_shader);
		glAttachShader(render_program_, fragment_shader);
		glLinkProgram(render_program_);

		// Free resources
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

#pragma endregion

#pragma region Camera

	void InitializeCamera()
	{
		camera_position_ = vmath::vec3(0.0f, 0.0f, 150.0f);
		UpdateCameraViewMatrix(camera_position_);
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void MoveCamera(float z)
	{
		camera_position_ += vmath::vec3(0.0f, 0.0f, z);
		UpdateCameraViewMatrix(camera_position_);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		const vmath::vec3 kTarget = vmath::vec3(0.0f, 0.0f, 0.0f);
		const vmath::vec3 kUp = vmath::vec3(0.0f, 1.0f, 0.0f);
		camera_view_matrix_ = vmath::lookat(position, kTarget, kUp);
	}

	void UpdateCameraProjectionMatrix(float width, float height)
	{
		float fov = 45.0f;
		float aspect = width / height;
		float n = 0.1f, f = 1000.0f;

		camera_projection_matrix_ = vmath::perspective(fov, aspect, n, f);
	}

#pragma endregion

private:
	vmath::vec4* initial_positions_;
	vmath::vec3* initial_velocities_;
	vmath::ivec4* connection_vectors_;

	GLuint vao_[2];
	GLuint vbo_[5];
	GLuint tbo_[2];

	vmath::mat4 model_world_matrix_;
	const float kObjectRotationYStep = 5.0f;

	GLuint update_program_;
	GLuint render_program_;

	vmath::vec3 camera_position_;
	vmath::mat4 camera_view_matrix_;
	vmath::mat4 camera_projection_matrix_;

	bool reset_simulation_;
	bool run_simulation_;
	unsigned int iterations_per_frame_;
	unsigned int iteration_index_;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);
