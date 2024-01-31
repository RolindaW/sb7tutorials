// Include the "sb7.h" header file
#include "sb7.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializeProgram();
		QueryProgramOutputs();
		InitializeVao();  // Not really necessary to query program interface resources
	}

	void render(double currentTime)
	{
		// Clear color buffer
		const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(program);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void shutdown()
	{
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
	}

private:
	void InitializeProgram()
	{
		// Vertex shader
		const char* vertexShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Declare a hard-coded array of positions						\n"
			"	const vec4 vertices[3] = vec4[3](vec4(-0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0)); \n"
			"																	\n"
			"	// Index into our array using gl_VertexID						\n"
			"	gl_Position = vertices[gl_VertexID];							\n"
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
			"layout (location = 2) out ivec2 data;								\n"
			"out float extra;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(0.0, 0.8, 1.0, 1.0);								\n"
			"	data = ivec2(7, -3);											\n"
			"	extra = 4.7;													\n"
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

	void QueryProgramOutputs()
	{
		// Query the number of active outputs (and the size of the longest variable name) in the program
		GLint activeOutputsNumber;
		glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &activeOutputsNumber);

		GLint activeOutputsMaxNameLength;
		glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_MAX_NAME_LENGTH, &activeOutputsMaxNameLength);

		// Query specific information about active outputs within the program
		const GLenum properties[] = { GL_TYPE, GL_LOCATION };

		GLint parameters[2];  // Same size as properties
		GLchar name[64];  // Same size as activeOutputsMaxNameLength
		const char* typeName;
		char output[256];

		for (GLint i = 0; i < activeOutputsNumber; i++)
		{
			// Query the name of the output
			glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, i, sizeof(name), NULL, name);

			// Query desired properties of the output
			glGetProgramResourceiv(program, GL_PROGRAM_OUTPUT, i, 2, properties, sizeof(parameters), NULL, parameters);

			typeName = TypeToName(parameters[0]);
			
			// Print to Output (on Debug mode)
			sprintf_s(output, sizeof(output), "Index %d: %s %s @ location %d.\n", i, typeName, name, parameters[1]);
			OutputDebugStringA(output);
		}
	}

	const char* TypeToName(GLint type)
	{
		const char* retval;

		switch (type)
		{
		case GL_FLOAT_VEC4:
			retval = "vec4";
			break;
		case GL_INT_VEC2:
			retval = "ivec2";
			break;
		case GL_FLOAT:
			retval = "float";
			break;
		default:
			retval = "???";
			break;
		}

		return retval;
	}

	void InitializeVao()
	{
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

private:
	GLuint program;
	GLuint vao;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);