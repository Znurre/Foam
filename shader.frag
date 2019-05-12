#version 330 core

uniform sampler2D texture;

in vec4 v_color;
in vec4 v_uv;
in vec2 v_position;

void main(void)
{
	gl_FragColor = v_color;
//	gl_FragColor = vec4(1, 1, 1, texture2D(texture, v_position).r) * v_color;

	if (length(v_uv) > 0)
	{
		vec2 uv = vec2(v_uv.x + (v_uv.z * v_position.x), v_uv.y + (v_uv.w * v_position.y));

//		gl_FragColor = vec4(uv, 0, 1);
		gl_FragColor = vec4(1, 1, 1, texture2D(texture, uv).r) * v_color;
	}
}
