$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(colorTexture, 0);

void main()
{
    vec4 texel = texture2D(colorTexture, v_texcoord0);
    gl_FragColor = texel * v_color0;
}
