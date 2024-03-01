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
		InitializeProgram();
		InitializeObject();

		glEnable(GL_CULL_FACE);
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		/*
		* The rendering loop is where (not explicitly, but via each material/object entities) the binding of vao, ebo ... is performed and other settings are managed (primitive restart, provoking index ...)
		* In this application there is a single program and a single geometry (multiple) setup, so it could be everything managed during creation
		*/

		glUseProgram(program);

		// Bind the vertex array object with vertex attributes format and buffer binding points
		glBindVertexArray(vao);

		// Bind the buffer object with the indices for indexed drawing
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		// Setup and enable primitive restart for indexed rendering (for triangle stripe primitive)
		glPrimitiveRestartIndex(255);
		glEnable(GL_PRIMITIVE_RESTART);

		// Bind the buffer object with the parameters of the multiple drawing commands
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, dibo);

		// Update uniforms
		glUniformMatrix4fv(0, 1, GL_FALSE, cameraProjectionMatrix * cameraViewMatrix);
		glUniform1f(1, (float)currentTime);

		glMultiDrawElementsIndirect(GL_TRIANGLE_STRIP, GL_UNSIGNED_BYTE, 0, 2, sizeof(DrawElementsIndirectCommand));  // pyramid and cube
		//glMultiDrawElementsIndirect(GL_TRIANGLE_STRIP, GL_UNSIGNED_BYTE, 0, 1, sizeof(DrawElementsIndirectCommand));  // only pyramid
		//glMultiDrawElementsIndirect(GL_TRIANGLE_STRIP, GL_UNSIGNED_BYTE, (void*)sizeof(DrawElementsIndirectCommand), 1, sizeof(DrawElementsIndirectCommand));  // only cube
	}

	void shutdown()
	{
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &vbo2);
		glDeleteBuffers(1, &ebo);
		glDeleteBuffers(1, &dibo);
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
		UpdateCameraViewMatrix(vmath::vec3(0.0f, 2.0f, 5.0f));
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		const vmath::vec3 kTarget = vmath::vec3(0.0f, 0.0f, 0.0f);
		const vmath::vec3 kUp = vmath::vec3(0.0f, 1.0f, 0.0f);
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

