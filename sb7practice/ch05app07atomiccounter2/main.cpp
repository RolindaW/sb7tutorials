// Include the "sb7.h" header file
#include "sb7.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializeProgram();

		InitializeAtomicCounter();

		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
	void render(double currentTime)
	{
		// Simply clear the window with red
		static const GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, red);

		glUseProgram(program);

		// Reset atomic counter
		GLuint* acboData = (GLuint*)glMapNamedBufferRange(acbo,
															0, sizeof(GLuint),
															GL_MAP_WRITE_BIT);
		*acboData = (GLuint)0;
		glUnmapNamedBuffer(acbo);

		glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		// Read the value of the atomic counter
		acboData = (GLuint*)glMapNamedBufferRange(acbo,
													0, sizeof(GLuint),
													GL_MAP_READ_BIT);

		glUnmapNamedBuffer(acbo);
	}
	void shutdown()
	{
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &acbo);
	}
private:
	void InitializeProgram()
	{
		// Vertex shader
		const GLchar* vertexShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	const vec4 vertices[3] = vec4[3](vec4( 0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4(-0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0)); \n"
			"	gl_Position = vertices[gl_VertexID];							\n"
			"}																	\n"
		};
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const GLchar* fragmentShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout(binding = 0, offset = 0) uniform atomic_uint counter;		\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(0.0, 0.8, 1.0, 1.0);								\n"
			"	atomicCounterIncrement(counter);								\n"
			"	memoryBarrierAtomicCounter();									\n"
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

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void InitializeAtomicCounter()
	{
		const GLuint data = (GLuint)3;  // Only to verify atomic counter is reset first rendering cycle
		glCreateBuffers(1, &acbo);
		glNamedBufferStorage(acbo, sizeof(GLuint), &data, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, acbo);
	}
private:
	GLuint program;
	GLuint vao;
	GLuint acbo;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);