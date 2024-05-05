// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"

enum CuadraticBezierPatch
{
	kPatchSize = 16  // Quadratic bezier patch
};

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:

	my_application() : tess_level_(16.0f), wireframe_mode_(true), show_cage_(true)
	{
	}

public:

	void startup()
	{
		InitializeCamera();
		InitializeObject();
		InitializeTessellationProgram();
		InitializeControlCageProgram();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// Program - Tessellation

		glUseProgram(tess_program_);

		glBindVertexArray(vao_);

		glUniformMatrix4fv(0, 1, GL_FALSE, camera_view_matrix_ * object_model_matrix_);
		glUniformMatrix4fv(1, 1, GL_FALSE, camera_projection_matrix_);

		// Set patch size and default tessellation levels (TCS not used)
		glPatchParameteri(GL_PATCH_VERTICES, kPatchSize);
		glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, GetTessLevelOuter());
		glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, GetTessLevelInner());

		if (wireframe_mode_)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		glDrawArrays(GL_PATCHES, 0, kPatchSize);

		// Program - Control cage

		if (show_cage_)
		{
			glUseProgram(cage_program_);

			glBindVertexArray(vao_);

			glUniformMatrix4fv(0, 1, GL_FALSE, camera_projection_matrix_ * camera_view_matrix_ * object_model_matrix_);

			glPointSize(5.0f);
			glDrawArrays(GL_POINTS, 0, kPatchSize);
		}
	}

	void shutdown()
	{
		glDeleteVertexArrays(1, &vao_);
		glDeleteBuffers(1, &vbo_);
		glDeleteProgram(tess_program_);
		glDeleteProgram(cage_program_);
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

	void onKey(int key, int action)
	{
		sb7::application::onKey(key, action);

		switch (key)
		{
		case GLFW_KEY_UP:
			if (action)
			{
				tess_level_ = tess_level_ + 1 > kMaxTessLevel ? kMaxTessLevel : tess_level_ + 1;
			}
			break;
		case GLFW_KEY_DOWN:
			if (action)
			{
				tess_level_ = tess_level_ - 1 < kMinTessLevel ? kMinTessLevel : tess_level_ - 1;
			}
			break;
		case GLFW_KEY_LEFT:
			if (action)
			{
				RotateYObject(-1);
			}
			break;
		case GLFW_KEY_RIGHT:
			if (action)
			{
				RotateYObject(1);
			}
			break;
		case GLFW_KEY_W:
			if (action)
			{
				wireframe_mode_ = !wireframe_mode_;
			}
			break;
		case GLFW_KEY_C:
			if (action)
			{
				show_cage_ = !show_cage_;
			}
			break;
		default:
			break;
		}
	}

private:

#pragma region Camera

	void InitializeCamera()
	{
		vmath::vec3 camera_position(0.0f, 0.0f, 20.0f);
		UpdateCameraViewMatrix(camera_position);
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		// Calculate the view matrix as the inverse of the model-world matrix of the camera (only translated in this case)
		//camera_view_matrix_ = vmath::translate(-position);

		vmath::vec3 target(0.0f, 0.0f, 0.0f);
		vmath::vec3 up(0.0f, 1.0f, 0.0f);
		camera_view_matrix_ = vmath::lookat(position, target, up);
	}

	void UpdateCameraProjectionMatrix(float width, float height)
	{
		float fov = 45.0f;
		float aspect = width / height;
		float n = 0.1f, f = 1000.0f;

		camera_projection_matrix_ = vmath::perspective(fov, aspect, n, f);
	}
#pragma endregion

#pragma region Object - Cubic bezier patch (or surface)

	void InitializeObject()
	{
		glCreateVertexArrays(1, &vao_);

		// Generate patch - x4 cuadratic bezier curve (x4 control points) == x16 control points
		// XZ plane
		const float kCellSize = 3.0f;
		vmath::vec4* positions = new vmath::vec4[kPatchSize];
		for (int i = 0; i < kPatchSize; i++)
		{
			//float x_coord = (float(i & 0x3) - 2) * kCellSize;  // Centered and scaled (by grid size) - Simpler, but not exactly centered in origin
			//float z_coord = (float(i >> 2) - 2) * kCellSize;

			float x_coord = (float(i & 0x3) * kCellSize) - (kCellSize * 0x3 / 2.0f);  // Scaled and centered (by patch size)
			float z_coord = (float(i >> 2) * kCellSize) - (kCellSize * 0x3 / 2.0f);
			float y_coord = (2.0f * kCellSize) * sinf(kCellSize * x_coord + z_coord);  // (2.0f * kCellSize) is the amplitude; kCellSize * x_coord is the freq; z_coord is the lag

			positions[i] = vmath::vec4(x_coord, y_coord, z_coord, 1.0f);
		}

		glCreateBuffers(1, &vbo_);
		glNamedBufferStorage(vbo_, kPatchSize * sizeof(vmath::vec4), positions, NULL);

		delete[] positions;

		// Vertex attribute: position
		glVertexArrayAttribFormat(vao_, 0, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao_, 0, 0);
		glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(vmath::vec4));
		glEnableVertexArrayAttrib(vao_, 0);

		object_model_matrix_ = vmath::rotate(0.0f, 60.0f, 0.0f);
	}

	void RotateYObject(int ccw)
	{
		const float kObjectRotationYStep = 1.0f;
		object_model_matrix_ = vmath::rotate(0.0f, ccw * kObjectRotationYStep, 0.0f) * object_model_matrix_;
	}

