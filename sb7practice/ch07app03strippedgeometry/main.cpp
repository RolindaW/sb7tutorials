// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializeProgram();
		InitializeCamera();
		InitializeObject();

		glEnable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(0xFF);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(program);

		// Update the uniforms of the Model-View and Projection matrices
		// Note: It is possible using MVP matrix as there are not required addtional world - or view - space calculations
		glUniformMatrix4fv(0, 1, GL_FALSE, cameraProjectionMatrix * cameraViewMatrix * modelWorldMatrix);

		// Draw indexed cube
		// TODO: Simulate dynamic object drawing ranging number of drawn elements between 0 and total required indices number (135)
		glDrawElements(GL_TRIANGLE_STRIP, 135, GL_UNSIGNED_BYTE, 0);
	}

	void shutdown()
	{
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &vebo);
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

private:

	void InitializeProgram()
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
			"	// Output vertex position in clip space							\n"
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
			"	color = vec4(0.0, 0.8, 1.0, 1.0);								\n"
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

#pragma region Camera

	void InitializeCamera()
	{
		// View matrix
		vmath::vec3 cameraPosition(0.0f, 0.0f, 20.0f);
		UpdateCameraViewMatrix(cameraPosition);

		// Projection matrix
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		// Calculate view matrix as the inverse of camera model-world matrix - assume only translation
		cameraViewMatrix = vmath::translate(-position);
	}

	void UpdateCameraProjectionMatrix(float viewportWidth, float viewportHeight)
	{
		float fov = 45.0f;
		float aspect = viewportWidth / viewportHeight;
		float n = 0.1f, f = 1000.0f;

		cameraProjectionMatrix = vmath::perspective(fov, aspect, n, f);
	}

#pragma endregion

