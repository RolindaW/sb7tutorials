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

		// Calculate offset to generate oval shape motion
		// Scalar multipliers for sin() and cos() functions results is just for fitting vertices position into screen visible area
		GLfloat v_att_offset[] = { sin(currentTime) * 0.5f,
								   cos(currentTime) * 0.3f,
								   0.0f, 0.0f };

		// Update the value of vertex attribute 0
		glVertexAttrib4fv(0, v_att_offset);

		// Define a color for the triangle
		const GLfloat v_att_color[] = { 0.0f, 0.8f, 1.0f, 1.0f };

		// Update the value of vertex attribute 1
		glVertexAttrib4fv(1, v_att_color);

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
			"#version 450 core														\n"
			"																		\n"
			"// 'offset' and 'color' are input vertex attributes					\n"
			"layout (location = 0) in vec4 offset;									\n"
			"layout (location = 1) in vec4 color;									\n"
			"																		\n"
			"// Declare VS_OUT as an output interface block							\n"
			"out VS_OUT																\n"
			"{																		\n"
			"	vec4 color;  // Send color to the next stage						\n"
			"} vs_out;																\n"
			"																		\n"
			"void main(void)														\n"
			"{																		\n"
			"	const vec4 vertices[3] = vec4[3](vec4( 0.25, -0.25, 0.5, 1.0),		\n"
			"									 vec4(-0.25, -0.25, 0.5, 1.0),		\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0));		\n"
			"																		\n"
			"	// Add 'offset' to our hard-coded vertex position					\n"
			"	gl_Position = vertices[gl_VertexID] + offset;						\n"
			"																		\n"
			"	// Output a fixed value for 'vs_out.color'							\n"
			"	vs_out.color = color;												\n"
			"}																		\n"
		};

		// Source code for fragment shader
		static const GLchar* fragment_shader_source[] =
		{
			"#version 450 core																\n"
			"																				\n"
			"// Declare VS_OUT as an input interface block									\n"
			"in VS_OUT																		\n"
			"{																				\n"
			"	vec4 color;  // Get color from the previous stage							\n"
			"} fs_in;																		\n"
			"																				\n"
			"// Output to the framebuffer													\n"
			"out vec4 color;																\n"
			"																				\n"
			"void main(void)																\n"
			"{																				\n"
			"	// Simply assign the color we were given by the vertex shader to our output	\n"
			"	color = fs_in.color;														\n"
			"}																				\n"
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