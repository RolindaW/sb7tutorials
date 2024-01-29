// Include the "sb7.h" header file
#include "sb7.h"
#include <string>

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializeVertexShaderProgram();
		InitializeFragmentShaderProgram();
		InitializeProgramPipeline();
		InitializeVao();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void shutdown()
	{
		glBindProgramPipeline(0);
		glDeleteProgramPipelines(1, &programPipeline);
		glDeleteProgram(vertexShaderProgram);
		glDeleteProgram(fragmentShaderProgram);
		glDeleteVertexArrays(1, &vao);
	}

private:
	void InitializeVertexShaderProgram()
	{
		const char* vertexShaderSource[] = {
			"#version 450 core																						\n"
			"																										\n"
			"// It is necessary to redeclare the in-use built-in variables inside the gl_PerVertex interface block	\n"
			"out gl_PerVertex																						\n"
			"{																										\n"
			"	vec4 gl_Position;																					\n"
			"};																										\n"
			"																										\n"
			"void main(void)																						\n"
			"{																										\n"
			"	// Declare a hard-coded array of positions															\n"
			"	const vec4 vertices[3] = vec4[3](vec4(-0.25, -0.25, 0.5, 1.0),										\n"
			"									 vec4( 0.25, -0.25, 0.5, 1.0),										\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0));										\n"
			"																										\n"
			"	// Index into our array using gl_VertexID															\n"
			"	gl_Position = vertices[gl_VertexID];																\n"
			"}																										\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		vertexShaderProgram = glCreateProgram();
		glAttachShader(vertexShaderProgram, vertexShader);
		glProgramParameteri(vertexShaderProgram, GL_PROGRAM_SEPARABLE, GL_TRUE);  // Set program as separable - before linking!
		glLinkProgram(vertexShaderProgram);

		glDeleteShader(vertexShader);
	}

	void InitializeFragmentShaderProgram()
	{
		const char* fragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(0.0, 0.8, 1.0, 1.0);								\n"
			"}																	\n"
		};

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		fragmentShaderProgram = glCreateProgram();
		glAttachShader(fragmentShaderProgram, fragmentShader);
		glProgramParameteri(fragmentShaderProgram, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glLinkProgram(fragmentShaderProgram);

		glDeleteShader(fragmentShader);
	}

	void InitializeProgramPipeline()
	{
		glCreateProgramPipelines(1, &programPipeline);
		glUseProgramStages(programPipeline, GL_VERTEX_SHADER_BIT, vertexShaderProgram);
		glUseProgramStages(programPipeline, GL_FRAGMENT_SHADER_BIT, fragmentShaderProgram);
		glBindProgramPipeline(programPipeline);
	}

	void InitializeVao()
	{
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

private:
	GLuint vertexShaderProgram;
	GLuint fragmentShaderProgram;
	GLuint programPipeline;
	GLuint vao;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);