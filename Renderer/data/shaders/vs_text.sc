$input a_position
$output v_color0

#include "bgfx_shader.sh"

uniform vec4 u_color;

void main()
{
	gl_Position = mul(vec4(a_position, 1.0), u_modelViewProj);
	v_color0 = u_color;
}
