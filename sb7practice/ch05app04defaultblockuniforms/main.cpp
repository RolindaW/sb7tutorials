// Include the "sb7.h" header file
#include "sb7.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		rendering_program = compile_shaders();

		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		vbo = create_vbo(vao);
	}

	void render(double currentTime)
	{
		// Simply clear the window with gren color
		static const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// The color of a material
		const GLfloat material_color[3] = { 1.0f, 0.0f, 0.5f };

		// Get the location of the uniform named "material_color" (not recommended approach)
		GLint uniform_location = glGetUniformLocation(rendering_program, "material_color");

		// Set a value to the uniform (from specified program - not required to be bound)
		// Note: Depending on the type of data the uniform represents, its value should be (or not) updated with different frequency.
		//glProgramUniform3fv(rendering_program, uniform_location, 1, material_color);

		// Use the program object we created earlier for rendering
		glUseProgram(rendering_program);

		// Set a value to the uniform (from bound program)
		//glUniform3f(uniform_location, material_color[0], material_color[1], material_color[2]);
		glUniform3fv(uniform_location, 1, material_color);

		// Draw one triangle
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void shutdown()
	{
		glDeleteProgram(rendering_program);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
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
			"layout (location = 0) uniform vec3 material_color;					\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"layout (location = 1) in vec3 color;								\n"
			"																	\n"
			"out vec4 vs_color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Add 4th-component to position - w coordinate					\n"
			"	gl_Position = vec4(position, 1.0);								\n"
			"	// Mix material base color with vertex specific color			\n"
			"	vec3 mixed_color = mix(color, material_color, 0.4);				\n"
			"	// Add 4th-component to color - alpha							\n"
			"	vs_color = vec4(mixed_color, 1.0);								\n"
			"}																	\n"
		};

		// Source code for fragment shader
		static const GLchar* fragment_shader_source[] =
		{
			"#version 450 core							\n"
			"											\n"
			"in vec4 vs_color;							\n"
			"											\n"
			"out vec4 color;							\n"
			"											\n"
			"void main(void)							\n"
			"{											\n"
			"	color = vs_color;						\n"
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

	GLuint create_vbo(GLuint vao)
	{
		static const float data[] = {
			 0.25, -0.25, 0.5, 0.0, 0.8, 1.0,
			-0.25, -0.25, 0.5, 0.8, 0.0, 1.0,
			 0.25,  0.25, 0.5, 1.0, 0.8, 0.0
		};

		// Create buffer object
		GLuint vbo;
		glCreateBuffers(1, &vbo);

		// Allocate memory and copy data
		glNamedBufferStorage(vbo, sizeof(data), data, 0);

		// Bind as a vertex buffer object
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// Set up vertex attribute "position" (location = 0) and enable automatic load
		int vertex_size = sizeof(float) * 3 + sizeof(float) * 3;
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, vertex_size);
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(vao, 0);

		// Set up vertex attribute "color" (location = 1) and enable automatic load
		glVertexArrayAttribBinding(vao, 1, 1);
		glVertexArrayVertexBuffer(vao, 1, vbo, 0, vertex_size);
		glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3);  // Vertex attribute "color" can be found after vertex attribute "position" (3 float values)
		glEnableVertexArrayAttrib(vao, 1);

		return vbo;
	}

private:
	GLuint rendering_program;
	GLuint vao;
	GLuint vbo;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);