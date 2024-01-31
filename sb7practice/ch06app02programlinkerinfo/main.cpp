// Include the "sb7.h" header file
#include "sb7.h"
#include <string>

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		// Initialize
		InitializeProgram1();
		InitializeProgram2();

		// Check
		CheckProgram(program1);
		CheckProgram(program2);

		// Delete + Check
		glDeleteProgram(program1);

		bool isDeleteProgram1 = IsDeleted(program1);
		bool isDeleteProgram2 = IsDeleted(program2);

		glDeleteProgram(program2);
	}

	// Our rendering function
	void render(double currentTime)
	{
		// Simply clear the window with red
		static const GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, red);
	}

private:
	void InitializeProgram1()
	{
		const char* shaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) out vec4 color;								\n"
			"																	\n"
			"vec3 myFunction();													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(myFunction(), 1.0);								\n"
			"}																	\n"
		};

		GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(shader, 1, shaderSource, NULL);
		glCompileShader(shader);

		program1 = glCreateProgram();
		glAttachShader(program1, shader);
		glLinkProgram(program1);

		glDeleteShader(shader);
	}

	void InitializeProgram2()
	{
		const char* shaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) out vec4 color;								\n"
			"																	\n"
			"vec3 myFunction()													\n"
			"{																	\n"
			"	return vec3(1.0, 1.0, 0.0);										\n"
			"}																	\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(myFunction(), 1.0);								\n"
			"}																	\n"
		};

		GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(shader, 1, shaderSource, NULL);
		glCompileShader(shader);

		program2 = glCreateProgram();
		glAttachShader(program2, shader);
		glLinkProgram(program2);

		glDeleteShader(shader);
	}

	void CheckProgram(GLuint program)
	{
		unsigned int attachedShaders =  GetAttachedShaders(program);
		unsigned int activeAttributes = GetActiveAttributes(program);

		if (!IsLinked(program))
		{
			std::string infoLog = GetInfoLog(program);
		}
	}

private:
	bool IsLinked(GLuint program)
	{
		GLint linkStatus;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

		return linkStatus == GL_TRUE ? true : false;
	}

	bool IsDeleted(GLuint program)
	{
		GLint deleteStatus;
		glGetProgramiv(program, GL_DELETE_STATUS, &deleteStatus);

		return deleteStatus == GL_TRUE ? true : false;
	}

	unsigned int GetAttachedShaders(GLuint program)
	{
		GLint attachedShaders;
		glGetProgramiv(program, GL_ATTACHED_SHADERS, &attachedShaders);

		return (unsigned int)attachedShaders;
	}

	unsigned int GetActiveAttributes(GLuint program)
	{
		GLint activeAttributes;
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &activeAttributes);

		return (unsigned int)activeAttributes;
	}

	std::string GetInfoLog(GLuint program)
	{
		unsigned int infoLogLength = GetInfoLogLength(program);

		std::string infoLog;
		infoLog.resize(infoLogLength);
		glGetProgramInfoLog(program, infoLogLength, NULL, &infoLog[0]);

		return infoLog;
	}

	unsigned int GetInfoLogLength(GLuint program)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		return (unsigned int)infoLogLength;
	}

private:

	GLuint program1;
	GLuint program2;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);