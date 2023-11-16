// Include the "sb7.h" header file
#include "sb7.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		//review_pointers();  // Review of pointers

		rendering_program = compile_shaders();

		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		int buffer_mode = 1;
		num_buffers = create_buffers(buffer_mode, vao, &buffers);
		if (num_buffers == -1) glfwSetWindowShouldClose(window, 1);
	}

	void render(double currentTime)
	{
		// Simply clear the window with gren color
		static const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		// Use the program object we created earlier for rendering
		glUseProgram(rendering_program);

		// Draw one triangle
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void shutdown()
	{
		glDeleteProgram(rendering_program);
		glDeleteBuffers(num_buffers, buffers);
		delete[] buffers;  // Deallocate memory
		buffers = NULL;  // Reset the pointer to null
		glDeleteVertexArrays(1, &vao);
	}

private:
	// Rendering program creation
	GLuint compile_shaders(void)
	{
		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint program;

		// Source code for vertex shader
		static const GLchar* vertex_shader_source[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"layout (location = 0) in vec3 position;							\n"
			"layout (location = 1) in vec3 color;								\n"
			"																	\n"
			"out vec4 vs_color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Add 4th-component to position - w coordinate					\n"
			"	gl_Position = vec4(position, 1.0);								\n"
			"	// Add 4th-component to color - alpha							\n"
			"	vs_color = vec4(color, 1.0);									\n"
			"}																	\n"
		};

		// Source code for fragment shader
		static const GLchar* fragment_shader_source[] =
		{
			"#version 450 core							\n"
			"											\n"
			"in vec4 vs_color;							\n"
			"											\n"
			"out vec4 color;							\n"
			"											\n"
			"void main(void)							\n"
			"{											\n"
			"	color = vs_color;						\n"
			"}											\n"
		};

		// Create and compile vertex shader
		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		// Create and compile fragment shader
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		// Create program, attach shaders to it, and link it
		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		// Delete the shaders as the program has them now
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}
	
	/// <summary>
	/// /// Create hard-coded buffers for a given vertex array object.
	/// </summary>
	/// <param name="buffer_mode">Define the way data is stored in memory.
	/// 0: separated attributes, single buffer;
	/// 1: separated attributes, multiple buffers;
	/// 2: interleaved attributed; single buffer. </param>
	/// <param name="vao">Target vertex array object.</param>
	/// <param name="buffers">Created buffer object(s) name(s).</param>
	/// <returns></returns>
	int create_buffers(int buffer_mode, GLuint vao, GLuint** buffers)
	{
		switch (buffer_mode)
		{
		case 0:
			return create_single_buffer_separated_attributes(vao, buffers);
			break;
		case 1:
			return create_multiple_buffers_separated_attributes(vao, buffers);
			break;
		case 2:
			return create_single_buffer_interleaved_attributes(vao, buffers);
			break;
		default:
			return -1;
			break;
		}
	}

	int create_single_buffer_separated_attributes(GLuint vao, GLuint** buffers)
	{
		const int num_buffers = 1;

		static const float positions[] = {
			 0.25, -0.25, 0.5,
			-0.25, -0.25, 0.5,
			 0.25,  0.25, 0.5
		};
		static const float colors[] = {
			 0.0, 0.8, 1.0,
			 0.8, 0.0, 1.0,
			 1.0, 0.8, 0.0
		};

		// Create buffer object
		GLuint* buffer = new GLuint;
		glCreateBuffers(num_buffers, buffer);

		// Allocate memory and copy data
		glNamedBufferStorage(*buffer, sizeof(positions) + sizeof(colors), NULL, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferSubData(*buffer, 0, sizeof(positions), positions);
		glNamedBufferSubData(*buffer, sizeof(positions), sizeof(colors), colors);

		// Bind as a vertex buffer object
		glBindBuffer(GL_ARRAY_BUFFER, *buffer);

		// Set up vertex attribute "position" (location = 0) and enable automatic load
		int position_attribute_size = sizeof(float) * 3;
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, *buffer, 0, position_attribute_size);  // Each vertex "position" attribute data can be found each 3 float values (position; position; ...), starting from the beginning
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(vao, 0);

		// Set up vertex attribute "color" (location = 1) and enable automatic load
		int color_attribute_size = sizeof(float) * 3;
		glVertexArrayAttribBinding(vao, 1, 1);
		glVertexArrayVertexBuffer(vao, 1, *buffer, sizeof(positions), color_attribute_size);  // Each vertex "color" attribute data can be found each 3 float values (color; color; ...), starting after "positions" data
		glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(vao, 1);

		*buffers = buffer;  // Save the memory address of the buffer name variable
		return num_buffers;
	}

	int create_multiple_buffers_separated_attributes(GLuint vao, GLuint** buffers)
	{
		const int num_buffers = 2;

		static const float positions[] = {
			 0.25, -0.25, 0.5,
			-0.25, -0.25, 0.5,
			 0.25,  0.25, 0.5
		};
		static const float colors[] = {
			 0.0, 0.8, 1.0,
			 0.8, 0.0, 1.0,
			 1.0, 0.8, 0.0
		};

		// Create buffer objects
		GLuint* buffer = new GLuint[num_buffers]();
		glCreateBuffers(num_buffers, &buffer[0]);  // Or just use "glCreateBuffers(num_buffers, buffer);", it is the same

		// Allocate memory and copy data - POSITIONS
		glNamedBufferStorage(buffer[0], sizeof(positions), positions, 0);

		// Bind as a vertex buffer object - POSITIONS
		glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);

		// Allocate memory and copy data - COLORS
		glNamedBufferStorage(buffer[1], sizeof(colors), colors, 0);

		// Bind as a vertex buffer object - COLORS
		glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);

		// Set up vertex attribute "position" (location = 0) and enable automatic load
		int position_attribute_size = sizeof(float) * 3;
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, buffer[0], 0, position_attribute_size);  // Each vertex "position" attribute data can be found each 3 float values (position; position; ...), starting from the beginning
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(vao, 0);

		// Set up vertex attribute "color" (location = 1) and enable automatic load
		int color_attribute_size = sizeof(float) * 3;
		glVertexArrayAttribBinding(vao, 1, 1);
		glVertexArrayVertexBuffer(vao, 1, buffer[1], 0, color_attribute_size);  // Each vertex "color" attribute data can be found each 3 float values (color; color; ...), starting from the beginning
		glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(vao, 1);

		*buffers = buffer;  // Save the memory address of the buffer name array
		return num_buffers;
	}

	int create_single_buffer_interleaved_attributes(GLuint vao, GLuint** buffers)
	{
		const int num_buffers = 1;

		static const float data[] = {
			 0.25, -0.25, 0.5, 0.0, 0.8, 1.0,
			-0.25, -0.25, 0.5, 0.8, 0.0, 1.0,
			 0.25,  0.25, 0.5, 1.0, 0.8, 0.0
		};

		// Create buffer object
		GLuint* buffer = new GLuint;
		glCreateBuffers(num_buffers, buffer);

		// Allocate memory and copy data
		glNamedBufferStorage(*buffer, sizeof(data), data, 0);

		// Bind as a vertex buffer object
		glBindBuffer(GL_ARRAY_BUFFER, *buffer);

		// Set up vertex attribute "position" (location = 0) and enable automatic load
		int vertex_size = sizeof(float) * 3 + sizeof(float) * 3;
		glVertexArrayAttribBinding(vao, 0, 0);
		glVertexArrayVertexBuffer(vao, 0, *buffer, 0, vertex_size);
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(vao, 0);

		// Set up vertex attribute "color" (location = 1) and enable automatic load
		glVertexArrayAttribBinding(vao, 1, 1);
		glVertexArrayVertexBuffer(vao, 1, *buffer, 0, vertex_size);
		glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3);  // Vertex attribute "color" can be found after vertex attribute "position" (3 float values)
		glEnableVertexArrayAttrib(vao, 1);

		*buffers = buffer;  // Save the memory address of the buffer name variable
		return num_buffers;
	}

