// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:

	my_application() : program_pipeline_index_(0)
	{
	}

public:
	void startup()
	{
		InitializeCamera();
		InitializeObject();
		InitializeRenderingProgram();
		InitializeIsolinesBlockTessellationProgram();
		InitializeIsolinesBlockSpiralTessellationProgram();
		InitializeProgramPipeline();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		unsigned int program_pipeline_index = unsigned int(program_pipeline_index_ % 3);

		glBindProgramPipeline(program_pipeline_[program_pipeline_index]);

		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, vmath::vec4(6.0f, 5.0f, 1.0f, 1.0f));  // TCS not used

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBindVertexArray(vao_);

		//glActiveShaderProgram(program_pipeline_, render_program_);
		//glUniformMatrix4fv(0, 1, GL_FALSE, camera_projection_matrix_ * camera_view_matrix_);
		glProgramUniformMatrix4fv(render_program_, 0, 1, GL_FALSE, camera_projection_matrix_ * camera_view_matrix_);

		if (!program_pipeline_index)
		{
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
		else
		{
			glDrawArrays(GL_PATCHES, 0, 4);
		}
	}

	void shutdown()
	{
		glDeleteVertexArrays(1, &vao_);
		glDeleteBuffers(1, &vbo_);
		glDeleteProgram(render_program_);
		glDeleteProgram(isolines_block_tess_program_);
		glDeleteProgram(isolines_block_spiral_tess_program_);
		glBindProgramPipeline(0);
		glDeleteProgramPipelines(3, program_pipeline_);
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
		case GLFW_KEY_T:
			if (action)
			{
				program_pipeline_index_ += 1;
			}
			break;
		default:
			break;
		}
	}

private:

#pragma region Camera

	void InitializeCamera()
	{
		vmath::vec3 camera_position(0.0f, 0.0f, 15.0f);
		UpdateCameraViewMatrix(camera_position);
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		// Calculate the view matrix as the inverse of the model-world matrix of the camera (only translated in this case)
		camera_view_matrix_ = vmath::translate(-position);
	}

	void UpdateCameraProjectionMatrix(float width, float height)
	{
		float fov = 45.0f;
		float aspect = width / height;
		float n = 0.1f, f = 1000.0f;

		camera_projection_matrix_ = vmath::perspective(fov, aspect, n, f);
	}

#pragma endregion

#pragma region Object - Quad

	void InitializeObject()
	{
		// VAO
		glCreateVertexArrays(1, &vao_);

		// Position - vertex attribute (VBO)
		const float kQuadWidth = 10.0f, kQuadHeight = 8.0f;
		const vmath::vec3 positions[] = {
			vmath::vec3(-kQuadWidth / 2.0f, -kQuadHeight / 2.0f, 0.0f),
			vmath::vec3(kQuadWidth / 2.0f, -kQuadHeight / 2.0f, 0.0f),
			vmath::vec3(-kQuadWidth / 2.0f,  kQuadHeight / 2.0f, 0.0f),
			vmath::vec3(kQuadWidth / 2.0f,  kQuadHeight / 2.0f, 0.0f)
		};

		glCreateBuffers(1, &vbo_);
		glNamedBufferStorage(vbo_, 4 * sizeof(vmath::vec3), positions, NULL);

		glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao_, 0, 0);
		glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(vmath::vec3));
		glEnableVertexArrayAttrib(vao_, 0);
	}

#pragma endregion

