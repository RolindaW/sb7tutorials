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
		InitializeShader1();
		InitializeShader2();

		// Check
		CheckShader(shader1);
		CheckShader(shader2);

		// Delete + Check
		glDeleteShader(shader1);
		
		bool isDeleteShader1 = IsDeleted(shader1);
		bool isDeleteShader2 = IsDeleted(shader2);

		glDeleteShader(shader2);
	}

	void render(double currentTime)
	{
		// Simply clear the window with red
		static const GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, red);
	}

private:
	void InitializeShader1()
	{
		const char* shaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) out vec4 color;								\n"
			"																	\n"
			"uniform scale;														\n"
			"uniform vec3 bias;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(1.0, 0.5, 0.2, 1.0) * scale + bias;				\n"
			"}																	\n"
		};

		shader1 = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(shader1, 1, shaderSource, NULL);
		glCompileShader(shader1);
	}

	void InitializeShader2()
	{
		const char* shaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) out vec4 color;								\n"
			"																	\n"
			"uniform vec4 scale;												\n"
			"uniform vec4 bias;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(1.0, 0.5, 0.2, 1.0) * scale + bias;				\n"
			"}																	\n"
		};

		shader2 = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(shader2, 1, shaderSource, NULL);
		glCompileShader(shader2);
	}

	void CheckShader(GLuint shader)
	{
		std::string type = GetType(shader);
		unsigned int sourceLength = GetSourceLength(shader);

		if (!IsCompiled(shader))
		{
			std::string infoLog = GetInfoLog(shader);
		}
	}

private:
	std::string GetType(GLuint shader)
	{
		GLint shaderType;
		glGetShaderiv(shader, GL_SHADER_TYPE, &shaderType);

		return TypeToStr((GLenum)shaderType);
	}

	std::string TypeToStr(GLenum type)
	{
		switch (type)
		{
		case GL_VERTEX_SHADER:
			return std::string("VERTEX SHADER");
			break;
		case GL_TESS_CONTROL_SHADER:
			return std::string("TESSELLATION CONTROL SHADER");
			break;
		case GL_TESS_EVALUATION_SHADER:
			return std::string("TESSELLATION EVALUATION SHADER");
			break;
		case GL_GEOMETRY_SHADER:
			return std::string("GEOMETRY SHADER");
			break;
		case GL_FRAGMENT_SHADER:
			return std::string("FRAGMENT SHADER");
			break;
		case GL_COMPUTE_SHADER:
			return std::string("COMPUTE SHADER");
			break;
		default:
			return std::string();
			break;
		}
	}

	bool IsCompiled(GLuint shader)
	{
		GLint compileStatus;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

		return compileStatus == GL_TRUE ? true : false;
	}

	bool IsDeleted(GLuint shader)
	{
		GLint deleteStatus;
		glGetShaderiv(shader, GL_DELETE_STATUS, &deleteStatus);

		return deleteStatus == GL_TRUE ? true : false;
	}

	unsigned int GetSourceLength(GLuint shader)
	{
		GLint shaderSourcelength;
		glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &shaderSourcelength);

		return (unsigned int)shaderSourcelength;
	}

	std::string GetInfoLog(GLuint shader)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		std::string infoLog;
		infoLog.resize(infoLogLength);
		glGetShaderInfoLog(shader, infoLogLength, NULL, &infoLog[0]);

		return infoLog;
	}

private:

	GLuint shader1;
	GLuint shader2;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);