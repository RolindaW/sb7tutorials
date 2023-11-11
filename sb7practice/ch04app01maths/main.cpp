// Include the "sb7.h" header file
#include "sb7.h"

// Include the "vmath" header file
#include "vmath.h"
//using namespace vmath;  // Avoid typing namespace everytime library is used

// Derive my_application from sb7::application
class my_application : public sb7::application
{
public:
	void startup()
	{
		sb7_math_sample();
		custom_math_sample();
	}

private:

	void sb7_math_sample()
	{
		// Declaring three-component vector
		vmath::vec3 vVector;

		// A number of constructors and copy operators
		vmath::vec3 vVertex1(0.0f, 0.0f, 1.0f);
		vmath::vec4 vVertex2 = vmath::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		vmath::vec4 vVertex3(vVertex1, 1.0f);

		// An array of three-component vertices, such as for a triangle
		vmath::vec3 vVerts[] = { vmath::vec3(-0.5f, 0.0f, 0.0f),
								 vmath::vec3(0.5f, 0.0f, 0.0f),
								 vmath::vec3(0.0f, 0.5f, 0.0f) };

		// Common vector operators
		vmath::vec3 a(1.0f, 2.0f, 3.0f);
		vmath::vec3 b(4.0f, 5.0f, 6.0f);
		vmath::vec3 c;

		c = a + b;
		c = a - b;
		c += b;
		c = -c;

		// Dot product
		//float c_dot = a.dot(b);
		float d_dot = vmath::dot(a, b);

		// Angle
		//float angle = vmath::angle(a, b);

		// Cross product
		//vmath::vec3 c_cross = a.cross(b);
		vmath::vec3 d_cross = vmath::cross(a, b);

		// Length
		float d_len = vmath::length(a);

		// Reflection
		vmath::vec3 r_in(1.0f, -1.0f, 0.0f);
		r_in = vmath::normalize(r_in);
		vmath::vec3 s_norm(0.0f, 1.0f, 0.0f);
		vmath::vec3 r_reflect = vmath::reflect(r_in, s_norm);

		// Refraction
		float refract_n = 0.0f;
		//vmath::vec3 r_refract = vmath::refract(r_in, s_norm, refract_n);

		// Most extensively used matrices by size: 2x2, 3x3, 4x4
		vmath::mat2 m1;
		vmath::mat3 m2;
		vmath::mat4 m3;

		// Handmade matrix construction
		GLfloat matrix[16];  // Nice OpenGL-friendly matrix
		GLfloat matrix_alt[4][4];  // Not as convenient for OpenGL programmers

		// Identity matrix
		GLfloat m1_identity[16] = { 1.0f, 0.0f, 0.0f, 0.0f,		// X Column
									0.0f, 1.0f, 0.0f, 0.0f,		// Y Column
									0.0f, 0.0f, 1.0f, 0.0f,		// Z Column
									0.0f, 0.0f, 0.0f, 1.0f };	// W Column

		vmath::mat4 m2_identity(vmath::vec4(1.0f, 0.0f, 0.0f, 0.0f),	// X Column
								vmath::vec4(0.0f, 1.0f, 0.0f, 0.0f),	// Y Column
								vmath::vec4(0.0f, 0.0f, 1.0f, 0.0f),	// Z Column
								vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));	// W Column

		vmath::mat2 m3_identity_2x2 = vmath::mat2::identity();
		vmath::mat3 m3_identity_3x3 = vmath::mat3::identity();
		vmath::mat4 m3_identity_4x4 = vmath::mat4::identity();

		// Translation matrix
		vmath::vec3 t(1.0f, -2.5f, 3.0f);
		vmath::mat4 m1_trans = vmath::translate(t[0], t[1], t[2]);
		vmath::mat4 m2_trans = vmath::translate(t);

		// Rotation matrix - Rotate around any of the three cardinal axes (x, y or z) of the coordinate system
		float angle_x = 0.0f;
		float angle_y = 30.0f;
		float angle_z = 0.0f;
		vmath::mat4 m1_rotate = vmath::rotate(angle_x, angle_y, angle_z);

