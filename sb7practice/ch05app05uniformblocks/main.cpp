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
		ubo = create_ubo(rendering_program);
	}

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

	void shutdown()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &ubo);
		glDeleteProgram(rendering_program);
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
			"layout (std140) uniform StdLayoutMaterialUniformBlock				\n"
			"{																	\n"
			"	vec3 color;														\n"
			"} material;														\n"
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
			"	// Mix material color with vertex specific color				\n"
			"	vec3 mixed_color = mix(color, material.color, 0.4);				\n"
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

	GLuint create_ubo(GLuint program)
	{
		const GLfloat material_color[] = { 1.0f, 0.0f, 0.5f };

		GLuint ubo;
		glCreateBuffers(1, &ubo);

		// Note: This is a very simple use case (vec3 type data), so allocated memory size and (each) data position (offset) calculation is not complex.
		glNamedBufferStorage(ubo, sizeof(material_color), material_color, NULL);

		GLuint uniform_index = glGetUniformBlockIndex(program, "StdLayoutMaterialUniformBlock");

		// Assign a binding point (by index) to the uniform (it can also be specified in the shader code with "layout (binding = n)" qualifier).
		const GLuint binding_point_index = 0;
		glUniformBlockBinding(program, uniform_index, binding_point_index);

		// Bind the uniform buffer object to the binding point.
		glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, ubo);

		return ubo;
	}

private:
	GLuint rendering_program;
	GLuint vao;
	GLuint vbo;
	GLuint ubo;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);