#pragma region Material

	void InitializeRenderingProgram()
	{
		// Vertex shader
		const char* vertex_shader_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"out gl_PerVertex													\n"
			"{																	\n"
			"	vec4 gl_Position;												\n"
			"};																	\n"
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
		glProgramParameteri(render_program_, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glLinkProgram(render_program_);

		// Free resources
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	void InitializeIsolinesBlockTessellationProgram()
	{
		const char* tes_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"in gl_PerVertex													\n"
			"{																	\n"
			"	vec4 gl_Position;												\n"
			"} gl_in[];															\n"
			"																	\n"
			"out gl_PerVertex													\n"
			"{																	\n"
			"	vec4 gl_Position;												\n"
			"};																	\n"
			"																	\n"
			"layout (isolines, equal_spacing, ccw) in;							\n"
			"//layout (point_mode) in;											\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Warning! Ordering of input patch vertices must be as			\n"
			"	// expected by TES for the interpolation to work correctly		\n"
			"	// Interpolate along bottom edge using x component of the		\n"
			"	// tessellation coordinate										\n"
			"	vec4 p1 = mix(gl_in[0].gl_Position,								\n"
			"				  gl_in[1].gl_Position,								\n"
			"				  gl_TessCoord.x);									\n"
			"																	\n"
			"	// Interpolate along top edge using x component of the			\n"
			"	// tessellation coordinate										\n"
			"	vec4 p2 = mix(gl_in[2].gl_Position,								\n"
			"				  gl_in[3].gl_Position,								\n"
			"				  gl_TessCoord.x);									\n"
			"																	\n"
			"	// Now interpolate those two results using the y component		\n"
			"	// of tessellation coordinate									\n"
			"	gl_Position = mix(p1, p2, gl_TessCoord.y);						\n"
			"}																	\n"
		};

		GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tes, 1, tes_source, NULL);
		glCompileShader(tes);

		// Program
		isolines_block_tess_program_ = glCreateProgram();
		glAttachShader(isolines_block_tess_program_, tes);
		glProgramParameteri(isolines_block_tess_program_, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glLinkProgram(isolines_block_tess_program_);

		// Free resources
		glDeleteShader(tes);
	}

	void InitializeIsolinesBlockSpiralTessellationProgram()
	{
		const char* tes_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"in gl_PerVertex													\n"
			"{																	\n"
			"	vec4 gl_Position;												\n"
			"} gl_in[];															\n"
			"																	\n"
			"out gl_PerVertex													\n"
			"{																	\n"
			"	vec4 gl_Position;												\n"
			"};																	\n"
			"																	\n"
			"layout (isolines, equal_spacing, ccw) in;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// This is not interpolation, but a distribution algorithm		\n"
			"	// working only with tessellated vertices position coordinates	\n"
			"	// Warning! Spiral isolines connect because computed radius		\n"
			"	// value is the same for last vertex in a line and first		\n"
			"	// vertex in the next one.										\n"
			"	float r = gl_TessCoord.y;  // closed isolines 					\n"
			"	r += gl_TessCoord.x / gl_TessLevelOuter[0];  // spiral isolines	\n"
			"	float t = gl_TessCoord.x * 2.0 * 3.14159;						\n"
			"	gl_Position = vec4(sin(t) * r, cos(t) * r, 0.5, 1.0);			\n"
			"}																	\n"
		};

		GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tes, 1, tes_source, NULL);
		glCompileShader(tes);

		// Program
		isolines_block_spiral_tess_program_ = glCreateProgram();
		glAttachShader(isolines_block_spiral_tess_program_, tes);
		glProgramParameteri(isolines_block_spiral_tess_program_, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glLinkProgram(isolines_block_spiral_tess_program_);

		// Free resources
		glDeleteShader(tes);
	}

	void InitializeProgramPipeline()
	{
		glCreateProgramPipelines(3, program_pipeline_);

		glUseProgramStages(program_pipeline_[0], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, render_program_);

		glUseProgramStages(program_pipeline_[1], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, render_program_);
		glUseProgramStages(program_pipeline_[1], GL_TESS_EVALUATION_SHADER_BIT, isolines_block_tess_program_);

		glUseProgramStages(program_pipeline_[2], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, render_program_);
		glUseProgramStages(program_pipeline_[2], GL_TESS_EVALUATION_SHADER_BIT, isolines_block_spiral_tess_program_);
	}

#pragma endregion

private:
	vmath::mat4 camera_view_matrix_;
	vmath::mat4 camera_projection_matrix_;

	GLuint vao_;
	GLuint vbo_;

	GLuint render_program_;
	GLuint isolines_block_tess_program_;
	GLuint isolines_block_spiral_tess_program_;

	GLuint program_pipeline_[3];
	unsigned int program_pipeline_index_;
	
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);