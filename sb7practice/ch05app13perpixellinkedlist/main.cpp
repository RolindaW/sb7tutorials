// Include the "sb7.h" header file
#include "sb7.h"
#include "vmath.h"
#include "object.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializePrograms();
		InitializeCamera();
		InitializeObject();
		InitializeAtomicCounter();
		InitializeFramebuffer();
		InitializeLinkedList();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		static const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// *** Fill linked list

		glUseProgram(fillingProgram);

		ResetAtomicCounter();
		glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, acbo);

		ResetFramebuffer();
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

		glUniformMatrix4fv(0, 1, GL_FALSE, viewMatrix * modelWorldMatrix);
		glUniformMatrix4fv(1, 1, GL_FALSE, projectionMatrix);

		object.render();

		// Read the value of the atomic counter and save the highest value (just because curious) - in this case it depends on object orientation
		//GLuint atomicCounter = CheckAtomicCounter();
		//atomicCounterMaxValue = atomicCounter > atomicCounterMaxValue ? atomicCounter : atomicCounterMaxValue;

		// Read the value of an item from the linked list (just because curious)
		//CheckLinkedList();

		// Barriers: atomic counter, texture image and shader storage block
		glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// *** Traverse linked list
		glUseProgram(traversingProgram);

		glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

		glUniformMatrix4fv(0, 1, GL_FALSE, viewMatrix * modelWorldMatrix);
		glUniformMatrix4fv(1, 1, GL_FALSE, projectionMatrix);

		object.render();
	}

	void shutdown()
	{
		DestroyPrograms();
		DestroyObject();
		DestroyAtomicCounter();
		DestroyFramebuffer();
		DestroyLinkedList();
	}

public:
	void onKey(int key, int action)
	{
		sb7::application::onKey(key, action);

		// Check keyboard arrows to...
		// - move camera foward/backward
		// - switch (left) texture sampling mode
		switch (key)
		{
		case GLFW_KEY_LEFT:
			if (action)
			{
				RotateObject(-1);
			}
			break;
		case GLFW_KEY_RIGHT:
			if (action)
			{
				RotateObject(1);
			}
			break;
		default:
			break;
		}
	}

