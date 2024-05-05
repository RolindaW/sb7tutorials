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
		InitializeObject();
		InitializeProgram();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(render_program_);

		//glPatchParameteri(GL_PATCH_VERTICES, 3);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBindVertexArray(vao_);

		glUniformMatrix4fv(0, 1, GL_FALSE, camera_projection_matrix_ * camera_view_matrix_);

		glDrawArrays(GL_PATCHES, 0, 3);
	}

	void shutdown()
	{
		glDeleteVertexArrays(1, &vao_);
		glDeleteBuffers(1, &vbo_);
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

#pragma region Object - Inscribed equilateral triangle

	void InitializeObject()
	{
		// VAO
		glCreateVertexArrays(1, &vao_);

		// Position - vertex attribute (VBO)
		const float kTriangleSide = 10.0f;
		float circleRadius = float(sqrtf(3.0f) / 3.0f) * kTriangleSide;
		const vmath::vec3 positions[] = {
			vmath::vec3(circleRadius * cosf(- M_PI_2 / 3.0f), circleRadius * sinf(-M_PI_2 / 3.0f), 0.0f),  // base-right
			vmath::vec3(circleRadius * cosf(M_PI_2), circleRadius * sinf(M_PI_2), 0.0f),  // top
			vmath::vec3(circleRadius * cosf(7.0f * M_PI_2 / 3.0f), circleRadius * sinf(7.0f * M_PI_2 / 3.0f), 0.0f)  // base-left
		};

		glCreateBuffers(1, &vbo_);
		glNamedBufferStorage(vbo_, 3 * sizeof(vmath::vec3), positions, NULL);

		glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao_, 0, 0);
		glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(vmath::vec3));
		glEnableVertexArrayAttrib(vao_, 0);
	}

#pragma endregion

#pragma region Material

	void InitializeProgram()
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

		// Tessellation control shader
		const char* tcs_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout(vertices = 3) out;											\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Compute and write the tessellation levels required by		\n"
			"	// selected abstract patch type: a triangle						\n"
			"	if (gl_InvocationID == 0)										\n"
			"	{																\n"
			"		gl_TessLevelInner[0] = 5.0;									\n"
			"		gl_TessLevelOuter[0] = 1.0;									\n"
			"		gl_TessLevelOuter[1] = 2.0;									\n"
			"		gl_TessLevelOuter[2] = 3.0;									\n"
			"	}																\n"
			"																	\n"
			"	// Pass-through TCS												\n"
			"	gl_out[gl_InvocationID].gl_Position =							\n"
			"		gl_in[gl_InvocationID].gl_Position;							\n"
			"}																	\n"
		};

		GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tcs, 1, tcs_source, NULL);
		glCompileShader(tcs);

		// Tessellation evaluation shader
		const char* tes_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (triangles, equal_spacing, ccw) in;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Warning! Ordering of input patch vertices must be as			\n"
			"	// expected by TES for the interpolation to work correctly		\n"
			"	// Barycantric coordinates interpolation						\n"
			"	gl_Position = gl_TessCoord.x * gl_in[0].gl_Position +			\n"
			"				  gl_TessCoord.y * gl_in[1].gl_Position +			\n"
			"				  gl_TessCoord.z * gl_in[2].gl_Position;			\n"
			"}																	\n"
		};

		GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tes, 1, tes_source, NULL);
		glCompileShader(tes);

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
		glAttachShader(render_program_, tcs);
		glAttachShader(render_program_, tes);
		glAttachShader(render_program_, fragment_shader);
		glLinkProgram(render_program_);

		// Free resources
		glDeleteShader(vertex_shader);
		glDeleteShader(tcs);
		glDeleteShader(tes);
		glDeleteShader(fragment_shader);
	}

#pragma endregion

private:
	vmath::mat4 camera_view_matrix_;
	vmath::mat4 camera_projection_matrix_;

	GLuint vao_;
	GLuint vbo_;

	GLuint render_program_;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);