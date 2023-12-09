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
		InitializeObject();

		glEnable(GL_CULL_FACE);
	}

	void render(double currentTime)
	{
		// Simply clear the window
		static const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// Calculate object pose
		SimulateObjectPose(currentTime);

		// Update objec model-view and camera projection matrices uniform values
		UpdateUbo();

		// Clear the value of corresponding atomic counter within the buffer
		GLuint* acboData = (GLuint*)glMapNamedBufferRange(acbo,
															0, sizeof(GLuint) * 5,
															GL_MAP_WRITE_BIT);
		acboData[2] = 0;  // Offset qualifier 8 (position for third unsigned int atomic counter) specified in uniform declaration in shader code
		glUnmapNamedBuffer(acbo);

		glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);

		// Use counter program to count used window (viewport) area by the object (number of generated fragments)
		glUseProgram(programCounter);
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);  // Note: If writting to the framebuffer is turned-off, atomic counter does not update the buffer
		glDrawArrays(GL_TRIANGLES, 0, 36);  // Draw 6 faces of 2 triangles of 3 vertices each = 36 vertices

		// Read the value of the counter from the buffer: only for checking purpose.
		/*acboData = (GLuint*)glMapNamedBufferRange(acbo,
													0, sizeof(GLuint) * 5,
													GL_MAP_READ_BIT);
		glUnmapNamedBuffer(acbo);*/

		// Use render program to display the object applying corresponding brightness (based on previously calculated used window area)
		glUseProgram(programRender);
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		// Update window area uniform value
		glUniform1ui(2, (GLuint)info.windowWidth * info.windowHeight);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	void shutdown()
	{
		DeleteObject();
	}

