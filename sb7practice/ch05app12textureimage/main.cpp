// Include the "sb7.h" header file
#include "sb7.h"
#include "sb7ktx.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializePrograms();
		InitializeObject();
		InitializeTextures();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// Use texturing program to process input texture
		glUseProgram(texturingProgram);

		glBindTexture(GL_TEXTURE_2D, inputTexture);
		glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);;

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
		// Use rendering program to draw processed texture
		glUseProgram(renderingProgram);

		glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void shutdown()
	{
		glDeleteProgram(texturingProgram);
		glDeleteProgram(renderingProgram);

		glDeleteVertexArrays(1, &vao);

		glDeleteTextures(1, &inputTexture);
		glDeleteTextures(1, &outputTexture);
	}

private:
	/// <summary>
	/// Fragment shader code sample taken from the book uses fragment coordinate (screen space) to access either input and output texture images.
	/// Because of this, in order to fully copy (inverted) input to output image, it is a must to generate (actually vertex-postprocessing fixed-function stage does this) at least required fragments whose position let access all existing texels of input image.
	/// 
	/// To do so, it is necessary to take into account...
	/// - Window/Viewport size: (800, 600)
	/// - Texture size: (512, 512)
	/// - Bottom/Left position: (-1, -1)
	/// - Top/Right position: TBD (x, y)
	/// ... so we can properly define a square geometry in clip space (actually in NDC space as homogeneous coordinate will be 1.0).
	/// 
	/// Define the top/right position of the square geometry: (x, y)
	/// Based on similar triangles principle:
	/// 800 / 2 = 512 / x -> x = (512 / 800) * 2 = 1.28
	/// 600 / 2 = 512 / y -> y = (512 / 600) * 2 = 1.107
	/// And, we need to apply an offset of -1 (as the size of the x and y axes in NDC space ranges from -1 to +1 - so we are using full range value 2 in the calculations)
	/// x1 = x - 1 = 1.28 - 1 = 0.28
	/// y1 = y - 1 = 1.707 - 1 = 0.707
	/// -> (0.28, 0.707)
	/// 
	/// * ------- *
	/// * -- *	  |
	/// |	 |	  |
	/// * -- * -- *
	/// 
	/// As it can be noticed, code supplied by the book to read/write from/to texture image is not useful enough; we actually need to force a very specific use case sceneario to make it work.
	/// </summary>
	void InitializePrograms()
	{
		// Vertex shader
		const GLchar* vertexShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	const vec4 vertices[6] = vec4[6](vec4(-1, -1, 0.5, 1.0),		\n"
			"									 vec4( 0.28, -1, 0.5, 1.0),		\n"
			"									 vec4( 0.28,  0.707, 0.5, 1.0),	\n"
			"									 vec4( 0.28,  0.707, 0.5, 1.0),	\n"
			"									 vec4(-1,  0.707, 0.5, 1.0),	\n"
			"									 vec4(-1, -1, 0.5, 1.0));		\n"
			"																	\n"
			"	gl_Position = vertices[gl_VertexID];							\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader: texturing
		const GLchar* texturingFragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"// Input image - note use of format qualifier because of loads		\n"
			"layout (binding = 0) uniform sampler2D s;							\n"
			"// Output image													\n"
			"layout (binding = 0) writeonly uniform image2D image_out;			\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Use fragment coordinate as image coordinate					\n"
			"	ivec2 uv = ivec2(gl_FragCoord.xy);								\n"
			"																	\n"
			"	// Read data from (input) texture sampler						\n"
			"	vec4 data = texelFetch(s, uv, 0);								\n"
			"																	\n"
			"	// Save data into (output) texture image						\n"
			"	//imageStore(image_out, uv, ~data);								\n"
			"	imageStore(image_out, uv, data.abgr);							\n"
			"	memoryBarrierImage();											\n"
			"}																	\n"
		};

		GLuint texturingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(texturingFragmentShader, 1, texturingFragmentShaderSource, NULL);
		glCompileShader(texturingFragmentShader);

		texturingProgram = glCreateProgram();
		glAttachShader(texturingProgram, vertexShader);
		glAttachShader(texturingProgram, texturingFragmentShader);
		glLinkProgram(texturingProgram);

		// Fragment shader: rendering
		const GLchar* renderingFragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (binding = 0, rgba8) readonly uniform image2D image_in;		\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	ivec2 uv = ivec2(gl_FragCoord.xy);								\n"
			"																	\n"
			"	color = imageLoad(image_in, uv);								\n"
			"}																	\n"
		};

		GLuint renderingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(renderingFragmentShader, 1, renderingFragmentShaderSource, NULL);
		glCompileShader(renderingFragmentShader);

		renderingProgram = glCreateProgram();
		glAttachShader(renderingProgram, vertexShader);
		glAttachShader(renderingProgram, renderingFragmentShader);
		glLinkProgram(renderingProgram);

		// Free resources
		glDeleteShader(vertexShader);
		glDeleteShader(texturingFragmentShader);
		glDeleteShader(renderingFragmentShader);
	}

	void InitializeObject()
	{
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	void InitializeTextures()
	{
		InitializeInputTexture();
		InitializeOutputTexture();
	}

	void InitializeInputTexture()
	{
		const char filename[] = "C:/workspace/sb7tutorials/resources/media/textures/brick.ktx";
		inputTexture = sb7::ktx::file::load(filename);
	}

	/// <summary>
	/// Notice there is not any compatible format type for 24 bit (GL_RGB8) image - input image.
	/// It is required to define the output texture as a 32 bit (GL_RGBA) image.
	/// Furthermore, it is required to use a texture sampler to read input image data from instead of a texture image (see overrided rendering method)
	/// </summary>
	void InitializeOutputTexture()
	{
		// Create a new 2D texture object
		glCreateTextures(GL_TEXTURE_2D, 1, &outputTexture);

		// Specify the amount of storage we want to use for the texture
		glTextureStorage2D(outputTexture,
			1,
			GL_RGBA8,
			512, 512);
	}

private:
	GLuint texturingProgram;
	GLuint renderingProgram;
	GLuint vao;

	GLuint inputTexture;
	GLuint outputTexture;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);