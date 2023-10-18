// Include the "sb7.h" header file
#include "sb7.h"

// Include "math.h" library to get access to sin() and cos() functions
#include "math.h"

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	// Our rendering function
	void render(double currentTime)
	{
		const bool fixed_color = false;
		if (fixed_color)
		{
			// Simply clear the window with red
			static const GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
			glClearBufferfv(GL_COLOR, 0, red);
		}
		else
		{
			// Calculate new color based on elapsed time since the application is started
			const GLfloat color[] = { (float)sin(currentTime) * 0.5f + 0.5f,
			(float)cos(currentTime) * 0.5f + 0.5f,
			0.0f,
			1.0f };
			glClearBufferfv(GL_COLOR, 0, color);
		}
	}
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);