// Include the "sb7.h" header file
#include "sb7.h"



// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	// Our rendering function
	void render(double currentTime)
	{
		// Simply clear the window with red
		static const GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, red);
	}
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);