// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"
#include "sb7ktx.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializeProgram();
		InitializeCamera();
		InitializeBaseObject();
		InitializeObjectInstances();
		InitializeTextures();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(program);

		RenderObjectInstance(worldMatrixBottom, textureBottom);
		RenderObjectInstance(worldMatrixTop, textureTop);
		RenderObjectInstance(worldMatrixLeft, textureLeft);
		RenderObjectInstance(worldMatrixRight, textureRight);
	}

	void shutdown()
	{
		DeleteProgram();
		DeleteBaseObject();
		DeleteTextures();
	}

public:
	void onResize(int w, int h)
	{
		sb7::application::onResize(w, h);

		// Update viewport
		int pos_x, pos_y;
		glfwGetWindowPos(window, &pos_x, &pos_y);
		glViewport(pos_x, pos_y, info.windowWidth, info.windowHeight);

		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

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
				// Forward
				MoveCamera(-0.5f);
			}
			break;
		case GLFW_KEY_DOWN:
			if (action)
			{
				// Backward
				MoveCamera(0.5f);
			}
			break;
		case GLFW_KEY_A:
			if (action)
			{
				SwapTextureWrappingMode(textureLeft);
			}
			break;
		case GLFW_KEY_S:
			if (action)
			{
				SwapTextureMagnificationFilteringMode(textureLeft);
			}
			break;
		case GLFW_KEY_D:
			if (action)
			{
				SwapTextureMinificationFilteringMode(textureLeft);
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
			"layout (location = 0) uniform mat4 mv;								\n"
			"layout (location = 1) uniform mat4 proj;							\n"
			"																	\n"
			"layout (location = 0) in vec4 position;							\n"
			"layout (location = 1) in vec2 tc;									\n"
			"																	\n"
			"out vec2 uv;														\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	gl_Position = proj * mv * position;								\n"
			"	uv = tc;														\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const GLchar* fragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (binding = 0) uniform sampler2D s;							\n"
			"																	\n"
			"in vec2 uv;														\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Scale texture coordinate on horizontal component				\n"
			"	color = texture(s, vec2(25.0, 1.0) * uv);						\n"
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

	void InitializeCamera()
	{
		// View matrix
		vmath::vec3 startPosition(0.0f, 0.0f, 0.0f);
		UpdateCameraViewMatrix(startPosition);

		// Projection matrix
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void InitializeBaseObject()
	{
		// VAO
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// VBO

		/* Rectangle (X-Y plane; Z=0)
		C*-----*D
		 |     |
		A*-----*B
		*/
		const float data[] =
		{
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // A
			b, 0.0f, 0.0f, 1.0f, 0.0f,  // B
			0.0f, h, 0.0f, 0.0f, 1.0f,  // C

			0.0f, h, 0.0f, 0.0f, 1.0f,  // C
			b, 0.0f, 0.0f, 1.0f, 0.0f,  // B
			b, h, 0.0f, 1.0f, 1.0f  // D
		};

		glCreateBuffers(1, &vbo);
		glNamedBufferStorage(vbo, sizeof(data), data, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// Set up vertex attributes: position, normalized texture coordinates
		int vertexSize = sizeof(float) * 3 + sizeof(float) * 2;

		// Set up vertex attribute "position" (location = 0) and enable automatic load
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, vertexSize);
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(vao, 0);

		// Set up vertex attribute "tc" (location = 1) and enable automatic load
		glVertexArrayAttribBinding(vao, 1, 1);
		glVertexArrayVertexBuffer(vao, 1, vbo, 0, vertexSize);
		glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
		glEnableVertexArrayAttrib(vao, 1);
	}

	void InitializeObjectInstances()
	{
		// Bottom
		worldMatrixBottom = vmath::translate(h / 2.0f, -h / 2.0f, 0.0f) * vmath::rotate(-90.0f, 90.0f, 0.0f);

		// Top
		worldMatrixTop = vmath::translate(-h / 2.0f, h / 2.0f, 0.0f) * vmath::rotate(90.0f, 90.0f, 0.0f);

		// Left
		worldMatrixLeft = vmath::translate(-h / 2.0f, -h / 2.0f, 0.0f) * vmath::rotate(0.0f, 90.0f, 0.0f);

		// Right
		worldMatrixRight = vmath::translate(h / 2.0f, -h / 2.0f, -b) * vmath::rotate(0.0f, -90.0f, 0.0f);
	}

	void InitializeTextures()
	{
		const char filenameBottom[] = "C:/workspace/sb7tutorials/resources/media/textures/floor.ktx";
		textureBottom = sb7::ktx::file::load(filenameBottom);
		SetupTexture(textureBottom, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		const char filenameTop[] = "C:/workspace/sb7tutorials/resources/media/textures/ceiling.ktx";
		textureTop = sb7::ktx::file::load(filenameTop);
		SetupTexture(textureTop, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		const char filenameLeft[] = "C:/workspace/sb7tutorials/resources/media/textures/brick.ktx";
		textureLeft = sb7::ktx::file::load(filenameLeft);
		SetupTexture(textureLeft, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		const char filenameRight[] = "C:/workspace/sb7tutorials/resources/media/textures/brick.ktx";
		textureRight = sb7::ktx::file::load(filenameRight);
		SetupTexture(textureRight, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	}

	void DeleteProgram()
	{
		glDeleteProgram(program);
	}

	void DeleteBaseObject()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}

	void DeleteTextures()
	{
		glDeleteTextures(1, &textureBottom);
		glDeleteTextures(1, &textureTop);
		glDeleteTextures(1, &textureLeft);
		glDeleteTextures(1, &textureRight);
	}

	void RenderObjectInstance(vmath::mat4 worldMatrix, GLuint texture)
	{
		// Calculate and update model-view matrix
		vmath::mat4 modelViewMatrix = cameraViewMatrix * worldMatrix;
		glUniformMatrix4fv(0, 1, GL_FALSE, modelViewMatrix);

		// Update projection matrix (could be actually only performed on camera projection matrix update)
		glUniformMatrix4fv(1, 1, GL_FALSE, cameraProjectionMatrix);

		// Update texture
		glBindTextureUnit(0, texture);

		// Render
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void MoveCamera(float z)
	{
		cameraPosition += vmath::vec3(0.0f, 0.0f, z);
		UpdateCameraViewMatrix(cameraPosition);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		// Calculate view matrix as the inverse of camera model-world matrix - assume only trasnlation
		cameraViewMatrix = vmath::translate(-position);
	}

	void UpdateCameraProjectionMatrix(float viewportWidth, float viewportHeight)
	{
		float fov = 45.0f;
		float aspect = viewportWidth / viewportHeight;
		float n = 0.1f, f = 1000.0f;

		cameraProjectionMatrix = vmath::perspective(fov, aspect, n, f);
	}

	void SetupTexture(GLuint texture, GLint wrapping = GL_ZERO, GLint magnificationFiltering = GL_ZERO, GLint minificationfiltering = GL_ZERO)
	{
		// Wrapping
		if (wrapping)
		{
			glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrapping);
			glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrapping);

			// Border color
			const float color[] = { 1.0f, 1.0f, 0.0f, 1.0f };
			glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, color);
		}

		// Filtering
		if (magnificationFiltering)
		{
			glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, magnificationFiltering);
		}
		
		if (minificationfiltering)
		{
			glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, minificationfiltering);
		}
	}

	void SwapTextureWrappingMode(GLuint texture)
	{
		GLint nextWrappingMode = GetNextWrappingMode(texture);
		SetupTexture(texture, nextWrappingMode);
	}

	GLint GetNextWrappingMode(GLuint texture)
	{
		GLint currentWrappingMode;
		glGetTextureParameteriv(texture, GL_TEXTURE_WRAP_S, &currentWrappingMode);

		switch (currentWrappingMode)
		{
		case GL_REPEAT:
			return GL_MIRRORED_REPEAT;
			break;
		case GL_MIRRORED_REPEAT:
			return GL_CLAMP_TO_EDGE;
			break;
		case GL_CLAMP_TO_EDGE:
			return GL_CLAMP_TO_BORDER;
			break;
		case GL_CLAMP_TO_BORDER:
			return GL_REPEAT;
			break;
		default:
			return GL_REPEAT;
			break;
		}
	}

	void SwapTextureMagnificationFilteringMode(GLuint texture)
	{
		GLenum nextMagnificationFilteringMode = GetNextMagnificationFilteringMode(texture);
		SetupTexture(texture, GL_ZERO, nextMagnificationFilteringMode);
	}

	GLint GetNextMagnificationFilteringMode(GLuint texture)
	{
		GLint currentMagnificationFilteringMode;
		glGetTextureParameteriv(texture, GL_TEXTURE_MAG_FILTER, &currentMagnificationFilteringMode);

		switch (currentMagnificationFilteringMode)
		{
		case GL_NEAREST:
			return GL_LINEAR;
			break;
		case GL_LINEAR:
			return GL_NEAREST;
			break;
		default:
			return GL_NEAREST;
			break;
		}
	}

	void SwapTextureMinificationFilteringMode(GLuint texture)
	{
		GLenum nextMinificationFilteringMode = GetNextMinificationFilteringMode(texture);
		SetupTexture(texture, GL_ZERO, GL_ZERO, nextMinificationFilteringMode);
	}

	GLint GetNextMinificationFilteringMode(GLuint texture)
	{
		GLint currentMinificationFilteringMode;
		glGetTextureParameteriv(texture, GL_TEXTURE_MIN_FILTER, &currentMinificationFilteringMode);

		switch (currentMinificationFilteringMode)
		{
		case GL_NEAREST:
			return GL_LINEAR;
			break;
		case GL_LINEAR:
			return GL_NEAREST_MIPMAP_NEAREST;
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
			return GL_NEAREST_MIPMAP_LINEAR;
			break;
		case GL_NEAREST_MIPMAP_LINEAR:
			return GL_LINEAR_MIPMAP_NEAREST;
			break;
		case GL_LINEAR_MIPMAP_NEAREST:
			return GL_LINEAR_MIPMAP_LINEAR;
			break;
		case GL_LINEAR_MIPMAP_LINEAR:
			return GL_NEAREST;
			break;
		default:
			return GL_NEAREST;
			break;
		}
	}

private:
	GLuint program;

	GLuint vao;
	GLuint vbo;
	const float b = 150.0f;
	const float h = 5.0f;

	GLuint textureBottom;
	GLuint textureTop;
	GLuint textureLeft;
	GLuint textureRight;

	vmath::mat4 worldMatrixBottom;
	vmath::mat4 worldMatrixTop;
	vmath::mat4 worldMatrixLeft;
	vmath::mat4 worldMatrixRight;

	vmath::vec3 cameraPosition;
	vmath::mat4 cameraViewMatrix;
	vmath::mat4 cameraProjectionMatrix;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);