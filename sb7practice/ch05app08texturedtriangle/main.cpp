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
		InitializeTexture(2);
	}

	void render(double currentTime)
	{
		// Clear the window with gren color
		static const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(program);

		glDrawArrays(GL_TRIANGLES, 0, 6);
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
			"out vec2 uv;														\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	const vec4 vertices[6] = vec4[6](vec4( 0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4(-0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0),	\n"
			"									 vec4(-0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4(-0.25,  0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0)); \n"
			"																	\n"
			"	const vec2 uvs[6] = vec2[6](vec2( 1.0, 0.0),					\n"
			"								vec2( 0.0, 0.0),					\n"
			"								vec2( 1.0, 1.0),					\n"
			"								vec2( 0.0, 0.0),					\n"
			"								vec2( 0.0, 1.0),					\n"
			"								vec2( 1.0, 1.0));					\n"
			"																	\n"
			"	gl_Position = vertices[gl_VertexID];							\n"
			"	uv = uvs[gl_VertexID];											\n"
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
			"in vec2 uv;														\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	//color = texelFetch(s, ivec2(gl_FragCoord.xy), 0);				\n"
			"	//color = texture(s, vec2(3*uv.x, 2*uv.y)).rrra;				\n"
			"	//color = texture(s, 3*uv).rrra;								\n"
			"	//color = texture(s, uv).rrra;									\n"
			"	color = texture(s, uv);											\n"
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

	void InitializeTexture(int mode)
	{
		switch (mode)
		{
		case 0:
			InitializeTextureExplicitGrayscaleColumns();
			break;
		case 1:
			InitializeTextureExplicitGrayscaleRows();
			break;
		case 2:
			InitializeTextureExplicitGrayscaleSquare();
			break;
		default:
			InitializeTextureKtx();
			break;
		}
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

	void InitializeTextureExplicitGrayscaleColumns()
	{
		// Create a new 2D texture object
		glCreateTextures(GL_TEXTURE_2D, 1, &texture);

		// Specify the amount of storage we want to use for the texture
		glTextureStorage2D(texture,
			1,
			GL_R8,
			5, 1);

		// Define some data to upload into the texture
		// Note: Data is laid out (this setup can be changed in OpenGL with a parameter) left to right, top to bottom.
		static const GLubyte data[] =
		{
			0x00, 0x40, 0x80, 0xB0, 0xFF
		};

		static GLubyte data2[5][1][1];
		data2[0][0][0] = 0x00;
		data2[1][0][0] = 0x40;
		data2[2][0][0] = 0x80;
		data2[3][0][0] = 0xB0;
		data2[4][0][0] = 0xFF;

		// Fill memory with image data
		glTextureSubImage2D(texture,
			0,
			0, 0,
			5, 1,
			GL_RED,
			GL_UNSIGNED_BYTE,
			data);

		// Wrapping
		// Note: Even if supplied texture coordinates fall inside expected range, wrapping is also performed when applying linear interpolation filtering on magnification for those coordinates between border and outer texels centre.
		
		// GL_REPEAT: Smooth change to white on left side; Smooth change to black on right side.
		// GL_MIRRORED_REPEAT: Darken on left side; Lighten on right side.
		// GL_CLAMP_TO_EDGE: Same to GL_MIRRORED_REPEAT.
		// GL_CLAMP_TO_BORDER (red border - borderColor[] = { 1.0f, 0.0f, 0.0f, 1.0f }): Smooth change to white on left side; Lighten on right side.
		// GL_CLAMP_TO_BORDER (black border - borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f }): Darken on left side; Smooth change to black on right side.
		glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // GL_REPEAT - GL_MIRRORED_REPEAT - GL_CLAMP_TO_EDGE - GL_CLAMP_TO_BORDER
		glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);  // GL_REPEAT (=GL_MIRRORED_REPEAT; =GL_CLAMP_TO_EDGE) - GL_CLAMP_TO_BORDER

		// Border color
		const GLfloat borderColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Filtering
		glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // GL_NEAREST - GL_LINEAR (affects wrapping)
		glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Dont apply

		// Swizzle (i.e. source) G and B channels from channel R (it is possible to perform it in shader code - e.g. "color = texture(s, uv).rrra;" - but constrains shader reuse).
		glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_B, GL_RED);

		// Bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void InitializeTextureExplicitGrayscaleRows()
	{
		// Create a new 2D texture object
		glCreateTextures(GL_TEXTURE_2D, 1, &texture);

		// Specify the amount of storage we want to use for the texture
		glTextureStorage2D(texture,
			1,
			GL_R8,
			1, 5);

		// Define some data to upload into the texture
		static const GLubyte data[] =
		{
			0x00, 0x40, 0x80, 0xB0, 0xFF
		};

		// Note: Be careful with pixel store...
		/*
		static const GLubyte data[] =
		{
			0xFF,  // 0 - Bottom
			0x00,
			0x00,
			0x00,

			0xB0,  // 1
			0x00,
			0x00,
			0x00,

			0x80,  // 2
			0x00,
			0x00,
			0x00,

			0x40,  // 3
			0x00,
			0x00,
			0x00,

			0x00,  // 4 - Top
			0x00,
			0x00,
			0x00
		};
		*/

		static GLubyte data2[1][5][1];
		data2[0][0][0] = 0x00;
		data2[0][1][0] = 0x40;
		data2[0][2][0] = 0x80;
		data2[0][3][0] = 0xB0;
		data2[0][4][0] = 0xFF;

		// Pixel store (next available byte is used; not memory waste)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Fill memory with image data
		glTextureSubImage2D(texture,
			0,
			0, 0,
			1, 5,
			GL_RED,
			GL_UNSIGNED_BYTE,
			data);

		// Wrapping
		glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);  // GL_REPEAT - GL_MIRRORED_REPEAT - GL_CLAMP_TO_EDGE - GL_CLAMP_TO_BORDER
		glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Border color
		const GLfloat borderColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Filtering
		glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // GL_NEAREST - GL_LINEAR
		glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Swizzle
		glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_B, GL_RED);

		// Bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void InitializeTextureExplicitGrayscaleSquare()
	{
		// Create a new 2D texture object
		glCreateTextures(GL_TEXTURE_2D, 1, &texture);

		// Specify the amount of storage we want to use for the texture
		glTextureStorage2D(texture,
			1,
			GL_R8,
			2, 2);

		// Define some data to upload into the texture
		// Note: Data is laid out (this setup can be changed in OpenGL with a parameter) left to right, top to bottom.
		static const GLubyte data[] =
		{
			0xFF, 0x00,
			0x00, 0xFF
		};

		static GLubyte data2[2][2][1];
		data2[0][0][0] = 0xFF;
		data2[0][1][0] = 0x00;
		data2[1][0][0] = 0x00;
		data2[1][1][0] = 0xFF;

		// Pixel store
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Fill memory with image data
		glTextureSubImage2D(texture,
			0,
			0, 0,
			2, 2,
			GL_RED,
			GL_UNSIGNED_BYTE,
			data);

		// Wrapping
		glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);  // GL_REPEAT - GL_MIRRORED_REPEAT - GL_CLAMP_TO_EDGE - GL_CLAMP_TO_BORDER
		glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Border color
		const GLfloat borderColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Filtering
		glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // GL_NEAREST - GL_LINEAR
		glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Swizzle
		glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_B, GL_RED);

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