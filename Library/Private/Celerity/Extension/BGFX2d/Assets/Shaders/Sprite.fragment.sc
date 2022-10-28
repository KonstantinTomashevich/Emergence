$input v_uv0

#include <bgfx_shader.sh>

SAMPLER2D(s_textureColor,  0);

void main()
{
	gl_FragColor = texture2D(s_textureColor, v_uv0);
}