#pragma region Object - 2D (XY) Hiragana "a"

	void InitializeObject()
	{
		// Vertex array object
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// TODO: Use different colors to distinguish between stripes
		// - Deep Sky Blue: (0, 191, 255)
		// - Green: (46, 184, 46)
		// - Purple: (153, 102, 255)
		// - Yellow: (230, 230, 0)
		// Note: Vertices position were obtained from a tool where +Y axis was top-bottom, so geometry is mirrored in X axis.
		// Vertex buffer object
		const unsigned int positions[] = {
			81, 267,  // horizontal (index 0)
			75, 284,
			105, 276,
			99, 314,
			129, 285,
			135, 344,
			164, 287,
			195, 337,
			225, 284,
			283, 328,  // joint horizontal - vertical A
			290, 275,  // B
			340, 319,  // C
			352, 262,  // D
			531, 280,
			478, 231,
			576, 267,
			504, 216,
			593, 247,
			554, 220,

			297, 20,   // vertical (index 19)
			290, 30,
			337, 34,
			317, 66,
			366, 59,
			315, 132,
			384, 93,
			305, 195,
			363, 192,
			// (reuse) joint horizontal - vertical B / D / A / C
			274, 416,
			326, 408,
			270, 478,
			321, 460,
			267, 537,  // joint vertical - circular top E
			320, 513,  // F
			266, 581,  // G
			321, 556,  // H
			270, 666,
			320, 594,
			273, 706,
			322, 659,
			281, 765,  // joint vertical - circular bottom I
			329, 713,  // J
			293, 824,  // K
			344, 774,  // L
			307, 862,
			366, 844,
			320, 884,
			360, 877,
			339, 898,

			508, 398,  // circular (index 49)
			501, 424,
			520, 409,
			495, 468,  // joint circular - circular M
			546, 468,  // N
			474, 510,  // O
			540, 506,  // P
			396, 633,
			430, 667,
			// (reuse) joint vertical - circular bottom J / L / I / K
			210, 809,
			217, 870,
			152, 837,
			166, 896,
			110, 845,
			105, 917,
			75, 844,
			73, 922,
			62, 839,
			56, 920,
			55, 822,
			34, 911,
			55, 808,
			14, 890,
			57, 795,
			4, 834,
			63, 765,
			7, 785,
			76, 736,
			24, 738,
			98, 707,
			52, 697,
			153, 655,
			129, 624,
			211, 613,
			199, 575,
			// (reuse) joint vertical - circular top G / E / H / F
			376, 533,
			374, 496,
			425, 518,
			431, 481,
			// (reuse) joint circular - circular O / M / P / N
			589, 511,
			598, 473,
			628, 522,
			645, 484,
			668, 543,
			699, 506,
			699, 575,
			745, 540,
			718, 618,
			776, 586,
			722, 644,
			791, 634,
			723, 695,
			795, 678,
			722, 722,
			790, 719,
			716, 747,
			779, 760,
			696, 790,
			754, 808,
			671, 822,
			714, 851,
			591, 881,
			650, 896,
			542, 908,
			575, 928,
			480, 932,
			482, 954,
			469, 944
		};

		// Create buffer object to store values of position vertex attribute, allocate required memory and bind to array buffer target
		glCreateBuffers(1, &vbo);
		glNamedBufferStorage(vbo, sizeof(positions), positions, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// Define position vertex attribute data format, index mapping, buffer object binding and enable it
		glVertexArrayAttribFormat(vao, 0, 2, GL_UNSIGNED_INT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(GL_UNSIGNED_INT) * 2);
		glEnableVertexArrayAttrib(vao, 0);

		// Vertex element (indices) buffer object
		const unsigned char indices[] = {
			0, 1, 2, 3, 4, 5, 6, 7, 8,  // horizontal
			9, 10, 11, 12,  // joint horizontal - vertical A / B / C / D
			13, 14, 15, 16, 17, 18,

			0xFF,  // primitive restart index value

			19, 20, 21, 22, 23, 24, 25, 26, 27,  // vertical
			10, 12, 9, 11,  // (reuse) joint horizontal - vertical B / D / A / C
			28, 29, 30, 31,
			32, 33, 34, 35,  // joint vertical - circular top E / F / G / H
			36, 37, 38, 39,
			40, 41, 42, 43,  // joint vertical - circular bottom I / J / K / L
			44, 45, 46, 47, 48,

			0xFF,  // primitive restart index value

			49, 50, 51,  // circular
			52, 53, 54, 55,  // joint circular - circular M / N / O / P
			56, 57,
			41, 43, 40, 42,  // (reuse) joint vertical - circular bottom J / L / I / K
			58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83,
			34, 32, 35, 33,  // (reuse) joint vertical - circular top G / E / H / F
			84, 85, 86, 87,
			54, 52, 55, 53,  // (reuse) joint circular - circular O / M / P / N
			88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116
		};

		unsigned int numberOfIndices = sizeof(indices) / sizeof(unsigned char);

		// Create buffer object to store values of indices, allocate required memory and bind to array element buffer target
		glCreateBuffers(1, &vebo);
		glNamedBufferStorage(vebo, sizeof(indices), indices, NULL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vebo);

		// Adjuste Model-World matrix
		// - Scale 1:100 - Geometry ranges between (0,0) and (800,950) aprox., so we want to make it look much smaller e.g. few units, range between (0,0) and (8,9.5)
		// - Mirror (Flip) +Y axis - Geometry was obtained from a tool where +Y axis was top-bottom, so we need opposite
		// - Traslate - Center resulting object into the viewport (offset according to X and Y sizes)
		float scaleValue = 0.01f;
		modelWorldMatrix = vmath::translate(-4.0f, 5.0f, 0.0f) * vmath::scale(scaleValue, -scaleValue, scaleValue);  // Scale model - very large design
	}

#pragma endregion

private:
	GLuint program;

	GLuint vao;  // Store vertex fetch stage settings
	GLuint vbo;  // Store per-vertex data
	GLuint vebo;  // Store vertex indices
	vmath::mat4 modelWorldMatrix;

	vmath::mat4 cameraViewMatrix;
	vmath::mat4 cameraProjectionMatrix;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);