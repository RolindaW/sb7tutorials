// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"
#include "sb7ktx.h"
#include "shader.h"

enum Grid
{
	kPatchSize = 4,  // Quad patch
	kGridSide = 64,  // = 2**6
	kPatchTotal = kGridSide * kGridSide  // Square grid 64 * 64 == 2**6 * 2**6 == 2**12 == 4096
};

enum RenderProgram
{
	kBookSample,
	kDistanceToCamera
};

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:

	my_application() : max_height_(6.0f), height_step_(0.15f), wireframe_mode_(true), render_program_index_(1)
	{
	}

public:
	
	void startup()
	{
		InitializeCamera();
		InitializeObject();
		InitializeProgram();
		InitializeProgram2();

		glEnable(GL_CULL_FACE);
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(render_program_[render_program_index_ & 1]);

		glBindVertexArray(vao_);

		glUniformMatrix4fv(0, 1, GL_FALSE, camera_view_matrix_);
		glUniformMatrix4fv(1, 1, GL_FALSE, camera_projection_matrix_);
		glUniform1f(2, max_height_);
		glBindTextureUnit(0, texture_2d_heightmap);
		glBindTextureUnit(1, texture_2d_color);

		if (wireframe_mode_)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		glPatchParameteri(GL_PATCH_VERTICES, kPatchSize);

		glDrawArraysInstanced(GL_PATCHES, 0, kPatchSize, kPatchTotal);
	}

	void shutdown()
	{
		glDeleteVertexArrays(1, &vao_);
		glDeleteProgram(render_program_[kBookSample]);
		glDeleteProgram(render_program_[kDistanceToCamera]);
		glDeleteTextures(1, &texture_2d_heightmap);
		glDeleteTextures(1, &texture_2d_color);
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
				// Forward
				MoveCamera(-0.15f);
			}
			break;
		case GLFW_KEY_DOWN:
			if (action)
			{
				// Backward
				MoveCamera(0.15f);
			}
			break;
		case GLFW_KEY_H:
			if (action)
			{
				// Add height step
				max_height_ += height_step_;
			}
			else
			{
				// Invert height step sense
				height_step_ *= -1.0f;
			}
			break;
		case GLFW_KEY_W:
			if (action)
			{
				wireframe_mode_ = !wireframe_mode_;
			}
			break;
		case GLFW_KEY_R:
			if (action)
			{
				render_program_index_++;
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
		camera_position_ = vmath::vec3(0.0f, 5.0f, 32.0f);
		UpdateCameraViewMatrix(camera_position_);
		UpdateCameraProjectionMatrix((float)info.windowWidth, (float)info.windowHeight);
	}

	void MoveCamera(float z)
	{
		camera_position_ += vmath::vec3(0.0f, 0.0f, z);
		UpdateCameraViewMatrix(camera_position_);
	}

	void UpdateCameraViewMatrix(vmath::vec3 position)
	{
		// Calculate the view matrix as the inverse of the model-world matrix of the camera (only translated in this case)
		camera_view_matrix_ = vmath::translate(-position);
	}

	void UpdateCameraProjectionMatrix(float width, float height)
	{
		float fov = 45.0f;
		float aspect = width / height;
		float n = 0.1f, f = 1000.0f;

		camera_projection_matrix_ = vmath::perspective(fov, aspect, n, f);
	}
#pragma endregion

#pragma region Object - Terrain

	void InitializeObject()
	{
		glCreateVertexArrays(1, &vao_);

		texture_2d_heightmap = sb7::ktx::file::load("C:/workspace/sb7tutorials/resources/media/textures/terragen1.ktx");
		texture_2d_color = sb7::ktx::file::load("C:/workspace/sb7tutorials/resources/media/textures/terragen_color.ktx");
	}

#pragma endregion

