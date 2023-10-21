// Include the "sb7.h" header file
#include "sb7.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	// Our rendering function
	void render(double currentTime)
	{
		// Simply clear the window with gren color
		static const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// Use the program object we created earlier for rendering
		glUseProgram(rendering_program);

		// Draw one triangle
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void startup()
	{
		rendering_program = compile_shaders();
		glCreateVertexArrays(1, &vertex_array_object);
		glBindVertexArray(vertex_array_object);
	}

	void shutdown()
	{
		glDeleteProgram(rendering_program);
		glDeleteVertexArrays(1, &vertex_array_object);
	}

private:
	// Rendering program creation
	GLuint compile_shaders(void)
	{
		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint program;

		// Source code for vertex shader
		static const GLchar* vertex_shader_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Declare a hard-coded array of positions						\n"
			"	const vec4 vertices[3] = vec4[3](vec4( 0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4(-0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0)); \n"
			"																	\n"
			"	// Index into our array using gl_VertexID						\n"
			"	gl_Position = vertices[gl_VertexID];							\n"
			"}																	\n"
		};

		// Source code for fragment shader
		static const GLchar* fragment_shader_source[] =
		{
			"#version 450 core							\n"
			"											\n"
			"out vec4 color;							\n"
			"											\n"
			"void main(void)							\n"
			"{											\n"
			"	color = vec4(0.0, 0.8, 1.0, 1.0);		\n"
			"}											\n"
		};

		// Create and compile vertex shader
		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// Create and compile fragment shader
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// Create program, attach shaders to it, and link it
		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		// Delete the shaders as the program has them now
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}

private:
	GLuint rendering_program;
	GLuint vertex_array_object;  // VAO
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);