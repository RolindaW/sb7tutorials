// Include the "sb7.h" header file
#include "sb7.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		InitializeProgram();
		QuerySubroutines(&subroutines);
		InitializeVao();
	}

	void render(double currentTime)
	{
		// Clear color buffer
		const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		int i = (int)currentTime;

		glUseProgram(program);
		
		GLuint subroutineIndex = i & 1;
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutines[subroutineIndex]);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void shutdown()
	{
		glDeleteProgram(program);
		glDeleteVertexArrays(1, &vao);
		delete[] subroutines;
	}

private:
	void InitializeProgram()
	{
		// Vertex shader
		const char* vertexShaderSource[] =
		{
			"#version 450 core													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Declare a hard-coded array of positions						\n"
			"	const vec4 vertices[3] = vec4[3](vec4(-0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25, -0.25, 0.5, 1.0),	\n"
			"									 vec4( 0.25,  0.25, 0.5, 1.0)); \n"
			"																	\n"
			"	// Index into our array using gl_VertexID						\n"
			"	gl_Position = vertices[gl_VertexID];							\n"
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
			"// Declare the subroutine type										\n"
			"subroutine vec4 sub_mySubroutine(vec4 param1);						\n"
			"																	\n"
			"// Declare a cople of functions that can be used as subroutines	\n"
			"layout (index = 2)													\n"
			"subroutine (sub_mySubroutine)										\n"
			"vec4 myFunction1(vec4 param1)										\n"
			"{																	\n"
			"	return param1 * vec4(1.0, 0.25, 0.25, 1.0);						\n"
			"}																	\n"
			"																	\n"
			"layout (index = 1)													\n"
			"subroutine (sub_mySubroutine)										\n"
			"vec4 myFunction2(vec4 param1)										\n"
			"{																	\n"
			"	return param1 * vec4(0.25, 0.25, 1.0, 1.0);						\n"
			"}																	\n"
			"																	\n"
			"layout (index = 7)													\n"
			"subroutine (sub_mySubroutine)										\n"
			"vec4 myFunction3(vec4 param1)										\n"
			"{																	\n"
			"	return param1 * vec4(0.25, 1.0, 0.25, 1.0);						\n"
			"}																	\n"
			"																	\n"
			"// Declare a subroutine uniform that can be pointed				\n"
			"subroutine uniform sub_mySubroutine mySubroutineUniform;			\n"
			"																	\n"
			"out vec4 color;													\n"
			"																	\n"
			"void main(void)													\n"
			"{																	\n"
			"	// Call subroutine through uniform								\n"
			"	color = mySubroutineUniform(vec4(1.0));							\n"
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

	void QuerySubroutines(GLuint** subroutines)
	{
		// ***
		// *** Subroutine (function)
		// ***

		// glGetProgramResourceIndex
		// Query the index of a subroutine (by name)
		GLint subroutineIndex1 = glGetProgramResourceIndex(program, GL_FRAGMENT_SUBROUTINE, "myFunction1");
		GLint subroutineIndex2 = glGetProgramResourceIndex(program, GL_FRAGMENT_SUBROUTINE, "myFunction2");
		GLint subroutineIndex3 = glGetProgramResourceIndex(program, GL_FRAGMENT_SUBROUTINE, "myFunction3");

		// glGetProgramResourceName
		// Query the name of a subroutine (by index)
		GLchar subroutineName1[64];
		glGetProgramResourceName(program, GL_FRAGMENT_SUBROUTINE, 2, sizeof(subroutineName1), NULL, subroutineName1);
		GLchar subroutineName2[64];
		glGetProgramResourceName(program, GL_FRAGMENT_SUBROUTINE, 1, sizeof(subroutineName2), NULL, subroutineName2);
		GLchar subroutineName3[64];
		glGetProgramResourceName(program, GL_FRAGMENT_SUBROUTINE, 7, sizeof(subroutineName3), NULL, subroutineName3);

		// glGetProgramStageiv
		// Query properties of a program stage
		GLint activeSubroutineNumber;
		glGetProgramStageiv(program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINES, &activeSubroutineNumber);

		GLint longestSubroutineNameLength;
		glGetProgramStageiv(program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_MAX_LENGTH, &longestSubroutineNameLength);

		// glGetSubroutineIndex
		// Query the index of a subroutine (by name)
		GLint subroutineIndex1b = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "myFunction1");
		GLint subroutineIndex2b = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "myFunction2");
		GLint subroutineIndex3b = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "myFunction3");

		// glGetActiveSubroutineName
		// Query the name of an active shader subroutine
		GLchar activeSubroutineName1[64];
		glGetActiveSubroutineName(program, GL_FRAGMENT_SHADER, 2, sizeof(activeSubroutineName1), NULL, activeSubroutineName1);
		GLchar activeSubroutineName2[64];
		glGetActiveSubroutineName(program, GL_FRAGMENT_SHADER, 1, sizeof(activeSubroutineName2), NULL, activeSubroutineName2);
		GLchar activeSubroutineName3[64];
		glGetActiveSubroutineName(program, GL_FRAGMENT_SHADER, 7, sizeof(activeSubroutineName3), NULL, activeSubroutineName3);

		// ***
		// *** Subroutine uniform
		// ***

		// glGetProgramInterfaceiv
		// Query a property of an interface: the number of compatible subroutines belonging to the active subroutine uniform in GL_FRAGMENT_SUBROUTINE_UNIFORM interface with the most compatible subroutine functions
		// Warning! It is not safe to iterate through active subroutine number to query properties of each when indices specified manually (because indices may not start at 0 or follow each other).
		GLint subroutineUniformCompatibleSubroutinesNumber;
		glGetProgramInterfaceiv(program, GL_FRAGMENT_SUBROUTINE_UNIFORM, GL_MAX_NUM_COMPATIBLE_SUBROUTINES, &subroutineUniformCompatibleSubroutinesNumber);

		// glGetProgramResourceiv
		// Query multiple properties of a subroutine ubiform (by index) in GL_FRAGMENT_SUBROUTINE_UNIFORM interface
		const GLuint kSubroutineUniformIndex = 0;
		const GLenum properties[] = { GL_NUM_COMPATIBLE_SUBROUTINES, GL_ARRAY_SIZE, GL_LOCATION };
		GLint parameters[3];
		glGetProgramResourceiv(program, GL_FRAGMENT_SUBROUTINE_UNIFORM, kSubroutineUniformIndex, 3, properties, sizeof(parameters), NULL, parameters);

		// Warning! Property GL_COMPATIBLE_SUBROUTINES returns an array, so be carefull with "parameters" output variable size.
		GLuint compatibleSubroutinesNumber = (GLuint)parameters[0];
		GLint* compatibleSubroutines = new GLint[compatibleSubroutinesNumber];
		const GLenum properties2[] = { GL_COMPATIBLE_SUBROUTINES };
		glGetProgramResourceiv(program, GL_FRAGMENT_SUBROUTINE_UNIFORM, kSubroutineUniformIndex, 1, properties2, sizeof(GLint) * compatibleSubroutinesNumber, NULL, compatibleSubroutines);
		//*subroutines = (GLuint*)compatibleSubroutines;
		delete[] compatibleSubroutines;

		// glGetProgramResourceIndex
		// Query the index of a subroutine uniform (by name)
		GLint subroutineUniformIndex = glGetProgramResourceIndex(program, GL_FRAGMENT_SUBROUTINE_UNIFORM, "mySubroutineUniform");

		// glGetProgramResourceName
		// Query the name of a subroutine uniform (by index)
		GLchar subroutineUniformName[64];
		glGetProgramResourceName(program, GL_FRAGMENT_SUBROUTINE_UNIFORM, kSubroutineUniformIndex, sizeof(subroutineUniformName), NULL, subroutineUniformName);

		// glGetProgramStageiv
		// Query properties of a program stage
		GLint activeSubroutineVariableNumber;
		glGetProgramStageiv(program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &activeSubroutineVariableNumber);

		GLint activeSubroutineVariableLocationNumber;
		glGetProgramStageiv(program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &activeSubroutineVariableLocationNumber);

		GLint longestSubroutineUniformNameLength;
		glGetProgramStageiv(program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH, &longestSubroutineUniformNameLength);

		// glGetSubroutineUniformLocation
		// Query the location of a subroutine uniform (by name)
		GLint subroutineUniformLocation = glGetSubroutineUniformLocation(program, GL_FRAGMENT_SHADER, "mySubroutineUniform");

		// glGetActiveSubroutineUniformiv
		// Query a property of a subroutine uniform (by index)
		GLint activeSubroutineUniformCompatibleSubroutinesNumber;
		glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, kSubroutineUniformIndex, GL_NUM_COMPATIBLE_SUBROUTINES, &activeSubroutineUniformCompatibleSubroutinesNumber);

		GLint* activeSubroutineUniformCompatibleSubroutines = new GLint[activeSubroutineUniformCompatibleSubroutinesNumber];
		glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, kSubroutineUniformIndex, GL_COMPATIBLE_SUBROUTINES, activeSubroutineUniformCompatibleSubroutines);
		*subroutines = (GLuint*) activeSubroutineUniformCompatibleSubroutines;
		//delete[] activeSubroutineUniformCompatibleSubroutines;

		GLint activeSubroutineUniformArraySize;
		glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, kSubroutineUniformIndex, GL_UNIFORM_SIZE, &activeSubroutineUniformArraySize);

		GLint activeSubroutineUniformNameLength;  // Including the terminating null character
		glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, kSubroutineUniformIndex, GL_UNIFORM_NAME_LENGTH, &activeSubroutineUniformNameLength);

		// glGetActiveSubroutineUniformName
		// Query the name of an subroutine uniform (by index)
		GLchar activeSubroutineUniformName[64];
		glGetActiveSubroutineUniformName(program, GL_FRAGMENT_SHADER, kSubroutineUniformIndex, sizeof(activeSubroutineUniformName), NULL, activeSubroutineUniformName);
	}

	void InitializeVao()
	{
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

private:
	GLuint program;
	GLuint* subroutines;
	GLuint vao;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);