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
	my_application() : reset_simulation_(false), run_simulation_(true)
	{
	}

public:
	void startup()
	{
		InitializeData();
		InitializeArrays();
		//InitializeUpdateProgram();
		//InitializeRenderProgram();
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
			reset_simulation_ = false;
		}

		// TODO: Simulate physics
		if (run_simulation_)
		{



		}

		// TODO: Render simulation

		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);




	}

	void shutdown()
	{
		delete[] initial_positions_;
		delete[] initial_velocities_;
		delete[] connection_vectors_;

		glDeleteTextures(2, tbo_);
		glDeleteBuffers(5, vbo_);
		glDeleteVertexArrays(2, vao_);
	}

public:
	void onResize(int w, int h)
	{
		sb7::application::onResize(w, h);

		// Update viewport
		glViewport(0, 0, info.windowWidth, info.windowHeight);

		// Update projection matrix: it is required viewport and projection to be consistent
		//UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
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
		default:
			break;
		}
	}

private:
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
												   1.0f);  // Same weight value (1 [Kg]) for all particles
				initial_velocities_[n] = vmath::vec3(0.0f);  // All particles start at rest (0 [m/s])
				connection_vectors_[n] = vmath::ivec4(-1);  // Null connection vertor (i.e. fixed position) by default

				if (j != (kPointsY - 1))  // Fix the position of all particles in the last row
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

private:
	vmath::vec4* initial_positions_;
	vmath::vec3* initial_velocities_;
	vmath::ivec4* connection_vectors_;

	GLuint vao_[2];
	GLuint vbo_[5];
	GLuint tbo_[2];

	//GLuint update_program_;
	//GLuint render_program_;

	bool reset_simulation_;
	bool run_simulation_;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);