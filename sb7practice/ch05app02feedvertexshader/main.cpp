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

		buffer = create_buffers();

		// Tell OpenGL which vertex buffer binding to use for this attribute (when specified vertex array object is bound)
		glVertexArrayAttribBinding(vao, 0, 0);

		// Tell OpenGL which (vertex) buffer object our data is in and where in that buffer object the data resides
		glVertexArrayVertexBuffer(vao, 0, buffer, 0, sizeof(float) * 4);

		// Tell OpenGL what the format of the attribute is
		glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, 0);

		// Enable automatic attribute feeding
		glEnableVertexArrayAttrib(vao, 0);
	}

	void render(double currentTime)
	{
		// Simply clear the window with gren color
		static const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// Use the program object we created earlier for rendering
		glUseProgram(rendering_program);

		// Vertex attribute static value
		static const GLfloat material_color[] = { 0.0f, 0.8f, 1.0f, 1.0f };

		glVertexAttrib4fv(1, material_color);

		// Draw one triangle
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void shutdown()
	{
		glDeleteProgram(rendering_program);
		glDeleteBuffers(1, &buffer);
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
			"layout (location = 0) in vec4 position;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	gl_Position = position;											\n"
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

	// Buffers creation
	GLuint create_buffers(void)
	{
		// Create a buffer object (and corresponding name)
		GLuint buffer;
		glCreateBuffers(1, &buffer);

		// Specify the data store parameters for the buffer
		glNamedBufferStorage(buffer,	// Name of the buffer
			1024 * 1024,	// 1 MiB of space - in bytes
			NULL,	// No initial data
			GL_DYNAMIC_STORAGE_BIT);	// Allow map for writting

		// Bind a named buffer to a target
		glBindBuffer(GL_ARRAY_BUFFER, buffer);  // Store vertex data

		// This is the data that we will place into the buffer
		static const float data[] = {
			 0.25, -0.25, 0.5, 1.0,
			-0.25, -0.25, 0.5, 1.0,
			 0.25,  0.25, 0.5, 1.0
		};

		// Put the data into the buffer
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);

		return buffer;
	}

private:
	GLuint rendering_program;
	GLuint buffer;
	GLuint vao;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);