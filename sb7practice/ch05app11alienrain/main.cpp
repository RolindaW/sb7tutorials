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

		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		InitializeTexture();
	}

	void render(double currentTime)
	{
		// Clear the color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(program);

		// Update active texture
		// TODO: Only on activeTextureIndex value change
		glVertexAttribI1ui(0, activeTextureIndex);

		// Draw texture
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void shutdown()
	{
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
		glDeleteTextures(1, &texture);
	}

public:
	void onKey(int key, int action)
	{
		sb7::application::onKey(key, action);

		// Check keyboard arrows to...
		// - move camera foward/backward
		// - switch (left) texture sampling mode
		switch (key)
		{
		case GLFW_KEY_UP:
			if (action)
			{
				IncreaseTextureIndex();
			}
			break;
		case GLFW_KEY_DOWN:
			if (action)
			{
				DecreaseTextureIndex();
			}
			break;
		default:
			break;
		}
	}

private:
	void InitializeProgram()
	{
		// Vertex shader
		const GLchar* vertexShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) in uint alien_index;							\n"
			"																	\n"
			"out ALIEN															\n"
			"{																	\n"
			"	flat uint index;												\n"
			"	vec2 uv;														\n"
			"} vs_out;															\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	const vec4 vertices[6] = vec4[6](vec4(-0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0),	\n"
			"									 vec4(-0.25,  0.25, 0.5, 1.0),	\n"
			"									 vec4(-0.25, -0.25, 0.5, 1.0)); \n"
			"																	\n"
			"	const vec2 uvs[6] = vec2[6](vec2( 0.0, 0.0),					\n"
			"								vec2( 1.0, 0.0),					\n"
			"								vec2( 1.0, 1.0),					\n"
			"								vec2( 1.0, 1.0),					\n"
			"								vec2( 0.0, 1.0),					\n"
			"								vec2( 0.0, 0.0));					\n"
			"																	\n"
			"	gl_Position = vertices[gl_VertexID];							\n"
			"																	\n"
			"	vs_out.index = alien_index;										\n"
			"	vs_out.uv = uvs[gl_VertexID];									\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const GLchar* fragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (binding = 0) uniform sampler2DArray s;						\n"
			"																	\n"
			"in ALIEN															\n"
			"{																	\n"
			"	flat uint index;												\n"
			"	vec2 uv;														\n"
			"} fs_in;															\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = texture(s, vec3(fs_in.uv, float(fs_in.index)));			\n"
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

	void InitializeTexture()
	{
		const char filename[] = "C:/workspace/sb7tutorials/resources/media/textures/aliens.ktx";
		texture = sb7::ktx::file::load(filename);
	}

	void IncreaseTextureIndex()
	{
		if (activeTextureIndex < TEXTURE_COUNT - 1)
		{
			activeTextureIndex++;
		}
	}

	void DecreaseTextureIndex()
	{
		if (activeTextureIndex > 0)
		{
			activeTextureIndex--;
		}
	}

private:
	GLuint program;
	GLuint vao;

	GLuint texture;
	unsigned int activeTextureIndex;
	const unsigned int TEXTURE_COUNT = 64;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);