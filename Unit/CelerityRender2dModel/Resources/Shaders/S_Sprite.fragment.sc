$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(colorTexture,  0);

void main()
{
    gl_FragColor = texture2D(colorTexture, v_texcoord0);
}
