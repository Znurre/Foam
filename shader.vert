#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in mat3 matrix;

out vec4 v_color;

void main(void)
{
	const float right = 800;
	const float left = 0;
	const float top = 0;
	const float bottom = 600;

	mat3 projection = mat3(
		vec3(2.0 / (right - left), 0, 0),
		vec3(0, 2.0 / (top - bottom), 0),
		vec3(-(right + left) / (right - left), -(top + bottom) / (top - bottom), 1)
	);

	gl_Position = vec4(projection * matrix * vec3(position, 1), 1);

	v_color = color;
}