#pragma region Program

	void InitializeProgram()
	{
		// Vertex shader
		const char* vertex_shader_source[] =
		{
			"#version 450 core																			\n"
			"																							\n"
			"out VERTEX_DATA																			\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} out_vdata;																				\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	const vec4 vertices[4] = vec4[4](vec4( 0.0, 0.0, 1.0, 1.0),								\n"
			"									 vec4( 1.0, 0.0, 1.0, 1.0),								\n"
			"									 vec4( 0.0, 0.0, 0.0, 1.0),								\n"
			"									 vec4( 1.0, 0.0, 0.0, 1.0));							\n"
			"	const float patch_side = 1.0f;															\n"
			"																							\n"
			"	// Position of the patch within the grid (64 x 64) based on corresponding instance		\n"
			"	// number gl_InstanceID range [0, patch_total) - where patch_total == 64 * 64 == 4096	\n"
			"	// Used mask (0x3F == 2**6 == 64) is the length (i.e. number of patches) of a grid row	\n"
			"	// Using MSB for z-coordinate makes Z-axis to arrange patch arrays along X-axis  		\n"
			"	int x = gl_InstanceID & 0x3F;  // From 6 LSBs (column)									\n"
			"	int z = gl_InstanceID >> 6;  // From 6 MSBs (row) 										\n"
			"	vec2 grid_coord = vec2(x, z);															\n"
			"																							\n"
			"	// Texture coordinate (normalized along 64x64 grid) of the vertex						\n"
			"	out_vdata.tc = (vertices[gl_VertexID].xz + grid_coord) / 64.0;							\n"
			"																							\n"
			"	// Position of the vertex																\n"
			"	// Before applying offset, the grid (and therefore patch position within it) must be	\n"
			"	// centered in the origin (substract half of the grid row length)						\n"
			"	vec2 centered_grid_coord = grid_coord + vec2(-32.0); 									\n"
			"	gl_Position = vertices[gl_VertexID] + vec4(centered_grid_coord.x, 0.0,					\n"
			"											   centered_grid_coord.y, 0.0) * patch_side;	\n"
			"}																							\n"
		};

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// Tess control shader
		const char* tess_control_shader_source[] =
		{
			"#version 450 core																			\n"			
			"																							\n"
			"layout (location = 0) uniform mat4 mv_matrix;												\n"
			"layout (location = 1) uniform mat4 p_matrix;												\n"
			"																							\n"
			"in VERTEX_DATA																				\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} in_vdata[];																				\n"
			"																							\n"
			"out VERTEX_DATA																			\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} out_vdata[];																				\n"
			"																							\n"
			"layout (vertices = 4) out;																	\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	if (gl_InvocationID == 0)																\n"
			"	{																						\n"
			"		// Get vertices NDC coordinates														\n"
			"		vec4 p0 = p_matrix * mv_matrix * gl_in[0].gl_Position;								\n"
			"		vec4 p1 = p_matrix * mv_matrix * gl_in[1].gl_Position;								\n"
			"		vec4 p2 = p_matrix * mv_matrix * gl_in[2].gl_Position;								\n"
			"		vec4 p3 = p_matrix * mv_matrix * gl_in[3].gl_Position;								\n"
			"		p0 /= p0.w;																			\n"
			"		p1 /= p1.w;																			\n"
			"		p2 /= p2.w;																			\n"
			"		p3 /= p3.w;																			\n"
			"																							\n"
			"		// Set tessellation levels as the length of corresponding the edge					\n"
			"		// Warning! Use a bias so very small edges do not result in 0 (patch culling)		\n"
			"		const float max_tess = 16.0;														\n"
			"		const float bias = 1.0;																\n"
			"		float l0 = length(p0.xy - p2.xy) * max_tess + bias;									\n"
			"		float l1 = length(p1.xy - p0.xy) * max_tess + bias;									\n"
			"		float l2 = length(p3.xy - p1.xy) * max_tess + bias;									\n"
			"		float l3 = length(p2.xy - p3.xy) * max_tess + bias;									\n"
			"		gl_TessLevelOuter[0] = l0;															\n"
			"		gl_TessLevelOuter[1] = l1;															\n"
			"		gl_TessLevelOuter[2] = l2;															\n"
			"		gl_TessLevelOuter[3] = l3;															\n"
			"		gl_TessLevelInner[0] = min(l1, l3);													\n"
			"		gl_TessLevelInner[1] = min(l0, l2);													\n"
			"	}																						\n"
			"																							\n"
			"	// Set output patch data																\n"
			"	out_vdata[gl_InvocationID].tc = in_vdata[gl_InvocationID].tc;							\n"
			"	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;				\n"
			"}																							\n"
		};

		GLuint tess_control_shader = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tess_control_shader, 1, tess_control_shader_source, NULL);
		glCompileShader(tess_control_shader);

		// Tess evaluation shader
		const char* tess_evaluation_shader_source[] =
		{
			"#version 450 core																			\n"
			"																							\n"
			"layout (location = 0) uniform mat4 mv_matrix;												\n"
			"layout (location = 1) uniform mat4 p_matrix;												\n"
			"layout (location = 2) uniform float max_height;											\n"
			"																							\n"
			"layout (binding = 0) uniform sampler2D tex_heightmap;										\n"
			"																							\n"
			"layout (quads, fractional_odd_spacing) in;													\n"
			"																							\n"
			"in VERTEX_DATA																				\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} in_vdata[];																				\n"
			"																							\n"
			"out VERTEX_DATA																			\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} out_vdata;																				\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	vec2 tc1 = mix(in_vdata[0].tc, in_vdata[1].tc, gl_TessCoord.x);							\n"
			"	vec2 tc2 = mix(in_vdata[2].tc, in_vdata[3].tc, gl_TessCoord.x);							\n"
			"	vec2 tc = mix(tc1, tc2, gl_TessCoord.y);												\n"
			"																							\n"
			"	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);				\n"
			"	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);				\n"
			"	vec4 p = mix(p1, p2, gl_TessCoord.y);													\n"
			"																							\n"
			"	p.y += texture(tex_heightmap, tc).r * max_height;										\n"
			"																							\n"
			"	gl_Position = p_matrix * mv_matrix * p;													\n"
			"	out_vdata.tc = tc;																		\n"
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
			"layout (binding = 1) uniform sampler2D tex_color;											\n"
			"																							\n"
			"in VERTEX_DATA																				\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} in_vdata;																				\n"
			"																							\n"
			"layout (location = 0) out vec4 color;														\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	color = texture(tex_color, in_vdata.tc);												\n"
			"}																							\n"
		};

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// Program
		render_program_[kBookSample] = glCreateProgram();
		glAttachShader(render_program_[kBookSample], vertex_shader);
		glAttachShader(render_program_[kBookSample], tess_control_shader);
		glAttachShader(render_program_[kBookSample], tess_evaluation_shader);
		glAttachShader(render_program_[kBookSample], fragment_shader);
		glLinkProgram(render_program_[kBookSample]);

		// Free resources
		glDeleteShader(vertex_shader);
		glDeleteShader(tess_control_shader);
		glDeleteShader(tess_evaluation_shader);
		glDeleteShader(fragment_shader);
	}

	void InitializeProgram2()
	{
		// Vertex shader
		const char* vertex_shader_source[] =
		{
			"#version 450 core																			\n"
			"																							\n"
			"out VERTEX_DATA																			\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} out_vdata;																				\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	const vec4 vertices[4] = vec4[4](vec4( 0.0, 0.0, 1.0, 1.0),								\n"
			"									 vec4( 1.0, 0.0, 1.0, 1.0),								\n"
			"									 vec4( 0.0, 0.0, 0.0, 1.0),								\n"
			"									 vec4( 1.0, 0.0, 0.0, 1.0));							\n"
			"	const float patch_side = 1.0f;															\n"
			"																							\n"
			"	// Position of the patch within the grid (64 x 64) based on corresponding instance		\n"
			"	// number gl_InstanceID range [0, patch_total) - where patch_total == 64 * 64 == 4096	\n"
			"	// Used mask (0x3F == 2**6 == 64) is the length (i.e. number of patches) of a grid row	\n"
			"	// Using MSB for z-coordinate makes Z-axis to arrange patch arrays along X-axis  		\n"
			"	int x = gl_InstanceID & 0x3F;  // From 6 LSBs (column)									\n"
			"	int z = gl_InstanceID >> 6;  // From 6 MSBs (row) 										\n"
			"	vec2 grid_coord = vec2(x, z);															\n"
			"																							\n"
			"	// Texture coordinate (normalized along 64x64 grid) of the vertex						\n"
			"	out_vdata.tc = (vertices[gl_VertexID].xz + grid_coord) / 64.0;							\n"
			"																							\n"
			"	// Position of the vertex																\n"
			"	// Before applying offset, the grid (and therefore patch position within it) must be	\n"
			"	// centered in the origin (substract half of the grid row length)						\n"
			"	vec2 centered_grid_coord = grid_coord + vec2(-32.0); 									\n"
			"	gl_Position = vertices[gl_VertexID] + vec4(centered_grid_coord.x, 0.0,					\n"
			"											   centered_grid_coord.y, 0.0) * patch_side;	\n"
			"}																							\n"
		};

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// Tess control shader
		const char* tess_control_shader_source[] =
		{
			"#version 450 core																			\n"
			"																							\n"
			"layout (location = 0) uniform mat4 mv_matrix;												\n"
			"layout (location = 1) uniform mat4 p_matrix;												\n"
			"																							\n"
			"in VERTEX_DATA																				\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} in_vdata[];																				\n"
			"																							\n"
			"out VERTEX_DATA																			\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} out_vdata[];																				\n"
			"																							\n"
			"layout (vertices = 4) out;																	\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	if (gl_InvocationID == 0)																\n"
			"	{																						\n"
			"		// Vertices position in view space													\n"
			"		vec4 p0 = mv_matrix * gl_in[0].gl_Position;											\n"
			"		vec4 p1 = mv_matrix * gl_in[1].gl_Position;											\n"
			"		vec4 p2 = mv_matrix * gl_in[2].gl_Position;											\n"
			"		vec4 p3 = mv_matrix * gl_in[3].gl_Position;											\n"
			"																							\n"
			"		// Middle point of each edge														\n"
			"		vec4 mpe0 = mix(p0, p2, 0.5);														\n"
			"		vec4 mpe1 = mix(p1, p0, 0.5);														\n"
			"		vec4 mpe2 = mix(p3, p1, 0.5);														\n"
			"		vec4 mpe3 = mix(p2, p3, 0.5);														\n"
			"																							\n"
			"		// Point closer than min_dist will get max_tess tessellation level					\n"
			"		// Point farther than max_dist will get min_tess tessellation level					\n"
			"		// Point within (min_dist, max_dist) will get inperpolated tessellation level		\n"
			"		// Tip: greater distance bandwidth and lower tess level bandwidth result in			\n"
			"		// much smoother transitions														\n"
			"		const float min_tess = 1.0;															\n"
			"		const float max_tess = 16.0;//32.0;													\n"
			"		const float min_dist = 0.0;//4.0;													\n"
			"		const float max_dist = 48.0;//20.0;													\n"
			"																							\n"
			"		// Normalized distance between each edge middle point and the camera (origin)		\n"
			"		float nde0 = clamp((length(mpe0) - min_dist) / (max_dist - min_dist), 0.0, 1.0);	\n"
			"		float nde1 = clamp((length(mpe1) - min_dist) / (max_dist - min_dist), 0.0, 1.0);	\n"
			"		float nde2 = clamp((length(mpe2) - min_dist) / (max_dist - min_dist), 0.0, 1.0);	\n"
			"		float nde3 = clamp((length(mpe3) - min_dist) / (max_dist - min_dist), 0.0, 1.0);	\n"
			"																							\n"
			"		// Interpolated tessellation level based on normalized distance						\n"
			"		float tle0 = mix(max_tess, min_tess, nde0);											\n"
			"		float tle1 = mix(max_tess, min_tess, nde1);											\n"
			"		float tle2 = mix(max_tess, min_tess, nde2);											\n"
			"		float tle3 = mix(max_tess, min_tess, nde3);											\n"
			"																							\n"
			"		gl_TessLevelOuter[0] = tle0;														\n"
			"		gl_TessLevelOuter[1] = tle1;														\n"
			"		gl_TessLevelOuter[2] = tle2;														\n"
			"		gl_TessLevelOuter[3] = tle3;														\n"
			"		gl_TessLevelInner[0] = max(tle1, tle3);												\n"
			"		gl_TessLevelInner[1] = max(tle0, tle2);												\n"
			"	}																						\n"
			"																							\n"
			"	// Set output patch data																\n"
			"	out_vdata[gl_InvocationID].tc = in_vdata[gl_InvocationID].tc;							\n"
			"	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;				\n"
			"}																							\n"
		};

		GLuint tess_control_shader = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tess_control_shader, 1, tess_control_shader_source, NULL);
		glCompileShader(tess_control_shader);

		// Tess evaluation shader
		const char* tess_evaluation_shader_source[] =
		{
			"#version 450 core																			\n"
			"																							\n"
			"layout (location = 0) uniform mat4 mv_matrix;												\n"
			"layout (location = 1) uniform mat4 p_matrix;												\n"
			"layout (location = 2) uniform float max_height;											\n"
			"																							\n"
			"layout (binding = 0) uniform sampler2D tex_heightmap;										\n"
			"																							\n"
			"layout (quads, fractional_odd_spacing) in;													\n"
			"																							\n"
			"in VERTEX_DATA																				\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} in_vdata[];																				\n"
			"																							\n"
			"out VERTEX_DATA																			\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} out_vdata;																				\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	vec2 tc1 = mix(in_vdata[0].tc, in_vdata[1].tc, gl_TessCoord.x);							\n"
			"	vec2 tc2 = mix(in_vdata[2].tc, in_vdata[3].tc, gl_TessCoord.x);							\n"
			"	vec2 tc = mix(tc1, tc2, gl_TessCoord.y);												\n"
			"																							\n"
			"	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);				\n"
			"	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);				\n"
			"	vec4 p = mix(p1, p2, gl_TessCoord.y);													\n"
			"																							\n"
			"	p.y += texture(tex_heightmap, tc).r * max_height;										\n"
			"																							\n"
			"	gl_Position = p_matrix * mv_matrix * p;													\n"
			"	out_vdata.tc = tc;																		\n"
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
			"layout (binding = 1) uniform sampler2D tex_color;											\n"
			"																							\n"
			"in VERTEX_DATA																				\n"
			"{																							\n"
			"	vec2 tc;																				\n"
			"} in_vdata;																				\n"
			"																							\n"
			"layout (location = 0) out vec4 color;														\n"
			"																							\n"
			"void main(void)																			\n"
			"{																							\n"
			"	color = texture(tex_color, in_vdata.tc);												\n"
			"}																							\n"
		};

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// Program
		render_program_[kDistanceToCamera] = glCreateProgram();
		glAttachShader(render_program_[kDistanceToCamera], vertex_shader);
		glAttachShader(render_program_[kDistanceToCamera], tess_control_shader);
		glAttachShader(render_program_[kDistanceToCamera], tess_evaluation_shader);
		glAttachShader(render_program_[kDistanceToCamera], fragment_shader);
		glLinkProgram(render_program_[kDistanceToCamera]);

		// Free resources
		glDeleteShader(vertex_shader);
		glDeleteShader(tess_control_shader);
		glDeleteShader(tess_evaluation_shader);
		glDeleteShader(fragment_shader);
	}

#pragma endregion

private:
	vmath::vec3 camera_position_;
	vmath::mat4 camera_view_matrix_;
	vmath::mat4 camera_projection_matrix_;

	GLuint vao_;
	GLuint texture_2d_heightmap;
	GLuint texture_2d_color;

	GLuint render_program_[2];
	unsigned int render_program_index_;
	float max_height_;
	float height_step_;
	bool wireframe_mode_;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);