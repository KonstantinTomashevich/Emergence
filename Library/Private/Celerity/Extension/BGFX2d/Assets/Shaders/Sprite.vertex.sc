$input a_position, a_uv0
$output v_uv0

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position.xy, 0.0, 1.0) );
	v_uv0 = a_uv0;
}
