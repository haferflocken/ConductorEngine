$input a_position, a_color0
$output v_color0

#include "bgfx_shader.sh"

void main()
{
	gl_Position = mul(vec4(a_position, 1.0), u_modelViewProj);
	v_color0 = a_color0;
}
