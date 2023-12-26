// Include the "sb7.h" header file
#include "sb7.h"
#include "sb7ktx.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializeProgram();
		InitializeVao();
		InitializeTexture();
	}

	void render(double currentTime)
	{
		// Clear the window with gren color
		static const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(program);

		// Simulate offset and update corresponding vertex attribute
		const GLfloat offset[] = { sin(currentTime) * 0.4f,
								   cos(currentTime) * 0.6f,
								   0.0f, 0.0f };;
		glVertexAttrib4fv(0, offset);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void shutdown()
	{
		DestroyProgram();
		DestroyVao();
		DestroyTexture();
	}

private:
	void InitializeProgram()
	{
		// Vertex shader
		const GLchar* vertexShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout(location = 0) in vec4 offset;								\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	const vec4 vertices[3] = vec4[3](vec4( 0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4(-0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0)); \n"
			"																	\n"
			"	gl_Position = vertices[gl_VertexID] + offset;					\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const GLchar* fragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"uniform sampler2D s;												\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = texelFetch(s, ivec2(gl_FragCoord.xy), 0);				\n"
			"}																	\n"
		};

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void InitializeVao()
	{
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	void InitializeTexture()
	{
		// Load texture from file
		const char filename[] = "C:/workspace/sb7tutorials/resources/media/textures/brick.ktx";
		texture = sb7::ktx::file::load(filename);
		if (!texture) return;

		// Bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void DestroyProgram()
	{
		glDeleteProgram(program);
	}

	void DestroyVao()
	{
		glDeleteVertexArrays(1, &vao);
	}

	void DestroyTexture()
	{
		glDeleteTextures(1, &texture);
	}

private:
	GLuint program;
	GLuint vao;
	GLuint texture;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);