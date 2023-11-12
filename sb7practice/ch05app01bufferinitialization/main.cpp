// Include the "sb7.h" header file
#include "sb7.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		// Create a buffer object (and corresponding name)
		GLuint buffer;
		glCreateBuffers(1, &buffer);

		/*GLuint buffers[2];
		glCreateBuffers(2, &buffers[0]);*/  // Create multiple buffers one time

		// Specify the data store parameters for the buffer
		glNamedBufferStorage(buffer,	// The target the buffer object is bound to
							 1024 * 1024,	// 1 MiB of space - in bytes
							 NULL,	// No initial data
							 GL_MAP_WRITE_BIT);	// Allow map for writting
		/*glBufferStorage(GL_ARRAY_BUFFER,
						1024 * 1024,
						NULL,
						GL_MAP_WRITE_BIT);*/  // Pre-binding required

		// Bind a named buffer object to a target
		glBindBuffer(GL_ARRAY_BUFFER, buffer);  // Store vertex data
	}
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);