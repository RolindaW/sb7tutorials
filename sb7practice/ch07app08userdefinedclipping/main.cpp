// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"
#include "object.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	my_application() : clip_plane_(vmath::vec4(0.0f, 1.0f, 0.0f, -4.0f))
	{
	}

public:
	
	void startup()
	{
		InitializeProgram();
		InitializeCamera();
		InitializeObject();

		glEnable(GL_CULL_FACE);
		glEnable(GL_CLIP_DISTANCE0);
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(render_program_);

		glUniformMatrix4fv(0, 1, GL_FALSE, vmath::rotate(0.0f, 60.0f, 0.0f));
		glUniformMatrix4fv(1, 1, GL_FALSE, camera_projection_matrix_ * camera_view_matrix_);
		glUniform4fv(2, 1, clip_plane_);

		object_.render();
	}

	void shutdown()
	{
		DestroyObject();
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
		case GLFW_KEY_UP:
			if (action)
			{
				TraslateClipPlane(-0.25f);
			}
			break;
		case GLFW_KEY_DOWN:
			if (action)
			{
				TraslateClipPlane(0.25f);
			}
			break;
		default:
			break;
		}
	}

private:

#pragma region Program

	void InitializeProgram()
	{
		// Vertex shader
		const char* vertex_shader_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 w_matrix;						\n"
			"layout (location = 1) uniform mat4 vp_matrix;						\n"
			"layout (location = 2) uniform vec4 clip_plane;						\n"
			"																	\n"
			"layout (location = 0) in vec4 position;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Transform positon into view space							\n"
			"	vec4 vs_position = w_matrix * position;							\n"
			"																	\n"
			"	// Calculate and write custom clip distance						\n"
			"	gl_ClipDistance[0] = dot(vs_position, clip_plane);				\n"
			"																	\n"
			"	gl_Position = vp_matrix * vs_position;							\n"
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
			"	color = vec4(0.0, 1.0, 0.0, 1.0);								\n"
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
		vmath::vec3 camera_position(0.0f, 15.0f, 15.0f);
		UpdateCameraViewMatrix(camera_position);
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
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

#pragma region Object

	void InitializeObject()
	{
		const char filename[] = "C:/workspace/sb7tutorials/resources/media/objects/dragon.sbm";
		object_.load(filename);
	}

	void DestroyObject()
	{
		object_.free();
	}

#pragma endregion

#pragma region Clip plane

	void TraslateClipPlane(float d)
	{
		clip_plane_ += vmath::vec4(0.0f, 0.0f, 0.0f, d);
	}

#pragma endregion

private:

	GLuint render_program_;

	vmath::mat4 camera_view_matrix_;
	vmath::mat4 camera_projection_matrix_;

	sb7::object object_;

	vmath::vec4 clip_plane_;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);

/*
* Warning!
* Rendering program (actually attached shaders) does not implement any lighting model, so that resulting image is not enough good looking.
*/