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
		//TestXorshiftp();
		//TestPairsXorshiftp();
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

		// Uniform grassland: integer seed; output range (-512, 511)
		/*
			The algorithm generating 2D coordinates expects input values (i.e. seed; gl_InstanceID) to range in [0, 2**20).
			It uses 10 LSBs for the X-coordinate and 10 MSBs for Z-coordinate, which makes resulting distribution to be square; any other bit split (e.g. 8 - 12 or 13 - 7) would make a rectangular distribution.
			As instance index grows up, first 10 LSBs (X-coordinate) will produce 1024 values ranging in [0, 1024); 10 MSBs (Z-coordinate) will not change. So, for each Z-coordinate ("row") value there will exist 1024 X-coordinate ("column") values.
			Every 10 MSBs combination - 1024 values ranging in [0, 1024) - generates a new "row" (Z-coordinate) containing 1024 "columns".
			
			Using different input value range, result in incomplete distribution value set.
			- Under 2**20 instances: incomplete "row" (X-coordinate), and/or incomplete "column" (Z-coordinate) number.
			- Over 2**20 instances: repeated output, as algorithm only process 20 LSBs of input value (i.e. a 21 bit number - or 40 or 56 - will produce same output as the same LSBs 20 bit number).

			Using an algorithm which inputs desired number of rows and columns would make possible to generate any size (different to power of 2) rectangular distribution. Distribution size should fit input values range to generate complete value set.
		*/
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
			"// Generate (int) 2D coordinate based on square grid distribution	\n"
			"vec2 gridCoord(int seed)											\n"
			"{																	\n"
			"	int seed_lsb = bitfieldExtract(seed, 0, 10);					\n"
			"	int seed_msb = bitfieldExtract(seed, 10, 10);					\n"
			"	return vec2(float(seed_lsb), float(seed_msb));					\n"
			"}																	\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Per-instance grid coordinate to offset vertex position		\n"
			"	vec2 p_grid = gridCoord(gl_InstanceID);							\n"
			"	// Offset vertex position along XZ plane						\n"
			"	vec3 p_offset = position + vec3(p_grid.x, 0.0, p_grid.y);		\n"
			"																	\n"
			"	gl_Position = vp_matrix * vec4(p_offset, 1.0);					\n"
			"	fs_color = vec4(0.1, 0.5, 0.1, 1.0);							\n"
			"}																	\n"
		};

		// Uniform grassland B: unsigned integer seed; output range (0, 1023)
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
			"// Generate (int) 2D coordinate based on square grid distribution	\n"
			"vec2 gridCoord(uint seed)											\n"
			"{																	\n"
			"	uint seed_lsb = bitfieldExtract(seed, 0, 10);					\n"
			"	uint seed_msb = bitfieldExtract(seed, 10, 10);					\n"
			"	return vec2(float(seed_lsb), float(seed_msb));					\n"
			"}																	\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Per-instance grid coordinate to offset vertex position		\n"
			"	vec2 p_grid = gridCoord(gl_InstanceID);							\n"
			"	// Offset vertex position along XZ plane						\n"
			"	vec3 p_offset = position + vec3(p_grid.x, 0.0, p_grid.y);		\n"
			"																	\n"
			"	gl_Position = vp_matrix * vec4(p_offset, 1.0);					\n"
			"	fs_color = vec4(0.1, 0.5, 0.1, 1.0);							\n"
			"}																	\n"
		};

		// Uniform grassland C: integer seed; output range (-512, 511); manual bit manipulation and offset
		const char* vertexShaderSource_01CUniformGrassland[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 vp_matrix;						\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"																	\n"
			"out vec4 fs_color;													\n"
			"																	\n"
			"// Generate (int) 2D coordinate based on square grid distribution	\n"
			"vec2 gridCoord(int seed)											\n"
			"{																	\n"
			"	// Select 10 MSBs and offset by max value half					\n"
			"	float x_pos = float((seed >> 10) & 0x3FF) - 512.0;				\n"
			"	// Select 10 LSBs and offset by max value half					\n"
			"	float y_pos = float(seed & 0x3FF) - 512.0;						\n"
			"	return vec2(x_pos, y_pos);										\n"
			"}																	\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Per-instance grid coordinate to offset vertex position		\n"
			"	vec2 p_grid = gridCoord(gl_InstanceID);							\n"
			"	// Offset vertex position along XZ plane						\n"
			"	vec3 p_offset = position + vec3(p_grid.x, 0.0, p_grid.y);		\n"
			"																	\n"
			"	gl_Position = vp_matrix * vec4(p_offset, 1.0);					\n"
			"	fs_color = vec4(0.1, 0.5, 0.1, 1.0);							\n"
			"}																	\n"
		};

		// Perturbed grassland: offset grid position with (normalized) random number; xorshift* RNG
		/*
			This algorithm consist of adding some noise (randomness) to previous squared distribution.
			Noise - for each coordinate - is obtained as a subset (8 bits) of a manually generated 32 bit signed integer random number. Then, the value of the subset is normalized using maximum value of 8 bits (2**8; 256).
			Random number is generated with a "xorshift*" algorithm, which consist of repeteadly (iterations) multiply a subset of a number (seed) by a very large number.
			Resulting number will be larger (e.g. 53 bit) than the maximum capacity of output variable type (32 bit), so it is just taking the 32 LBSs of the output value. Thus, random positive and negative 32 bit numbers seem to be generated.
		*/
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
			"// Generate (int) 2D coordinate based on square grid distribution	\n"
			"vec2 gridCoord(int seed)											\n"
			"{																	\n"
			"	// Select 10 MSBs and offset by max value half					\n"
			"	float x_pos = float((seed >> 10) & 0x3FF) - 512.0;				\n"
			"	// Select 10 LSBs and offset by max value half					\n"
			"	float y_pos = float(seed & 0x3FF) - 512.0;						\n"
			"	return vec2(x_pos, y_pos);										\n"
			"}																	\n"
			"																	\n"
			"// Non-linear xorshift RNG (Random Number Generator): xorshift*	\n"
			"int random(int seed, uint iterations)								\n"
			"{																	\n"
			"	int value = seed;												\n"
			"	int i;															\n"
			"																	\n"
			"	// Iterate over to increase randomness							\n"
			"	for (i = 0; i < iterations; i++)								\n"
			"	{																\n"
			"		// Multiply by a great number to generate a random number	\n"
			"		value = ((value >> 7) ^ (value << 9)) * 15485863;			\n"
			"	}																\n"
			"																	\n"
			"	return value;													\n"
			"}																	\n"
			"																	\n"
			"vec2 randGridCoord(int seed)										\n"
			"{																	\n"
			"	// Grid coordinate												\n"
			"	vec2 p_grid = gridCoord(gl_InstanceID);							\n"
			"																	\n"
			"	// Random number to offset each coordinate						\n"
			"	int number1 = random(seed, 3);									\n"
			"	int number2 = random(number1, 2);								\n"
			"																	\n"
			"	// Select subset (8 LSBs) of random number and normalize		\n"
			"	float x_offset = float(number1 & 0xFF) / 256.0;					\n"
			"	float y_offset = float(number2 & 0xFF) / 256.0;					\n"
			"																	\n"
			"	return p_grid + vec2(x_offset, y_offset);						\n"
			"}																	\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Per-instance rand grid coordinate to offset vertex position	\n"
			"	vec2 p_rgrid = randGridCoord(gl_InstanceID);					\n"
			"	// Offset vertex position along XZ plane						\n"
			"	vec3 p_offset = position + vec3(p_rgrid.x, 0.0, p_rgrid.y);		\n"
			"																	\n"
			"	gl_Position = vp_matrix * vec4(p_offset, 1.0);					\n"
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

	void TestXorshiftp()
	{
		// Declare input and output buffers and allocate required memory
		const unsigned int kInputNumber = 1024*1024;
		int* input = new int[kInputNumber];
		int* output = new int[kInputNumber];

		// Initialize input buffer with some data
		for (unsigned int i = 0; i < kInputNumber; i++)
		{
			input[i] = i;
		}

		// Call RNG xorshiftp algorithm with input buffer content and save return value in output buffer
		const unsigned int kIterations = 1;
		for (unsigned int i = 0; i < kInputNumber; i++)
		{
			output[i] = Xorshiftp(input[i], kIterations);
		}

		// Free input and output buffers memory allocation
		delete[] input;
		delete[] output;
	}

	void TestPairsXorshiftp()
	{
		// Declare 
		int seeds[] = { 0, 58575, 999999, 1048575};
		unsigned int seedNumber = sizeof(seeds) / sizeof(int);

		// Simulate randomess
		for (unsigned int i = 0; i < seedNumber; i++)
		{
			int seed = seeds[i];
			int number1 = Xorshiftp(seed, 3);
			int number2 = Xorshiftp(number1, 2);
			number2 = number2;
		}
	}

	int Xorshiftp(int seed, unsigned int iterations)
	{
		int value = seed;

		for (unsigned int i = 0; i < iterations; i++)
		{
			// Generated number overflows 32 bit (e.g. seed=999999 and iterations=1 generate a 53 bit number)
			// 32 LSBs represent a random number that seem not to follow any sequence between function calls (even when using very close value seeds)
			value = ((value >> 7) ^ (value << 9)) * 15485863;
		}

		return value;
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