private:
	// Few code lines to review pointer variable type
	void review_pointers()
	{
		// Operators:
		//  - Pointer: * (It is used to define a variable of "pointer" type. A pointer stores the memory address of another variable - primitive, class or another pointer.
		//  - Memory address: & (It is not possible to define a variable of "memory address" type. This operator is just to access the memory address of a variable.).

		int num1;
		int num2;

		int* int_ptr;
		void* any_ptr;  // Generic (just want to store a memory address so, why need to specify type?); less safe. Value access require proper type cast.

		int** ptr_to_int_ptr;
		void* any_ptr_other;  // Advantage - not need to recursively specify a ptr to a ptr to a ...

		int_ptr = &num1;
		any_ptr = &num1;

		ptr_to_int_ptr = &int_ptr;
		any_ptr_other = &int_ptr;

		num1 = 3;
		num2 = 5;
		
		// Modify the value of variable num1
		bool error = false;
		int new_value = 4;
		set_variable_value(new_value, int_ptr);
		error = num1 != new_value ? true : false;
		// -> Check status of pointer int_ptr (can access new value stored in variable num1)

		// Make int_ptr point to variable num2 (may be more useful with pointers to objects - create new class instance inside a method and return pointer to that object - rather than primitive variable types)
		error = false;
		set_pointer_value_error(&num2, int_ptr);
		error = int_ptr != &num2 ? true : false;
		// -> Check status of pointer int_ptr (not updated)

		// Now, do it properly: pass to the function the address of the pointer int_ptr, not the address stored in the pointer (address of variable num1).
		error = false;
		set_pointer_value_success(&num2, &int_ptr);
		error = int_ptr != &num2 ? true : false;
		// -> Check status of pointer int_ptr (now points to the other variable num2)
	}

	void set_variable_value(int value, int* ptr)
	{
		*ptr = value;
	}

	void set_pointer_value_error(int* value, int* ptr)
	{
		ptr = value;
	}

	void set_pointer_value_success(int* value, int** ptr)
	{
		*ptr = value;
	}

private:
	GLuint rendering_program;
	GLuint vao;
	int num_buffers;
	GLuint* buffers;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);