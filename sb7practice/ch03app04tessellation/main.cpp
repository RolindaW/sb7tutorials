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

		// Update the value of vertex attribute 0
		glVertexAttrib4fv(1, v_att_color);

		// Render all polygons in wireframe mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// Draw one triangle
		glDrawArrays(GL_PATCHES, 0, 3);
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
		GLuint control_shader;
		GLuint evaluation_shader;
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

		// Source code for tessellation control shader
		static const GLchar* control_shader_source[] =
		{
			"#version 450 core																\n"
			"																				\n"
			"// Number of control points produced by the control shader						\n"
			"layout (vertices = 3) out;														\n"
			"																				\n"
			"// Declare VS_OUT as an input interface block									\n"
			"in VS_OUT																		\n"
			"{																				\n"
			"	vec4 color;  // Get color from the previous stage							\n"
			"} cs_in[];																		\n"
			"																				\n"
			"// Declare VS_OUT as an output interface block									\n"
			"out VS_OUT																		\n"
			"{																				\n"
			"	vec4 color;  // Send color to the next stage								\n"
			"} cs_out[];																	\n"
			"																				\n"
			"void main(void)																\n"
			"{																				\n"
			"	// Calculate tessellation levels only for a single vertex run (efficiency)	\n"
			"	if (gl_InvocationID == 0)													\n"
			"	{																			\n"
			"		gl_TessLevelInner[0] = 5.0;												\n"
			"		gl_TessLevelOuter[0] = 1.0;												\n"
			"		gl_TessLevelOuter[1] = 5.0;												\n"
			"		gl_TessLevelOuter[2] = 3.0;												\n"
			"	}																			\n"
			"																				\n"
			"	// Everybody copies their input to their output								\n"
			"	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;	\n"
			"																				\n"
			"	cs_out[gl_InvocationID].color = cs_in[gl_InvocationID].color;				\n"
			"}																				\n"
		};

		// Source code for tessellation evaluation shader
		static const GLchar* evaluation_shader_source[] =
		{
			"#version 450 core																\n"
			"																				\n"
			"// Tessellation mode															\n"
			"layout (triangles, equal_spacing, cw) in;										\n"
			"																				\n"
			"// Declare VS_OUT as an input interface block									\n"
			"in VS_OUT																		\n"
			"{																				\n"
			"	vec4 color;  // Get color from the previous stage							\n"
			"} es_in[];																		\n"
			"																				\n"
			"// Declare VS_OUT as an output interface block									\n"
			"out VS_OUT																		\n"
			"{																				\n"
			"	vec4 color;  // Send color to the next stage								\n"
			"} es_out[];																	\n"
			"																				\n"
			"void main(void)																\n"
			"{																				\n"
			"	// Calculate position for new generated vertex								\n"
			"	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +						\n"
			"				   gl_TessCoord.y * gl_in[1].gl_Position +						\n"
			"				   gl_TessCoord.z * gl_in[2].gl_Position);						\n"
			"																				\n"
			"	es_out[0].color = es_in[0].color;											\n"
			"}																				\n"
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

		// Create and compile control shader
		control_shader = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(control_shader, 1, control_shader_source, NULL);
		glCompileShader(control_shader);

		// Create and compile evaluation shader
		evaluation_shader = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(evaluation_shader, 1, evaluation_shader_source, NULL);
		glCompileShader(evaluation_shader);

		// Create and compile fragment shader
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// Create program, attach shaders to it, and link it
		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, control_shader);
		glAttachShader(program, evaluation_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		// Delete the shaders as the program has them now
		glDeleteShader(vertex_shader);
		glDeleteShader(control_shader);
		glDeleteShader(evaluation_shader);
		glDeleteShader(fragment_shader);

		return program;
	}

private:
	GLuint rendering_program;
	GLuint vertex_array_object;  // VAO
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);