// Include the "sb7.h" header file
#include "sb7.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		buffer = create_buffers();
	}

	void render(double currentTime)
	{
		// Put into a buffer is a constant value (replicate the data across the range of the buffer’s data store)
		//const float new_data = sin(currentTime);
		//glClearNamedBufferSubData(buffer, GL_R32F, 0, 4 * 4, GL_RED, GL_FLOAT, &new_data);
		const float new_data[] = { sin(currentTime), cos(currentTime) };
		glClearNamedBufferSubData(buffer, GL_RG32F, 0, 4 * 4, GL_RG, GL_FLOAT, new_data);

		float buffer_data[12];
		glGetNamedBufferSubData(buffer, 0, 4 * 12, &buffer_data[0]);
	}

	void shutdown()
	{
		glDeleteBuffers(1, &buffer);
	}

private:
	// Buffers creation
	GLuint create_buffers(void)
	{
		// Create a buffer object (and corresponding name)
		GLuint buffer;
		glCreateBuffers(1, &buffer);
		/*GLuint buffers[2];
		glCreateBuffers(2, &buffers[0]);*/  // Create multiple buffers one time

		// Specify the data store parameters for the buffer
		glNamedBufferStorage(buffer,	// Name of the buffer
			1024 * 1024,	// 1 MiB of space - in bytes
			NULL,	// No initial data
			GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);	// Allow map for writting
		/*glBufferStorage(GL_ARRAY_BUFFER,
						1024 * 1024,
						NULL,
						GL_DYNAMIC_STORAGE_BIT & GL_MAP_WRITE_BIT);*/  // Pre-binding required

						// Bind a named buffer to a target
		glBindBuffer(GL_ARRAY_BUFFER, buffer);  // Store vertex data

		// This is the data that we will place into the buffer
		static const float data[] = {
			 0.25, -0.25, 0.5, 1.0,
			-0.25, -0.25, 0.5, 1.0,
			 0.25,  0.25, 0.5, 1.0
		};

		// Put the data into the buffer
		bool map = false;
		if (!map)
		{
			// Copy data
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
			//glNamedBufferSubData(buffer, 0, sizeof(data), data);
		}
		else  // Map buffer and then copy the data there yourself.
		{
			// Get a pointer to the buffer's data store
			void* ptr = glMapNamedBuffer(buffer, GL_WRITE_ONLY);
			//glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

			// Get a pointer to specific range of the buffer's data store
			//void* ptr = glMapNamedBufferRange(buffer, 0, sizeof(data), GL_MAP_WRITE_BIT);
			//void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(data), GL_MAP_WRITE_BIT);

			// Copy data into the data store
			memcpy(ptr, data, sizeof(data));

			// Unmap the buffer
			glUnmapNamedBuffer(buffer);
			//glUnmapBuffer(GL_ARRAY_BUFFER);
		}

		return buffer;
	}

private:
	GLuint buffer;
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);