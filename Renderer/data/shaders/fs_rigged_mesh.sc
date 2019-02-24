$input v_texcoord0

#include "bgfx_shader.sh"

void main()
{
	gl_FragColor.r = v_texcoord0.x;
	gl_FragColor.g = v_texcoord0.y;
	gl_FragColor.b = 0;
	gl_FragColor.a = 1;
}
