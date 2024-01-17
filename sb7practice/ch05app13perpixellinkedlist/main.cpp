// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"
#include "object.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializePrograms();
		InitializeCamera();
		InitializeObject();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(fillingProgram);

		glUniformMatrix4fv(0, 1, GL_FALSE, viewMatrix * modelWorldMatrix);
		glUniformMatrix4fv(1, 1, GL_FALSE, projectionMatrix);

		object.render();
	}

	void shutdown()
	{
		DestroyPrograms();
		DestroyObject();
	}

public:
	void onKey(int key, int action)
	{
		sb7::application::onKey(key, action);

		// Check keyboard arrows to...
		// - move camera foward/backward
		// - switch (left) texture sampling mode
		switch (key)
		{
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
	void InitializePrograms()
	{
		// Vertex shader
		const GLchar* vertexShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 mv;								\n"
			"layout (location = 1) uniform mat4 proj;							\n"
			"																	\n"
			"layout (location = 0) in vec4 position;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	gl_Position = proj * mv * position;								\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader: texturing
		const GLchar* fillingFragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(0.0, 1.0, 0.0, 1.0);								\n"
			"}																	\n"
		};

		GLuint fillingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fillingFragmentShader, 1, fillingFragmentShaderSource, NULL);
		glCompileShader(fillingFragmentShader);

		fillingProgram = glCreateProgram();
		glAttachShader(fillingProgram, vertexShader);
		glAttachShader(fillingProgram, fillingFragmentShader);
		glLinkProgram(fillingProgram);

		// Fragment shader: rendering
		const GLchar* traversingFragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(0.0, 1.0, 0.0, 1.0);								\n"
			"}																	\n"
		};

		GLuint traversingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(traversingFragmentShader, 1, traversingFragmentShaderSource, NULL);
		glCompileShader(traversingFragmentShader);

		traversingProgram = glCreateProgram();
		glAttachShader(traversingProgram, vertexShader);
		glAttachShader(traversingProgram, traversingFragmentShader);
		glLinkProgram(traversingProgram);

		// Free resources
		glDeleteShader(vertexShader);
		glDeleteShader(fillingFragmentShader);
		glDeleteShader(traversingFragmentShader);
	}

	void InitializeCamera()
	{
		// View
		vmath::vec3 cameraPosition(0.0f, 7.5f, 20.0f);
		vmath::vec3 target(0.0f, 5.0f, 0.0f);
		vmath::vec3 up(0.0f, 1.0f, 0.0f);
		viewMatrix = vmath::lookat(cameraPosition, target, up);

		// Projection
		float fov = 45.0f;
		float aspect = (float)info.windowWidth / (float)info.windowHeight;
		float n = 0.1f, f = 1000.0f;

		projectionMatrix = vmath::perspective(fov, aspect, n, f);
	}

	void InitializeObject()
	{
		const char filename[] = "C:/workspace/sb7tutorials/resources/media/objects/dragon.sbm";
		object.load(filename);

		modelWorldMatrix = vmath::rotate(0.0f, 125.0f, 0.0f);
	}

	void RotateObject(int ccw)
	{
		modelWorldMatrix = vmath::rotate(0.0f, ccw * objectRotationYStep, 0.0f) * modelWorldMatrix;
	}

	void DestroyPrograms()
	{
		glDeleteProgram(fillingProgram);
		glDeleteProgram(traversingProgram);
	}

	void DestroyObject()
	{
		object.free();
	}
private:
	GLuint fillingProgram;
	GLuint traversingProgram;

	sb7::object object;
	vmath::mat4 modelWorldMatrix;
	const float objectRotationYStep = 15.0f;

	vmath::mat4 viewMatrix;
	vmath::mat4 projectionMatrix;
};

// TODO: Update fragment shader code and implement correspoding GL code

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);