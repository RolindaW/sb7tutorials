// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{	
		InitializeCamera();
		InitializeGroundProgram();
		InitializeGround();
		InitializeGrassProgram();
		InitializeGrass();
	}

	void render(double currentTime)
	{
		// Clar color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		if (kSimulateCameraMotion)
		{
			SimulateCameraMotion(currentTime);
		}

		if (kDrawGround)
		{
			// Draw - Ground
			glUseProgram(groundProgram);
			glBindVertexArray(groundVao);
			glUniformMatrix4fv(0, 1, GL_FALSE, cameraProjectionMatrix * cameraViewMatrix * groundModelWorldMatrix);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		// Draw - Grass
		glUseProgram(grassProgram);
		glBindVertexArray(grassVao);
		glUniformMatrix4fv(0, 1, GL_FALSE, cameraProjectionMatrix * cameraViewMatrix);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, pow(2, 20));  // 1024 x 1024 grassland; it would be usefull to send this value into the vertex shader stage for use in randomization functions
	}

	void shutdown()
	{
		RemoveGround();
		RemoveGrass();
	}

public:
	void onResize(int w, int h)
	{
		sb7::application::onResize(w, h);

		// Update viewport
		glViewport(0, 0, info.windowWidth, info.windowHeight);

		// Update projection matrix: it is required viewport and projection to be consistent
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

private:

#pragma region Camera

	void InitializeCamera()
	{
		SimulateCameraMotion(0.0);
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void SimulateCameraMotion(double currentTime)
	{
		float scaledElapsedTime = (float)currentTime * 0.02f;
		float radius = kGroundSide;
		float height = 25.0f;
		vmath::vec3 newPosition(vmath::vec3(sinf(scaledElapsedTime) * radius, height, cosf(scaledElapsedTime) * radius));  // Circular motion
		UpdateCameraViewMatrix(newPosition);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		const vmath::vec3 kTarget = vmath::vec3(0.0f, -50.0f, 0.0f);
		const vmath::vec3 kUp = vmath::vec3(0.0f, 1.0f, 0.0f);  // Note: Can be constante value (+Y) as we are not adding transformations, but calculating always new one based on a dynamic seed (elapsed time)
		cameraViewMatrix = vmath::lookat(position, kTarget, kUp);
	}

	void UpdateCameraProjectionMatrix(float width, float height)
	{
		float fov = 45.0f;
		float aspect = width / height;
		float n = 0.1f, f = 1000.0f;

		cameraProjectionMatrix = vmath::perspective(fov, aspect, n, f);
	}

#pragma endregion

#pragma region Object - Ground (earth)

	void InitializeGroundProgram()
	{
		// Vertex shader
		const char* vertexShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 mvp_matrix;						\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	gl_Position = mvp_matrix * vec4(position, 1.0);					\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const char* fragmentShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(0.31, 0.22, 0.15, 1.0);							\n"
			"}																	\n"
		};

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		// Program
		groundProgram = glCreateProgram();
		glAttachShader(groundProgram, vertexShader);
		glAttachShader(groundProgram, fragmentShader);
		glLinkProgram(groundProgram);

		// Free resources
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void InitializeGround()
	{
		// Vertex array object - VAO
		glCreateVertexArrays(1, &groundVao);

		// Vertex buffer object - VBO

		// Vertex attribute values - Position (triangle stripe)
		const GLfloat positions[] = {
			-kGroundSide, 0.0f, kGroundSide,
			kGroundSide, 0.0f, kGroundSide,
			-kGroundSide, 0.0f, -kGroundSide,
			kGroundSide, 0.0f, -kGroundSide
		};

		// Create buffer, allocate memory and store data
		glCreateBuffers(1, &groundVbo);
		glNamedBufferStorage(groundVbo, sizeof(positions), positions, NULL);

		// Setup vertex attribute - Position
		glVertexArrayAttribFormat(groundVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(groundVao, 0, 0);
		glVertexArrayVertexBuffer(groundVao, 0, groundVbo, 0, sizeof(GLfloat) * 3);
		glEnableVertexArrayAttrib(groundVao, 0);

		groundModelWorldMatrix = vmath::mat4::identity();
	}

	void RemoveGround()
	{
		glDeleteProgram(groundProgram);
		glDeleteVertexArrays(1, &groundVao);
		glDeleteBuffers(1, &groundVbo);
	}

#pragma endregion

#pragma region Object - Grass (blade)

	void InitializeGrassProgram()
	{
		// Vertex shader

		// Uniform grassland - integer seed output range (-512, 511)
		const char* vertexShaderSource_01UniformGrassland[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 vp_matrix;						\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"																	\n"
			"out vec4 fs_color;													\n"
			"																	\n"
			"vec3 calculatePositionOffset(int seed)								\n"
			"{																	\n"
			"	int seed_lsb = bitfieldExtract(seed, 0, 10);					\n"
			"	int seed_msb = bitfieldExtract(seed, 10, 10);					\n"
			"	float x_offset = float(seed_lsb);								\n"
			"	float z_offet = float(seed_msb);								\n"
			"	return vec3(x_offset, 0.0, z_offet);							\n"
			"}																	\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	vec3 p_offset = calculatePositionOffset(gl_InstanceID);			\n"
			"	gl_Position = vp_matrix * vec4(position + p_offset, 1.0);		\n"
			"	fs_color = vec4(0.1, 0.5, 0.1, 1.0);							\n"
			"}																	\n"
		};

		// Uniform grassland - unsigned integer seed output range (0, 1023)
		const char* vertexShaderSource_01BUniformGrassland[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 vp_matrix;						\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"																	\n"
			"out vec4 fs_color;													\n"
			"																	\n"
			"vec3 calculatePositionOffset(uint seed)							\n"
			"{																	\n"
			"	uint seed_lsb = bitfieldExtract(seed, 0, 10);					\n"
			"	uint seed_msb = bitfieldExtract(seed, 10, 10);					\n"
			"	float x_offset = float(seed_lsb);								\n"
			"	float z_offet = float(seed_msb);								\n"
			"	return vec3(x_offset, 0.0, z_offet);							\n"
			"}																	\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	vec3 p_offset = calculatePositionOffset(gl_InstanceID);			\n"
			"	gl_Position = vp_matrix * vec4(position + p_offset, 1.0);		\n"
			"	fs_color = vec4(0.1, 0.5, 0.1, 1.0);							\n"
			"}																	\n"
		};

		// Perturbed grassland
		const char* vertexShaderSource_02PerturbedGrassland[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 vp_matrix;						\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"																	\n"
			"out vec4 fs_color;													\n"
			"																	\n"
			"int random(int seed, uint iterations)								\n"
			"{																	\n"
			"	int value = seed;												\n"
			"	int i;															\n"
			"																	\n"
			"	for (i = 0; i < iterations; i++)								\n"
			"	{																\n"
			"		//value = bitfieldExtract(value * 1048576, 0, 10);			\n"
			"		value = ((value >> 7) ^ (value << 9)) * 15485863;			\n"
			"	}																\n"
			"																	\n"
			"	return value;													\n"
			"}																	\n"
			"																	\n"
			"vec3 calculatePositionOffset(int seed)								\n"
			"{																	\n"
			"	int number1 = random(seed, 3);									\n"
			"	int number2 = random(number1, 2);								\n"
			"																	\n"
			"	float x_offset = float(seed >> 10) - 512.0;						\n"
			"	x_offset += float(number1 & 0xFF) / 256.0;						\n"
			"																	\n"
			"	float z_offset = float(seed & 0x3FF) - 512.0;					\n"
			"	z_offset += float(number2 & 0xFF) / 256.0;						\n"
			"																	\n"
			"	return vec3(x_offset, 0.0, z_offset);							\n"
			"}																	\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	vec3 p_offset = calculatePositionOffset(gl_InstanceID);			\n"
			"	gl_Position = vp_matrix * vec4(position + p_offset, 1.0);		\n"
			"	fs_color = vec4(0.1, 0.5, 0.1, 1.0);							\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource_02PerturbedGrassland, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const char* fragmentShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"in vec4 fs_color;													\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = fs_color;												\n"
			"}																	\n"
		};

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		// Program
		grassProgram = glCreateProgram();
		glAttachShader(grassProgram, vertexShader);
		glAttachShader(grassProgram, fragmentShader);
		glLinkProgram(grassProgram);

		// Free resources
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void InitializeGrass()
	{
		// Vertex array object - VAO
		glCreateVertexArrays(1, &grassVao);

		// Vertex buffer object - VBO

		// Vertex attribute values - Position (triangle stripe)
		const GLfloat positions[] =
		{
			-0.3f, 0.0f,
			 0.3f, 0.0f,
			-0.20f, 1.0f,
			 0.1f, 1.3f,
			-0.05f, 2.3f,
			 0.0f, 3.3f
		};

		// Create buffer, allocate memory and store data
		glCreateBuffers(1, &grassVbo);
		glNamedBufferStorage(grassVbo, sizeof(positions), positions, NULL);

		// Setup vertex attribute - Position
		glVertexArrayAttribFormat(grassVao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(grassVao, 0, 0);
		glVertexArrayVertexBuffer(grassVao, 0, grassVbo, 0, sizeof(GLfloat) * 2);
		glEnableVertexArrayAttrib(grassVao, 0);
	}

	void RemoveGrass()
	{
		glDeleteProgram(grassProgram);
		glDeleteVertexArrays(1, &grassVao);
		glDeleteBuffers(1, &grassVbo);
	}

#pragma endregion	

private:
	// Ground
	GLuint groundProgram;
	GLuint groundVao;
	GLuint groundVbo;
	vmath::mat4 groundModelWorldMatrix;
	const float kGroundSide = 512.0f;
	const bool kDrawGround = false;

	// Grass
	GLuint grassProgram;
	GLuint grassVao;
	GLuint grassVbo;

	// Camera
	vmath::mat4 cameraViewMatrix;
	vmath::mat4 cameraProjectionMatrix;
	const bool kSimulateCameraMotion = false;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);