private:
	void InitializePrograms()
	{
		// Vertex shader
		const GLchar* vertexShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) uniform mat4 mv;								\n"
			"layout (location = 1) uniform mat4 proj;							\n"
			"																	\n"
			"layout (location = 0) in vec4 position;							\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	gl_Position = proj * mv * position;								\n"
			"}																	\n"
		};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Fragment shader: texturing
		const GLchar* fillingFragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"// Atomic counter for filled size									\n"
			"layout (binding = 0, offset = 0) uniform atomic_uint fill_counter;	\n"
			"																	\n"
			"// 2D image to store head pointers									\n"
			"layout (binding = 0, r32ui) uniform uimage2D head_pointer;			\n"
			"																	\n"
			"struct list_item													\n"
			"{																	\n"
			"	float depth;													\n"
			"	int facing;														\n"
			"	uint prev;														\n"
			"};																	\n"
			"																	\n"
			"// Linked list														\n"
			"layout (binding = 0, std430) buffer list_item_block				\n"
			"{																	\n"
			"	list_item items[];												\n"
			"};																	\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	ivec2 P = ivec2(gl_FragCoord.xy);								\n"
			"																	\n"
			"	uint index = atomicCounterIncrement(fill_counter);				\n"
			"	memoryBarrierAtomicCounter();									\n"
			"																	\n"
			"	uint old_head = imageAtomicExchange(head_pointer, P, index);	\n"
			"	memoryBarrierImage();											\n"
			"																	\n"
			"	items[index].depth = gl_FragCoord.z;							\n"
			"	items[index].facing = gl_FrontFacing ? 1 : 0;					\n"
			"	items[index].prev = old_head;									\n"
			"	memoryBarrierBuffer();											\n"
			"}																	\n"
		};

		GLuint fillingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fillingFragmentShader, 1, fillingFragmentShaderSource, NULL);
		glCompileShader(fillingFragmentShader);

		fillingProgram = glCreateProgram();
		glAttachShader(fillingProgram, vertexShader);
		glAttachShader(fillingProgram, fillingFragmentShader);
		glLinkProgram(fillingProgram);

		// Fragment shader: rendering
		const GLchar* traversingFragmentShaderSource[] = {
			"#version 450 core													\n"
			"																	\n"
			"// 2D image storing head pointers									\n"
			"layout (binding = 0, r32ui) readonly uniform uimage2D head_pointer;\n"
			"																	\n"
			"struct list_item													\n"
			"{																	\n"
			"	float depth;													\n"
			"	int facing;														\n"
			"	uint prev;														\n"
			"};																	\n"
			"																	\n"
			"// Linked list														\n"
			"layout (binding = 0, std430) readonly buffer list_item_block		\n"
			"{																	\n"
			"	list_item items[];												\n"
			"};																	\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"const uint max_fragments = 10;										\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	uint frag_count = 0;											\n"
			"	float depth_accum = 0.0;										\n"
			"	ivec2 P = ivec2(gl_FragCoord.xy);								\n"
			"																	\n"
			"	uint index = imageLoad(head_pointer, P).x;						\n"
			"																	\n"
			"	while (index != 0xFFFFFFFF && frag_count < max_fragments)		\n"
			"	{																\n"
			"		list_item this_item = items[index];							\n"
			"																	\n"
			"		if (this_item.facing == 1)									\n"
			"		{															\n"
			"			depth_accum -= this_item.depth;							\n"
			"		}															\n"
			"		else														\n"
			"		{															\n"
			"			depth_accum += this_item.depth;							\n"
			"		}															\n"
			"																	\n"
			"		index = this_item.prev;										\n"
			"		frag_count++;												\n"
			"	}																\n"
			"																	\n"
			"	depth_accum *= 1000.0;											\n"
			"																	\n"
			"	color = vec4(depth_accum, depth_accum, depth_accum, 1.0);		\n"
			"}																	\n"
		};

		GLuint traversingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(traversingFragmentShader, 1, traversingFragmentShaderSource, NULL);
		glCompileShader(traversingFragmentShader);

		traversingProgram = glCreateProgram();
		glAttachShader(traversingProgram, vertexShader);
		glAttachShader(traversingProgram, traversingFragmentShader);
		glLinkProgram(traversingProgram);

		// Free resources
		glDeleteShader(vertexShader);
		glDeleteShader(fillingFragmentShader);
		glDeleteShader(traversingFragmentShader);
	}

	void DestroyPrograms()
	{
		glDeleteProgram(fillingProgram);
		glDeleteProgram(traversingProgram);
	}

	void InitializeCamera()
	{
		// View
		vmath::vec3 cameraPosition(0.0f, 7.5f, 20.0f);
		vmath::vec3 target(0.0f, 5.0f, 0.0f);
		vmath::vec3 up(0.0f, 1.0f, 0.0f);
		viewMatrix = vmath::lookat(cameraPosition, target, up);

		// Projection
		float fov = 45.0f;
		float aspect = (float)info.windowWidth / (float)info.windowHeight;
		float n = 0.1f, f = 1000.0f;

		projectionMatrix = vmath::perspective(fov, aspect, n, f);
	}

	void InitializeObject()
	{
		const char filename[] = "C:/workspace/sb7tutorials/resources/media/objects/dragon.sbm";
		object.load(filename);

		modelWorldMatrix = vmath::rotate(0.0f, 125.0f, 0.0f);
	}

	void RotateObject(int ccw)
	{
		modelWorldMatrix = vmath::rotate(0.0f, ccw * objectRotationYStep, 0.0f) * modelWorldMatrix;
	}

	void DestroyObject()
	{
		object.free();
	}

	void InitializeAtomicCounter()
	{
		const GLuint data = (GLuint)3;  // Only to verify atomic counter is reset first rendering cycle
		glCreateBuffers(1, &acbo);
		glNamedBufferStorage(acbo, sizeof(GLuint), &data, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
	}

	void ResetAtomicCounter()
	{
		GLuint* acboData = (GLuint*)glMapNamedBufferRange(acbo,
															0, sizeof(GLuint),
															GL_MAP_WRITE_BIT);
		*acboData = (GLuint)0;
		glUnmapNamedBuffer(acbo);
	}

	GLuint CheckAtomicCounter()
	{
		GLuint* acboData = (GLuint*)glMapNamedBufferRange(acbo,
															0, sizeof(GLuint),
															GL_MAP_READ_BIT);

		glUnmapNamedBuffer(acbo);

		return *acboData;
	}

	void DestroyAtomicCounter()
	{
		glDeleteBuffers(1, &acbo);
	}

	void InitializeFramebuffer()
	{
		// Create a new 2D texture object
		glCreateTextures(GL_TEXTURE_2D, 1, &texture);

		// Specify the amount of storage we want to use for the texture
		glTextureStorage2D(texture,
			1,
			GL_R32UI,
			info.windowWidth, info.windowHeight);
	}

	void ResetFramebuffer()
	{
		const GLuint data[] = { 0xFFFFFFFF };  // Security value
		glClearTexImage(texture, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void DestroyFramebuffer()
	{
		glDeleteTextures(1, &texture);
	}

	void InitializeLinkedList()
	{
		// For current application setttings ...
		// - window size
		// - camera pose (position and orientation)
		// - object pose (position, orientation and scale)
		// ... the highest number of generated fragments on a specific object orientation on Y axis is 258894.
		// So, initializing a buffered shader storage block with space enough for 260000 items (linked list) would be enough

		const unsigned int itemCount = 260000;

		linkedList = new LinkedListItem[itemCount];

		glCreateBuffers(1, &ssbo);
		glNamedBufferStorage(ssbo, sizeof(LinkedListItem) * itemCount, &(linkedList[0]), /*GL_MAP_WRITE_BIT |*/ GL_MAP_READ_BIT);  // GL_MAP_WRITE_BIT access flag only if buffer reset required
	}

	/// <summary>
	/// An example of how to map a shader storage block buffer for writing
	/// Corresponding access flag (GL_MAP_WRITE_BIT) is required on buffer memory allocation
	/// </summary>
	void ResetLinkedList()
	{
		LinkedListItem* ssboData = (LinkedListItem*)glMapNamedBufferRange(ssbo,
																			0, sizeof(LinkedListItem) * 2,
																			GL_MAP_WRITE_BIT);

		ssboData[0].prev = 87;
		ssboData[1].prev = 88;

		glUnmapNamedBuffer(ssbo);
	}

	/// <summary>
	/// An example of how to map a shader storage block buffer for reading
	/// Find the first item linking occurrence, i.e. the first time a fragment instance occurs for a position that already happended
	/// Notice this may change between drawing commands as the order of shader instaces execution is unknown (parallel execution)
	/// </summary>
	void CheckLinkedList()
	{
		LinkedListItem* ssboData = (LinkedListItem*)glMapNamedBufferRange(ssbo,
																			0, sizeof(LinkedListItem) * 10000,
																			GL_MAP_READ_BIT);
		int firstItemLinkOccurrence;
		for (int i = 0; i < 10000; i++)
		{
			if (ssboData[i].prev != 0xFFFFFFFF)
			{
				firstItemLinkOccurrence = i;
				break;
			}
		}

		glUnmapNamedBuffer(ssbo);
	}

	void DestroyLinkedList()
	{
		glDeleteBuffers(1, &ssbo);
		delete[] linkedList;
	}

private:
		struct LinkedListItem
		{
			GLfloat depth;
			GLint facing;
			GLuint prev;
		};

private:
	GLuint fillingProgram;
	GLuint traversingProgram;

	sb7::object object;
	vmath::mat4 modelWorldMatrix;
	const float objectRotationYStep = 1.0f;

	vmath::mat4 viewMatrix;
	vmath::mat4 projectionMatrix;

	GLuint acbo;
	GLuint atomicCounterMaxValue = 0;

	GLuint texture;

	GLuint ssbo;
	LinkedListItem* linkedList;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);

/*

Notes

Atomic counter is used to generate per-fragment indices
Texture image is used to store the indices - by position (fragment coordinate) - of lastly processed fragment
Shader storage block is used to store a linked list

Atomic counter and texture image require to be reset before each filling fragment shader execution
Shader storage block is not required to be reset as exactly same all items being processed during traversing fragment shader execution would be updated previously during filling fragment shader execution; there is no risk to have incorrectly linked active items in the list

It is possible to avoid using a texture image (or a shader storage block if preferred) if the fragment coordinate is stored as an item member in the linked list
If so, we should only maintain this list, but performance during traversing fragment shadding execution would be higher

This is by far not the most efficient way to access the corresponding elements (by index) of the texture image variable
Surely (I do not know, but I have seen something on the Internet), it would be possible to force a more specific iteration (for the texture image variable) with a Compute Shader
In this case, we are processing all the fragments, when it would not really be necessary
For the same position, there may be more than one fragment that has an entry in the list, but for this calculation, we are only interested in the highest one
Hence, regardless of the fragment instance that is executed, the one whose index is in the texture image variable is always searched in the list
So, for all fragment instances in the same position, the same index will always be taken (the one in the texture image variable) and the same number of linked elements in the list will always be iterated in the same order
As they will all produce the same output (the alpha is the same, because the purpose of this shader is that OpenGL does not have to do depth test or geometry discarding - culling - in the vertex post-processing stage), it is a loss of efficiency to process the n fragments of the same position; only one would be enough

The initialization value of the framebuffer could be, as in the example, the maximum value of a uint type variable (32 bits for values): 0xFFFFFFFFFFFFFFFF
Here, we would run the risk that if the atomic counter (also uint) reached its maximum value (0xFFFFFFFFFFFF), we would not be able to distinguish between a framebuffer initialization value and the index itself
We would have problems in the processing of the texture image variable because when trying to search in the list from that index read, we would not know if we should discard it or, if it will effectively refer to the index of the last processable fragment
It should be noted that, given the case, it would be more than likely that there would be more fragments than this one to process, but it would not be possible because we could not generate more indexes, so the result of this shaing would be incomplete

Another approach would be to use a framebuffer of type int (1 bit sign; 31 bits for values)
We would use the value -1 to initialize the framebuffer and thus we would not have the problem of the previous case
The drawback with this approach would be that we would lose half of the values available for storing fragment indices
With 32 bits (0xFFFFFFFFFFFF) we have 4,294,967,295 values, while with 31 bits (0x7FFFFFFFFFFF) we only have 2,147,483,647 (half the number of values available for storing fragment indices)
We are talking, by no means, of more than 2 billion values that we no longer have at our disposal because of this security
In my opinion, I think that, in terms of efficiency, it would not be convenient

*/