#pragma region Material

	void InitializeProgram()
	{
		// Vertex shader
		const char* vertexShaderSource[] =
		{
			"#version 450 core																\n"
			"																				\n"
			"layout (location = 0) uniform mat4 vp_matrix;									\n"
			"layout (location = 1) uniform float time;										\n"
			"																				\n"
			"layout (location = 0) in vec4 position;										\n"
			"layout (location = 10) in uint draw_id;										\n"
			"																				\n"
			"out vec4 vs_color;																\n"
			"																				\n"
			"mat4 GetRotationY(float deg)													\n"
			"{																				\n"
			"	mat4 rot_y;																	\n"
			"	rot_y[0] = vec4(cos(deg), 0.0, sin(deg), 0.0);								\n"
			"	rot_y[1] = vec4(0.0, 1.0, 0.0, 0.0);										\n"
			"	rot_y[2] = vec4(-sin(deg), 0.0, cos(deg), 0.0);								\n"
			"	rot_y[3] = vec4(0.0, 0.0, 0.0, 1.0);										\n"
			"																				\n"
			"	return rot_y;																\n"
			"}																				\n"
			"																				\n"
			"void main(void)																\n"
			"{																				\n"
			"	gl_Position = vp_matrix * GetRotationY(time) * position;					\n"
			"																				\n"
			"	if (draw_id % 2 == 0)														\n"
			"	{																			\n"
			"		vs_color = vec4(0.5, 0.5, 0.5, 1.0);									\n"
			"	}																			\n"
			"	else																		\n"
			"	{																			\n"
			"		vs_color = vec4(1.0, 1.0, 0.0, 1.0);									\n"
			"	}																			\n"
			"}																				\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader
		const char* fragmentShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"in vec4 vs_color;													\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vs_color;												\n"
			"}																	\n"
		};

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		// Program
		program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		// Free resources
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

#pragma endregion

#pragma region Objects - Asterois (pyramid and cube)

	void InitializeObject()
	{
		// * VAO
		glCreateVertexArrays(1, &vao);
		//glBindVertexArray(vao);

		// * VBO

		// (centered equilateral triangular-based) pyramid
		const GLfloat kPyramidRadius = 1.0f;
		GLfloat pyramidHeight = kPyramidRadius;
		GLfloat pyramidHalfHeight = pyramidHeight / 2.0f;
		const GLfloat pyramidPositions[] = {
			0.0f, pyramidHalfHeight, 0.0f, 1.0f,
			kPyramidRadius, -pyramidHalfHeight, 0.0f, 1.0f,
			kPyramidRadius * cos((2.0f / 3.0f) * M_PI), -pyramidHalfHeight, kPyramidRadius * sin((2.0f / 3.0f) * M_PI), 1.0f,
			kPyramidRadius * cos((- 2.0f / 3.0f) * M_PI), -pyramidHalfHeight, kPyramidRadius* sin((- 2.0f / 3.0f)* M_PI), 1.0f
		};

		// (centered) cube
		const GLfloat kCubeSide = 1.0f;
		GLfloat cubeHalfSide = kCubeSide / 2.0f;
		const GLfloat cubePositions[] = {
			-cubeHalfSide, -cubeHalfSide, cubeHalfSide, 1.0f,
			cubeHalfSide, -cubeHalfSide, cubeHalfSide, 1.0f,
			cubeHalfSide, cubeHalfSide, cubeHalfSide, 1.0f,
			-cubeHalfSide, cubeHalfSide, cubeHalfSide, 1.0f,

			-cubeHalfSide, -cubeHalfSide, -cubeHalfSide, 1.0f,
			cubeHalfSide, -cubeHalfSide, -cubeHalfSide, 1.0f,
			cubeHalfSide, cubeHalfSide, -cubeHalfSide, 1.0f,
			-cubeHalfSide, cubeHalfSide, -cubeHalfSide, 1.0f,
		};

		// Draw command indices
		const GLuint drawCommandIndices[] = {
			0, 255, 255, 255, 255, 1, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 255, 255 // etc... Because of base vertex it is not possible to define just a single value for each drawing command index
		};

		// Create and setup verter buffer object and vertex attributes (+include draw_id attribute)
		glCreateBuffers(1, &vbo);
		glNamedBufferStorage(vbo, sizeof(pyramidPositions) + sizeof(cubePositions), pyramidPositions, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferSubData(vbo, sizeof(pyramidPositions), sizeof(cubePositions), cubePositions);

		glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(GLfloat) * 4);
		glEnableVertexArrayAttrib(vao, 0);

		glCreateBuffers(1, &vbo2);
		glNamedBufferStorage(vbo2, sizeof(drawCommandIndices), drawCommandIndices, NULL);

		glVertexArrayAttribIFormat(vao, 10, 1, GL_UNSIGNED_INT, 0);
		glVertexArrayAttribBinding(vao, 10, 10);
		glVertexArrayVertexBuffer(vao, 10, vbo2, 0, sizeof(GLuint));
		glVertexArrayBindingDivisor(vao, 10, 1);  // The value of divisor can be 1 because drawing single instances; otherwise, it should be a value greater than the highest number of instances value among all commands (because the instanced vertex attribute is reset per drawing command)
		glEnableVertexArrayAttrib(vao, 10);

		// * EBO (by triangle strip primitive)

		// pyramid
		const GLubyte pyramidIndices[] = {
			0, 1, 2, 3, 0, 1
		};

		// cube (primitive restart index = 255)
		const GLubyte cubeIndices[] = {
			0, 1, 3, 2, 7, 6, 4, 5, 255, 6, 2, 5, 1, 4, 0, 7, 3
		};

		// Create and setup index buffer object
		glCreateBuffers(1, &ebo);
		glNamedBufferStorage(ebo, sizeof(pyramidIndices) + sizeof(cubeIndices), pyramidIndices, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferSubData(ebo, sizeof(pyramidIndices), sizeof(cubeIndices), cubeIndices);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		//glPrimitiveRestartIndex(255);
		//glEnable(GL_PRIMITIVE_RESTART);

		// * DIBO

		// commands
		/*
		* Warning!
		* The approach presented in the book to have unique per-command attributes (e.g drawing command index) of works in a very specific use case: non-indexed non-instanced multi-drawing
		* - In this scenario it is possible to store in a buffer object per-command attribute values
		* Otherwise, it is not possible.
		* - On indexed drawing, it is required to specify corresponding base vertex offset value for each drawing command, so it would be required to offset per-command attribute data with corresponding base vertex number of elements (see the example here)
		* - On instanced drawing (where number of specified instances is greater than 1; even different for each drawing command), it is required in a similar fashion to offset (using base instance) per-command attribute data to fit different number of instances
		* - Also, in this last case, using base instace this way would make not possible to assign instance index range (0,x] to other drawing commands rather than the first one.
		*/
		const DrawElementsIndirectCommand cmds[] = {
			{
				6,  // number of indices to process
				1,  // number of instaces to draw
				0,  // position of first index
				0,  // offset applied to index value (not taken into account for primitive restart test)
				0   // offset applied to instance index value (only applies for instanced vertex attribute)
			},
			{
				17,  // including the indices (in this case only one) for primitive restart
				1,
				6,  // in the same EBO there are also indices (6) of previous geometries (pyramid)
				4,  // in the same VBO there are vertex atrtibute values (4) of previous geometries (pyramid)
				1  // use base instance to offset (by the unit) per-command in the array (buffer object) storing the per-command attribute values; warning, in this case base vertex must be also considered => offset = base vertex (4) + 1 = 5
			}
		};

		// Create and setup indirect drawing commands buffer object
		glCreateBuffers(1, &dibo);
		glNamedBufferStorage(dibo, sizeof(cmds), cmds, NULL);
		//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, dibo);
	}

#pragma endregion

private:
	typedef  struct {
		GLuint  count;
		GLuint  instanceCount;
		GLuint  firstIndex;
		GLuint  baseVertex;
		GLuint  baseInstance;
	} DrawElementsIndirectCommand;

private:
	// Camera
	vmath::mat4 cameraViewMatrix;
	vmath::mat4 cameraProjectionMatrix;

	// Material
	GLuint program;

	// Objects - Asteroids (pyramid and cube)
	GLuint vao;
	GLuint vbo;
	GLuint vbo2;  // Drawing command index buffer object
	GLuint ebo;
	GLuint dibo;  // Draw indirect buffer object
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);