		// Rotation matrix - Rotate around an arbitrary axis (passing through the origin of coordinate system)
		float angle = 45.0f;
		vmath::vec3 v_rotate = vmath::vec3(1.0f, 1.0f, 1.0f);
		vmath::mat4 m2_rotate = vmath::rotate(angle, v_rotate[0], v_rotate[1], v_rotate[2]);
		vmath::mat4 m2_rotate_alt = vmath::rotate(angle, v_rotate);

		// Rotation matrix - Euler angles
		float pitch = 0.0f;
		float yaw = 30.0f;
		float roll = 0.0f;
		vmath::mat4 m1_euler = vmath::rotate(pitch, yaw, roll);

		// Rotation matrix - Quaternion
		vmath::vec4 quaternion = vmath::quaternion(angle, v_rotate[0], v_rotate[1], v_rotate[2]);
		vmath::vec4 quaternion_alt = vmath::quaternion(angle, v_rotate);
		vmath::mat4 m1_quaternion = vmath::mat4(quaternion);
		vmath::mat4 m2_quaternion = vmath::mat4(quaternion_alt);

		// Scaling matrix
		vmath::vec3 scale(2.5f, 1.0f, 7.0f);
		vmath::mat4 m1_scale = vmath::scale(scale[0], scale[1], scale[2]);
		vmath::mat4 m2_scale = vmath::scale(scale);
		vmath::mat4 m3_scale = vmath::scale(scale[0]);  // Uniform scaling

		// LookAt matrix
		vmath::vec3 eye(10.0f, 3.0f, 10.0f);
		vmath::vec3 target(0.0f, 0.0f, 0.0f);
		vmath::vec3 up;  // Stored and updated all time in camera game object
		vmath::mat4 m1_lookat = vmath::lookat(eye, target, up);

		// Proyection matrix - Orthograpic
		float left = -10.0f, right = 10.0f, bottom = -4.0f, top = 4.0f;
		float n = 0.1f, f = 100.0f;  // Near and far planes.
		vmath::mat4 m1_ortho = vmath::ortho(left, right, bottom, top, n, f);

		// Proyection matrix - Perspective
		float field_of_view = 45.0f;
		float aspect_ratio = (float)this->info.windowWidth / (float)this->info.windowHeight;
		vmath::mat4 m1_pers = vmath::perspective(field_of_view, aspect_ratio, n, f);
		vmath::mat4 m2_pers = vmath::frustum(left, right, bottom, top, n, f);
	}

	void custom_math_sample()
	{
		// Dot product
		vmath::vec3 v1(1.0f, 0.0f, 0.0f);
		vmath::vec3 v2(-1.0f, 0.0f, 0.0f);
		// v1.x * v2.x + v1.y * v2.y + v1.z * v2.z
		float dot_v1_v2 = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
		//float dot_v1_v2_test = vmath::dot(v1, v2);

		// Angle
		float angle = acos(dot_v1_v2);  // rad
		//float angle_test = vmath::angle(v1, v2);

		// Cross product
		vmath::vec3 v1_cross(1.0f, 0.0f, 0.0f);
		vmath::vec3 v2_cross(0.0f, 1.0f, 0.0f);
		// [v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x]
		vmath::vec3 v3_cross(v1_cross[1] * v2_cross[2] - v1_cross[2] * v2_cross[1],
							 v1_cross[2] * v2_cross[0] - v1_cross[0] * v2_cross[2], 
							 v1_cross[0] * v2_cross[1] - v1_cross[1] * v2_cross[0]);
		//vmath::vec3 v3_cross_test = vmath::cross(v1_cross, v2_cross);

		// Length
		vmath::vec3 v(1.0f, 1.0f, 0.0f);
		// sqrt(x^2 + y^2 + z^2)
		float v_length = sqrtf(powf(v[0], 2) + powf(v[1], 2) + powf(v[2], 2));
		float v_length_test = vmath::length(v);
	}
};

// Our one and only instance of DECLARE_MAIN
DECLARE_MAIN(my_application);