#pragma endregion

#pragma region Program - Tessellation

	void InitializeTessellationProgram()
	{
		// Vertex shader
		const char* vertex_shader_source[] =
		{
			"#version 450 core																			\n"
			"																							\n"
			"layout (location = 0) uniform mat4 mv_matrix;												\n"
			"																							\n"
			"layout (location = 0) in vec4 position;													\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	gl_Position = mv_matrix * position;														\n"
			"}																							\n"
		};

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// Tess evaluation shader
		const char* tess_evaluation_shader_source[] =
		{
			"#version 450 core																			\n"
			"																							\n"
			"layout (location = 1) uniform mat4 p_matrix;												\n"
			"																							\n"
			"layout (quads, equal_spacing, cw) in;														\n"
			"																							\n"
			"out VERTEX_DATA																			\n"
			"{																							\n"
			"	vec3 n;																					\n"
			"} out_vdata;																				\n"
			"																							\n"
			"vec4 quadratic_bezier(vec4 A, vec4 B, vec4 C, float t)										\n"
			"{																							\n"
			"	vec4 D = mix(A, B, t);																	\n"
			"	vec4 E = mix(B, C, t);																	\n"
			"																							\n"
			"	return mix(D, E, t);																	\n"
			"}																							\n"
			"																							\n"
			"vec4 cubic_bezier(vec4 A, vec4 B, vec4 C, vec4 D, float t)									\n"
			"{																							\n"
			"	vec4 E = mix(A, B, t);																	\n"
			"	vec4 F = mix(B, C, t);																	\n"
			"	vec4 G = mix(C, D, t);																	\n"
			"																							\n"
			"	return quadratic_bezier(E, F, G, t);													\n"
			"}																							\n"
			"																							\n"
			"vec4 evaluate_patch(vec2 at)																\n"
			"{																							\n"
			"	vec4 p_list[4];																			\n"
			"	int i;																					\n"
			"																							\n"
			"	// Warning! Vertices ordering in memory must match expected order here; otherwise,		\n"
			"	// interpolation will not be correct													\n"
			"																							\n"
			"	// Interpolate u coordinate along each cubic bezier curve								\n"
			"	// Warning! Because bezier curves have been defined in X-axis in memory; so each		\n"
			"	// iteration represent the interpolation along a single cubiz bezier curve				\n"
			"	for (i = 0; i < 4; i++)																	\n"
			"	{																						\n"
			"		p_list[i] = cubic_bezier(gl_in[4 * i + 0].gl_Position,								\n"
			"								 gl_in[4 * i + 1].gl_Position,								\n"
			"								 gl_in[4 * i + 2].gl_Position,								\n"
			"								 gl_in[4 * i + 3].gl_Position,								\n"
			"								 at.x);														\n"
			"	}																						\n"
			"																							\n"
			"	// Interpolate v coordinate along results												\n"
			"	return cubic_bezier(p_list[0], p_list[1], p_list[2], p_list[3], at.y);					\n"
			"}																							\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	// Interpolate tessellated vertex position along the cubic bezier patch					\n"
			"	vec4 p = evaluate_patch(gl_TessCoord.xy);												\n"
			"																							\n"
			"	// Calculate normal using very close position pair										\n"
			"	const float epsilon	= 0.01;																\n"
			"	vec4 p2 = evaluate_patch(gl_TessCoord.xy + vec2(0.0, epsilon));							\n"
			"	vec4 p3 = evaluate_patch(gl_TessCoord.xy + vec2(epsilon, 0.0));							\n"
			"																							\n"
			"	vec3 v1 = normalize(p2.xyz - p.xyz);													\n"
			"	vec3 v2 = normalize(p3.xyz - p.xyz);													\n"
			"																							\n"
			"	gl_Position = p_matrix * p;																\n"
			"	out_vdata.n = cross(v1, v2);															\n"
			"}																							\n"
		};

		GLuint tess_evaluation_shader = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tess_evaluation_shader, 1, tess_evaluation_shader_source, NULL);
		glCompileShader(tess_evaluation_shader);

		// Fragment shader
		const char* fragment_shader_source[] =
		{
			"#version 450 core																			\n"
			"																							\n"
			"in VERTEX_DATA																				\n"
			"{																							\n"
			"	vec3 n;																					\n"
			"} in_vdata;																				\n"
			"																							\n"
			"layout (location = 0) out vec4 color;														\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	// Alternate between red or green color depending on the surface orientation			\n"
			"	vec3 n = normalize(in_vdata.n);															\n"
			"	vec4 c = vec4(1.0, -1.0, 0.0, 0.0) * n.z + vec4(0.0, 0.0, 0.0, 1.0);					\n"
			"	color = clamp(c, vec4(0.0), vec4(1.0));													\n"
			"}																							\n"
		};

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// Program
		tess_program_ = glCreateProgram();
		glAttachShader(tess_program_, vertex_shader);
		glAttachShader(tess_program_, tess_evaluation_shader);
		glAttachShader(tess_program_, fragment_shader);
		glLinkProgram(tess_program_);

		// Free resources
		glDeleteShader(vertex_shader);
		glDeleteShader(tess_evaluation_shader);
		glDeleteShader(fragment_shader);
	}

	vmath::vec4 GetTessLevelOuter()
	{
		return vmath::vec4(tess_level_, tess_level_, tess_level_, tess_level_);
	}

	vmath::vec2 GetTessLevelInner()
	{
		return vmath::vec2(tess_level_, tess_level_);
	}

#pragma endregion

#pragma region Program - Control Cage

	void InitializeControlCageProgram()
	{
		// Vertex shader
		const char* vertex_shader_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 mvp_matrix;						\n"
			"																	\n"
			"layout (location = 0) in vec4 position;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	gl_Position = mvp_matrix * position;							\n"
			"}																	\n"
		};

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// Fragment shader
		const char* fragment_shader_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) out vec4 color;								\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	color = vec4(1.0);												\n"
			"}																	\n"
		};

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// Program
		cage_program_ = glCreateProgram();
		glAttachShader(cage_program_, vertex_shader);
		glAttachShader(cage_program_, fragment_shader);
		glLinkProgram(cage_program_);

		// Free resources
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

#pragma endregion

private:
	vmath::mat4 camera_view_matrix_;
	vmath::mat4 camera_projection_matrix_;

	vmath::mat4 object_model_matrix_;
	GLuint vao_;
	GLuint vbo_;

	GLuint tess_program_;
	float tess_level_;
	const float kMinTessLevel = 1.0f;
	const float kMaxTessLevel = 32.0f;
	bool wireframe_mode_;

	GLuint cage_program_;
	bool show_cage_;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);