private:
	void InitializeCamera()
	{
		// Pose (position and orientation)
		cameraPosition = vmath::vec3(1.0f, 1.0f, 5.0f);

		// Method 1: Calculate the inverse of the model-world matrix of the camera
		//cameraViewMatrix = vmath::translate(-position);

		// Method 2: Calculate the lookat matrix
		// Note: It is required to update the up vector anytime the matrix is recalculated
		// Note: If camera is only translated in +z (e.g. (0.0, 0.0, 5.0)), resulting matrix should be the equal
		vmath::vec3 target(0.0f, 0.0f, 0.0f);
		vmath::vec3 up(0.0f, 1.0f, 0.0f);
		cameraViewMatrix = vmath::lookat(cameraPosition, target, up);

		// Projection: perspecive
		// Note (NOT IMPLEMENTED): Is is required to update de projection matrix anytime the window (viewpoint) is resized
		float fov = 45.0f;
		float aspect = (float)info.windowWidth / (float)info.windowHeight;
		float n = 0.1f, f = 1000.0f;

		cameraProjectionMatrix = vmath::perspective(fov, aspect, n, f);
	}

	void InitializeObject()
	{
		CreatePrograms();
		CreateVao();
		CreateVbo();
		CreateUbo();
		CreateAcbo();
	}

	void CreatePrograms()
	{
		// Vertex shader
		const GLchar* vertexShaderSource[] = {
			"#version 450 core																				\n"
			"																								\n"
			"layout(std140, binding = 0) uniform TRANSFORMS													\n"
			"{																								\n"
			"	mat4 mv_matrix;																				\n"
			"	mat4 projection_matrix;																		\n"
			"} transforms;																					\n"
			"																								\n"
			"layout(location = 0) in vec3 position;															\n"
			"layout(location = 1) in vec3 color;															\n"
			"																								\n"
			"out VS_OUT																						\n"
			"{																								\n"
			"	vec3 color;																					\n"
			"} vs_out;																						\n"
			"																								\n"
			"void main(void)																				\n"
			"{																								\n"
			"	gl_Position = transforms.projection_matrix * transforms.mv_matrix * vec4(position, 1.0);	\n"
			"	vs_out.color = color;																		\n"
			"}																								\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader: counter
		const GLchar* fragmentShaderCounterSource[] = {
			"#version 450 core																				\n"
			"																								\n"
			"layout(binding = 3, offset = 8) uniform atomic_uint area_counter;								\n"
			"																								\n"
			"void main(void)																				\n"
			"{																								\n"
			"	atomicCounterIncrement(area_counter);														\n"
			"	memoryBarrierAtomicCounter();																\n"
			"}																								\n"
		};

		GLuint fragmentShaderCounter = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShaderCounter, 1, fragmentShaderCounterSource, NULL);
		glCompileShader(fragmentShaderCounter);

		// Fragment shader: render
		const GLchar* fragmentShaderRenderSource[] = {
			"#version 450 core																				\n"
			"																								\n"
			"layout(location = 2) uniform uint window_area;													\n"
			"																								\n"
			"layout(std140, binding = 1) uniform COUNTERS													\n"
			"{																								\n"
			"	layout (offset = 8) uint area_counter;														\n"
			"} counters;																					\n"
			"																								\n"
			"in VS_OUT																						\n"
			"{																								\n"
			"	vec3 color;																					\n"
			"} fs_in;																						\n"
			"																								\n"
			"out vec4 color;																				\n"
			"																								\n"
			"void main(void)																				\n"
			"{																								\n"
			"	// Calculate brightness as the ratio between used window area and total window area			\n"
			"	float brightness = clamp(float(counters.area_counter) / float(window_area), 0.0, 1.0);		\n"
			"	// Mix objects color and brightness color													\n"
			"	vec3 mixed_color = mix(fs_in.color, vec3(brightness, brightness, brightness), 0.5);			\n"
			"	// SuperBible example: use only brightness to define fragments color						\n"
			"	//mixed_color = vec3(brightness, brightness, brightness);									\n"
			"	color = vec4(mixed_color, 1.0);																\n"
			"}																								\n"
		};

		GLuint fragmentShaderRender = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShaderRender, 1, fragmentShaderRenderSource, NULL);
		glCompileShader(fragmentShaderRender);

		// Program: counter
		programCounter = glCreateProgram();
		glAttachShader(programCounter, vertexShader);
		glAttachShader(programCounter, fragmentShaderCounter);
		glLinkProgram(programCounter);

		// Program: render
		programRender = glCreateProgram();
		glAttachShader(programRender, vertexShader);
		glAttachShader(programRender, fragmentShaderRender);
		glLinkProgram(programRender);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShaderCounter);
		glDeleteShader(fragmentShaderRender);
	}

	void CreateVao()
	{
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	void CreateVbo()
	{
		const float data[] = {
			// Face 1: near - red
			-0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
			 0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
			-0.5,  0.5,  0.5, 1.0, 0.0, 0.0,

			-0.5,  0.5,  0.5, 1.0, 0.0, 0.0,
			 0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
			 0.5,  0.5,  0.5, 1.0, 0.0, 0.0,

			// Face 2: right - green
			 0.5, -0.5,  0.5, 0.0, 1.0, 0.0,
			 0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
			 0.5,  0.5,  0.5, 0.0, 1.0, 0.0,

			 0.5,  0.5,  0.5, 0.0, 1.0, 0.0,
			 0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
			 0.5,  0.5, -0.5, 0.0, 1.0, 0.0,

			// Face 3: left - blue
			-0.5, -0.5, -0.5, 0.0, 0.0, 1.0,
			-0.5, -0.5,  0.5, 0.0, 0.0, 1.0,
			-0.5,  0.5, -0.5, 0.0, 0.0, 1.0,

			-0.5,  0.5, -0.5, 0.0, 0.0, 1.0,
			-0.5, -0.5,  0.5, 0.0, 0.0, 1.0,
			-0.5,  0.5,  0.5, 0.0, 0.0, 1.0,

			// Face 4: up - yellow
			-0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
			 0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
			-0.5,  0.5, -0.5, 1.0, 1.0, 0.0,

			-0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
			 0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
			 0.5,  0.5, -0.5, 1.0, 1.0, 0.0,

			// Face 5: down - pink
			-0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
			 0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
			-0.5, -0.5,  0.5, 1.0, 0.0, 1.0,

			-0.5, -0.5,  0.5, 1.0, 0.0, 1.0,
			 0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
			 0.5, -0.5,  0.5, 1.0, 0.0, 1.0,

			// Face 6: far - light blue
			 0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
			-0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
			 0.5,  0.5, -0.5, 0.0, 1.0, 1.0,

			 0.5,  0.5, -0.5, 0.0, 1.0, 1.0,
			-0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
			-0.5,  0.5, -0.5, 0.0, 1.0, 1.0
		};

		glCreateBuffers(1, &vbo);
		glNamedBufferStorage(vbo, sizeof(data), data, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		int vertex_size = sizeof(float) * 3 + sizeof(float) * 3;

		// Set up vertex attribute "position" (location = 0) and enable automatic load
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, vertex_size);
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(vao, 0);

		// Set up vertex attribute "color" (location = 1) and enable automatic load
		glVertexArrayAttribBinding(vao, 1, 1);
		glVertexArrayVertexBuffer(vao, 1, vbo, 0, vertex_size);
		glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
		glEnableVertexArrayAttrib(vao, 1);
	}

	void CreateUbo()
	{
		glCreateBuffers(1, &ubo);
		glNamedBufferStorage(ubo, sizeof(float) * 4 * 4 * 2, NULL, GL_DYNAMIC_STORAGE_BIT);  // Size: x2 4x4 float matrix
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);  // Binding qualifier 0 specified in uniform declaration in shader code.
	}

	void UpdateUbo()
	{
		vmath::mat4 modelViewMatrix = cameraViewMatrix * modelWorldMatrix;

		// Using standard layout uniform block
		glNamedBufferSubData(ubo, 0, sizeof(float) * 4 * 4, modelViewMatrix);
		glNamedBufferSubData(ubo, sizeof(float) * 4 * 4, sizeof(float) * 4 * 4, cameraProjectionMatrix);
	}

	void CreateAcbo()
	{
		const GLuint data[] = {
			1, 2, 3, 4, 5
		};

		glCreateBuffers(1, &acbo);
		glNamedBufferStorage(acbo, sizeof(GLuint) * 5, data, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);  // Because this buffer is aimed to store up to 5 unsigned int atomic counter
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 3, acbo);  // Binding qualifier 3 specified in uniform declaration in shader code.

		glBindBufferBase(GL_UNIFORM_BUFFER, 1, acbo);  // Binding qualifier 1 specified in uniform declaration in shader code.
	}

	void DeleteObject()
	{
		DeletePrograms();
		DeleteVao();
		DeleteVbo();
		DeleteUbo();
		DeleteAcbo();
	}

	void DeletePrograms()
	{
		glDeleteProgram(programCounter);
		glDeleteProgram(programRender);
	}

	void DeleteVao()
	{
		glDeleteVertexArrays(1, &vao);
	}

	void DeleteVbo()
	{
		glDeleteBuffers(1, &vbo);
	}

	void DeleteUbo()
	{
		glDeleteBuffers(1, &ubo);
	}

	void DeleteAcbo()
	{
		glDeleteBuffers(1, &acbo);
	}

	void SimulateObjectPose(double currentTime)
	{
		const vmath::vec3 initPosition = cameraPosition;
		vmath::mat4 initPoseMatrix = vmath::translate(initPosition);

		const vmath::vec3 endPosition(0.0f, 0.0f, 0.0f);
		vmath::mat4 endPoseMatrix = vmath::translate(endPosition) * vmath::rotate(0.0f, 50.0f, 0.0f);

		// Simulate motion using linear interpolation based on elapsed time
		const float cycleTime = 3.0f;
		float cycle = currentTime / cycleTime;
		float whole, fractional;
		fractional = modff(cycle, &whole);
		modelWorldMatrix = initPoseMatrix * (1.0f - fractional) + endPoseMatrix * fractional;
	}

private:
	GLuint programCounter;
	GLuint programRender;
	GLuint vao;
	GLuint vbo;
	GLuint ubo;
	GLuint acbo;
	vmath::mat4 modelWorldMatrix;

	vmath::vec3 cameraPosition;
	vmath::mat4 cameraViewMatrix;
	vmath::mat4 cameraProjectionMatrix;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);