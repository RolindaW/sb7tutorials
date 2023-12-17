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
			"void main(void)													\n"
			"{																	\n"
			"	const vec4 vertices[3] = vec4[3](vec4( 0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4(-0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0)); \n"
			"																	\n"
			"	gl_Position = vertices[gl_VertexID];							\n"
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
		bool ktx = true;
		if (ktx) InitializeTextureKtx();
		else InitializeTextureScratch();
	}

	void InitializeTextureKtx()
	{	
		// Load texture from file
		const char filename[] = "C:/workspace/sb7tutorials/resources/media/textures/brick.ktx";
		texture = sb7::ktx::file::load(filename);
		if (!texture) return;

		// Bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void InitializeTextureScratch()
	{
		// Create a new 2D texture object
		glCreateTextures(GL_TEXTURE_2D, 1, &texture);

		// Specify the amount of storage we want to use for the texture
		glTextureStorage2D(texture,
							1,
							GL_RGBA32F,
							256, 256);

		// Update texture data

		// Define some data to upload into the texture
		float* data = new float[256 * 256 * 4];

		// Fill memory with image data
		Generate2DRGBATexture(data, 256, 256);

		glTextureSubImage2D(texture,
							0,
							0, 0,
							256, 256,
							GL_RGBA,
							GL_FLOAT,
							data);

		// Free the memory we allocated before - OpenGL now has our data
		delete[] data;

		// Bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void Generate2DRGBATexture(float* data, int width, int height)
	{
		int size = width * height;
		for (int i = 0; i < size; i++)
		{
			int indexBase = i * 4;  // Times number of channels
			data[indexBase] = 1.0f;		// R
			data[indexBase + 1] = 0.5f;	// G
			data[indexBase + 2] = 0.5f;	// B
			data[indexBase + 3] = 1.0f;	// A
		}
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